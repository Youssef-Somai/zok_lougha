#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <LiquidCrystal.h>
#include <Wire.h>


#define RST_PIN 9
#define SS_PIN 10
#define LED_GREEN 6
#define LED_RED 5
#define SERVO_PIN 6

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myServo;
LiquidCrystal lcd(8, 9, 4, 5, 2, 3); // تعديل حسب التوصيل

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  myServo.attach(SERVO_PIN);
  myServo.write(0); // Servo في الوضع المغلق
  lcd.begin(16,2);
  lcd.print("Place your card");
}

void loop() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) return;
  if ( ! mfrc522.PICC_ReadCardSerial()) return;

  String rfidID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    rfidID += String(mfrc522.uid.uidByte[i], HEX);
  }
  rfidID.toUpperCase();

  Serial.println(rfidID);  // يبعث للـ Qt

  // انتظار نتيجة من Qt
  while (Serial.available() == 0) {}
  String response = Serial.readStringUntil('\n');
  response.trim();

  if(response == "OK") {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
    myServo.write(90); // فتح الباب
    lcd.clear();
    lcd.print("Bienvenue");
    delay(3000);
    myServo.write(0); // رجوع السيرفو
    digitalWrite(LED_GREEN, LOW);
  } else {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
    lcd.clear();
    lcd.print("Invalid");
    delay(3000);
    digitalWrite(LED_RED, LOW);
  }
}
