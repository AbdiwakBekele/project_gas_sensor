// Libraries
#include <Keypad.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

SoftwareSerial mySerial(12, 13); // RX, TX
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {8, 9, 10, 11};
byte colPins[COLS] = {12, 13, 14, 15};

int gasSensor = 0; // storing gass value
int startBtn = 16;
int relayMotor = 17;
int greenLed = 18;
int yellowLed = 19;
int redLed = 20;
int buzzer = 21;
bool btnState = false;
bool alcState = false;
int lcdCursor = 0;
String passKey = "0000";
String userKey = "";


Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
  pinMode(startBtn, OUTPUT);
  pinMode(relayMotor, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  lcd.begin(16, 2);

}

void loop() {

  // Getting Keypad Key
  char customKey = customKeypad.getKey();

  // reading Gas Status
  gasSensor = analogRead(0);

  // Push Button Starter
  if (digitalRead(13) == HIGH) {
    btnState = !btnState;
    delay(200);
  }

  if (btnState and !alcState) {

    //checking Alchol level
    if (gasSensor < 300) {
      // Status Normal
      digitalWrite(relayMotor, HIGH);
      digitalWrite(buzzer, LOW);
      digitalWrite(greenLed, HIGH);
      digitalWrite(yellowLed, LOW);
      digitalWrite(redLed, LOW);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Status: Sober");
      lcd.setCursor(0, 1);
      lcd.print("Pass");
      delay(100);
    } else if (gasSensor < 600) {
      // Status conscious
      digitalWrite(relayMotor, HIGH);
      digitalWrite(buzzer, HIGH);
      digitalWrite(greenLed, LOW);
      digitalWrite(yellowLed, HIGH);
      digitalWrite(redLed, LOW);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Status: conscious");
      lcd.setCursor(0, 1);
      lcd.print("Pass with Limit");
      delay(100);
    } else {
      // Status conscious
      sendMessage("Drunk, Car Locked");
      digitalWrite(relayMotor, LOW);
      digitalWrite(buzzer, HIGH);
      digitalWrite(greenLed, LOW);
      digitalWrite(yellowLed, LOW);
      digitalWrite(redLed, HIGH);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Status: Drunk");
      lcd.setCursor(0, 1);
      lcd.print("Locked");

      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter Password");
      lcd.setCursor(0, 1);

      alcState = true;
      delay(100);
    }

  } else if (btnState and alcState) {
    // Status: Locked

    if (customKey) {

      //Input Keys
      if (lcdCursor <= 3) {
        lcd.print(customKey);
        lcdCursor++;
        lcd.setCursor(lcdCursor, 1);
        userKey += customKey;

        if (lcdCursor == 4) {
          loading();
          delay(500);

          // Check password
          if (userKey == passKey) {
            accessGranted();
          }
          else {
            accessDenied();
          }
          clearPassword();

        }
      }
    }
  }


}

// Send Message
void sendMessage(String msg) {
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  mySerial.println("AT+CMGS=\"+251932265791\"\r"); // Replace x with mobile number
  delay(1000);
  mySerial.println(msg);// The SMS text you want to send
  delay(100);
  mySerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}

// Access Granted
void accessGranted() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Granted");
  lcd.setCursor(0, 1);
  lcd.println("Unlocked");
  digitalWrite(relayMotor, HIGH);
  digitalWrite(buzzer, LOW);
  digitalWrite(greenLed, HIGH);
  digitalWrite(yellowLed, LOW);
  digitalWrite(redLed, LOW);
  alcState = false;
  delay(500);
}

// Access Denied
void accessDenied() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Denied");
  lcd.setCursor(0, 1);
  lcd.println("Incorrect Key");
  digitalWrite(relayMotor, LOW);
  digitalWrite(buzzer, HIGH);
  digitalWrite(greenLed, LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(redLed, HIGH);
  alcState = true;
  delay(500);
}

// Clear Password
void clearPassword() {
  lcdCursor = 0;
  userKey = "";
}

// Loading
void loading() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Loading...");
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 1);
    lcd.print("-");
    delay(50);
  }
}
