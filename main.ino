#include <deprecated.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>
#include <LiquidCrystal.h>

bool access = false;
bool diode = LOW;
bool cardAdded = false;
bool overflow = false;

String menu[] = {"Poziom cieczy", "Dodaj karte", "Usun blad", "Zablokuj"};

int measurement = 0;
int position = 0;
int pressedButton1Time = 0;
int pressedButton2Time = 0;
int cardsCount = 0;

int UID[5][4];

LiquidCrystal lcd(3, 4, 5, 6, 7, 8);

MFRC522 rfid(10,9);

bool CheckIfCardExists(int UIDs[][4])
{
  bool result = true;
  for(int i = 0; i < 5; i++)
    {
      result = true;
      for(int j = 0; j < 4; j++)
      {
        if(UIDs[i][j] != rfid.uid.uidByte[j])
        {
          result = false;
        }
      }
      if(result)
      {
        return true;
      }
    }
  return false;
}

void setup() {
  SPI.begin();
  lcd.begin(16, 2);
  rfid.PCD_Init();

  TCCR1B |= (1<<CS12);
  TIMSK1 |= (1<<OCIE0A);

  OCR1A = 31250;

  rfid.PCD_Init();

  pinMode(2, OUTPUT);
  pinMode(1, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP);

  sei(); 

  while (!cardAdded)
  {
    lcd.setCursor(0, 0);
    lcd.print("Dodaj karte");
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()){
      cardAdded = true;
      for (int i = 0; i < 4; i++)
      {
        UID[0][i] = rfid.uid.uidByte[i];
      }
      lcd.setCursor(0, 0);
      lcd.print("Karta dodana");
      lcd.setCursor(0, 1);
      lcd.print("pomyslnie");
      delay(1000);
      cardsCount++;
      lcd.clear();
    }
  }
}

void loop() {

  measurement = analogRead(A0);

  if(measurement > 270)
  {
    overflow = true;
  }

  if(access == false)
  {
    lcd.setCursor(0, 0);
    lcd.print("Zablokowane ");
    if(measurement > 270)
    {
      lcd.setCursor(0, 1);
      lcd.print("Przepelnienie!");
    }
    else if(measurement > 225)
    {
      lcd.setCursor(6, 1);
      lcd.print("        ");
      lcd.setCursor(0, 1);
      lcd.print("Uwaga!");
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print("              ");
    }
  }

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial() && !access)
  {
    if(CheckIfCardExists(UID))
    {
      access = true;
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print("Zla karta!");
      delay(1000);
      lcd.clear();
    }
  }

  while(access)
  {
    lcd.setCursor(0, 1);
    lcd.print("              ");
    lcd.setCursor(0, 0);
    lcd.print(menu[position] + "     ");
    if(digitalRead(1) == LOW)
    {
      pressedButton1Time++;
    }
    if(digitalRead(0) == LOW)
    {
      pressedButton2Time++;
    }
    if(pressedButton1Time > 50)
    {
      pressedButton1Time = 0;
      position++;
    }
    if(position > 3)
    {
      position = 0;
    }
    if(pressedButton2Time > 50)
    {
      pressedButton2Time = 0;
      switch (position)
      {
        case 0:
          lcd.clear();
          while(digitalRead(1) != LOW)
          {
            measurement = analogRead(A0);

            lcd.setCursor(0, 0);
            lcd.print("Poziom: ");
            lcd.setCursor(8, 0);
            lcd.print((measurement*100)/300);
            lcd.print("%");
            if(measurement < 1000)
            {
              lcd.print(" ");
              if(measurement < 100)
              {
                lcd.print(" ");
                if(measurement < 10)
                {
                  lcd.print(" ");
                }
              }
            }
          }
          break;
        case 1:
          lcd.clear();
          if(cardsCount < 5)
          {
            cardAdded = false;
            while(!cardAdded)
            {
            lcd.setCursor(0, 0);
            lcd.print("Zbliz karte");
              if(rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
              {
              if(CheckIfCardExists(UID))
              {
                lcd.setCursor(0, 0);
                lcd.print("Karta istnieje");
                delay(1000);
                break;
              }
              else
              {
                for (int i = 0; i < 4; i++)
                {
                  UID[cardsCount][i] = rfid.uid.uidByte[i];
                }
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Karta dodana");
                lcd.setCursor(0, 1);
                lcd.print("pomyslnie");
                delay(1000);
                lcd.clear();
                cardsCount++;
                cardAdded = true;
                position = 0;
                break;
              }
              }
            }
          }
          else
          {
            lcd.setCursor(0, 0);
            lcd.print("Osiagnieto limit");
            delay(1000);
          }
          break;
        case 2:
          if (overflow)
          {
            overflow = false;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Blad usuniety");
            delay(1000);
          }
          else
          {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Brak bledow");
            delay(1000);
          }
          position = 0;
          break;
        case 3:
          access = false;
          position = 0;
          break;
        default:
          break;
      }
    }
  }
}

ISR(TIMER1_COMPA_vect){
  TCNT1  = 0;   
  if(overflow && !access)
  {
    diode = !diode;
    digitalWrite(2, diode);
  }
  else 
  {
    digitalWrite(2, LOW);
  }
}
