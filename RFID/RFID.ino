#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// RFID pins
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

// LCD 16x2 I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); // changer 0x27 si ton adresse est différente

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Scan RFID Card");
}

void loop() {
  // Vérifier s'il y a une carte
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  // Lire UID
  String uidStr = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uidStr += "0";
    uidStr += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();

  // Afficher sur LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Card UID:");
  lcd.setCursor(0,1);
  lcd.print(uidStr);

  // Afficher aussi sur Serial Monitor
  Serial.println("Card UID: " + uidStr);

  delay(2000); // attendre avant lecture suivante
}
