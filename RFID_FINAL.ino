#include "SPI.h"
#include "MFRC522.h"
#include <TM1637Display.h>

#define SS_PIN 10
#define RST_PIN 9
#define RED_LIGHT 8
#define GREEN_LIGHT 7
#define BUTTON_PIN 6
#define LIGHTS_TIME 500


const int CLK = 4; //Set the CLK pin connection to the display
const int DIO = 3 ; //Set the DIO pin connection to the display
long minute = 60000; // 60000 milliseconds in a minute
long second =  1000; // 1000 milliseconds in a second

int buttonState = 0;
unsigned long startTimeFirst = 0;
unsigned long startTimeSecond = 0;
int carsCount = 0;
int NumStep =0;

TM1637Display display(CLK, DIO);
MFRC522 rfid(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  pinMode(RED_LIGHT, OUTPUT);
  pinMode(GREEN_LIGHT, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  rfid.PCD_Init();
  //set the diplay to maximum brightness
  display.setBrightness(0x0a);
}

void loop() {
  buttonState = digitalRead(BUTTON_PIN);
  blinkLight(buttonState);

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

   MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }
  
  String strID = getRFID();
  Serial.println("Time first:");
  Serial.println(startTimeFirst);
  onCardChecked("12:34:98:0D", startTimeFirst, strID, carsCount);
  onCardChecked("E5:81:F7:C2", startTimeSecond, strID, carsCount);
  
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// Get ID of checked card
String getRFID() {
   String strID = "";
   for (byte i = 0; i < 4; i++) {
     strID +=
    (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
    String(rfid.uid.uidByte[i], HEX) +
    (i!=3 ? ":" : "");
  }
  strID.toUpperCase();
  return strID;
}

//rfid is checked 
void onCardChecked(String ID,unsigned long& parkingTime, String& strID, int& carsCount) {
  if (strID.indexOf(ID) >= 0) {
    if (parkingTime == 0) {
       carsCount ++;
       parkingTime = millis();
    } else {
      showTime(parkingTime);
      parkingTime = 0;
      carsCount = carsCount -1;
    }
    strID = "";
  }
}

//Checking for parking availability
void blinkLight(int buttonState) {
   if (buttonState == HIGH) {
    // no places in parking lot
    if( carsCount > 1) {
        delay(LIGHTS_TIME);  
        digitalWrite(RED_LIGHT, HIGH);
        delay(LIGHTS_TIME);           
        digitalWrite(RED_LIGHT, LOW); 
      } 
     // there is place
     else {
        delay(LIGHTS_TIME);           
        digitalWrite(GREEN_LIGHT, HIGH);
        delay(LIGHTS_TIME);            
        digitalWrite(GREEN_LIGHT, LOW); 
      }
     delay(1000);
  }
}

void showTime(unsigned long startTime) {
  unsigned long endTime = millis();
  unsigned long parkingTime = endTime - startTime;
  int minutes = parkingTime / minute ;
  int seconds = (parkingTime % minute) / second;
  display.showNumberDec(minutes,true,2,0);
  display.showNumberDec(seconds,true,2,2);
  delay(5000);
  display.showNumberDec(0);
}

