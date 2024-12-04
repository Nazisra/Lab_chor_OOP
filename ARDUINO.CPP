
#include <SPI.h>
#include <MFRC522.h>
#include <OnewireKeypad.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

// LCD Configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Servo Configuration
Servo servo;
int servoPos = 0;

// Sensor and Buzzer Pins
#define sensorPin1 A3
#define sensorPin2 A2
#define buzzerPin A1

int senVal1 = 0;
int senVal2 = 0;

// RFID Configuration
#define RST_PIN 8
#define SS_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Balances for RFID Cards
int card1Balance = 5000;
int card2Balance = 300;

// Keypad Configuration
#define num 7
char Data[num];
byte data_count = 0;
char KEYS[] = {
  '1', '2', '3', 'A',
  '4', '5', '6', 'B',
  '7', '8', '9', 'C',
  '*', '0', '#', 'D'
};
OnewireKeypad <Print, 16 > KP2(Serial, KEYS, 4, 4, A0, 4700, 1000, ExtremePrec);

String num1, num2, card;
int state = 0; // Servo state (0 = Down, 1 = Up)
bool recharge = true;

// Keypad Input
char Key;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  
  servo.attach(9);
  servo.write(30); // Initial servo position
  
  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);
  pinMode(buzzerPin, OUTPUT);

  KP2.SetKeypadVoltage(5.0);
  SPI.begin();
  mfrc522.PCD_Init();

  lcd.setCursor(0, 0);
  lcd.print(" Automatic toll");
  lcd.setCursor(0, 1);
  lcd.print("collection system");
  delay(3000);
  lcd.clear();
}

void loop() {
  if (!recharge) {
    reCharge();
  } else {
    lcd.setCursor(0, 0);
    lcd.print("   Welcome!!!");
    sensorRead();
    rfid();
    KeyPad();

    if (senVal1 == 0) {
      servoDown();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Vehicle detected");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Put your card to");
      lcd.setCursor(0, 1);
      lcd.print("the reader......");
      delay(2000);
      lcd.clear();
    } else if (senVal2 == 0 && state == 1) {
      servoUp();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Have a safe");
      lcd.setCursor(0, 1);
      lcd.print("journey");
      delay(1000);
      lcd.clear();
      state = 0;
    }
  }
}

void servoDown() {
  servo.attach(9);
  for (servoPos = 30; servoPos <= 120; servoPos += 1) {
    servo.write(servoPos);
    delay(5);
  }
}

void servoUp() {
  servo.attach(9);
  for (servoPos = 120; servoPos >= 30; servoPos -= 1) {
    servo.write(servoPos);
    delay(5);
  }
}

void sensorRead() {
  senVal1 = digitalRead(sensorPin1);
  senVal2 = digitalRead(sensorPin2);
}

void rfid() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();  // Convert to uppercase
  content.trim();         // Remove leading/trailing spaces

  // Print the card UID to Serial Monitor for debugging
  Serial.print("Card UID: ");
  Serial.println(content);

  // Check against valid RFID card UIDs
  if (content == "93 D7 71 BD" || content == "A2 0D 3F 1D" || content == "04 51 58 6A 21 76 80") {
    Serial.println("Access granted: Valid card detected.");
    handleTransaction(card1Balance);  // Or card2Balance depending on the card
  } else {
    Serial.println("Access denied: Unknown vehicle.");
    denyAccess();
  }
}



void handleTransaction(int &balance) {
  if (balance >= 500) {
    lcdPrint();
    balance -= 500;
    lcd.setCursor(9, 1);
    lcd.print(balance);
    delay(2000);
    lcd.clear();
    state = 1;
  } else {
    lcdInsufficientBalance(balance);
    state = 0;
  }
}

void denyAccess() {
  digitalWrite(buzzerPin, HIGH);
  lcd.setCursor(0, 0);
  lcd.print("Unknown Vehicle");
  lcd.setCursor(0, 1);
  lcd.print("Access denied");
  delay(1500);
  lcd.clear();
  digitalWrite(buzzerPin, LOW);
}

void KeyPad() {
  byte KState = KP2.Key_State();
  if (KState == PRESSED) {
    Key = KP2.Getkey();
    if (Key && Key == 'A') {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Recharging Mode.");
      delay(1500);
      lcd.clear();
      recharge = false;
    }
  }
}

void reCharge() {
  lcd.setCursor(0, 0);
  lcd.print("Enter amount:");
  byte KState = KP2.Key_State();


if (KState == PRESSED) {
    Key = KP2.Getkey();
    if (Key) {
      if (Key == 'D') {
        if (card == "") {
          addBalance(card1Balance);
        } else if (card == "F4 C2 37 BB") {
          addBalance(card2Balance);
        }
      } else {
        Data[data_count] = Key;
        lcd.setCursor(data_count, 1);
        lcd.print(Data[data_count]);
        data_count++;
      }
    }
  }
}

void addBalance(int &balance) {
  String enteredAmount = Data;
  balance += enteredAmount.toInt();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("New Balance:");
  lcd.setCursor(0, 1);
  lcd.print(balance);
  lcd.print(" Tk");
  delay(3000);
  clearData();
  recharge = true;
}

void clearData() {
  while (data_count != 0) {
    Data[data_count--] = 0;
  }
}

void lcdPrint() {
  digitalWrite(buzzerPin, HIGH);
  delay(200);
  digitalWrite(buzzerPin, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Successfully");
  lcd.setCursor(0, 1);
  lcd.print(" paid your bill");
  delay(1500);
  lcd.clear();
}

void lcdInsufficientBalance(int balance) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Balance Low");
  lcd.setCursor(0, 1);
  lcd.print("Remaining: ");
  lcd.print(balance);
  lcd.print(" Tk");
  delay(2000);
  lcd.clear();
}
