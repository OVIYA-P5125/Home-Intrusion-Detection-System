#include <SPI.h>
#include <MFRC522.h>

// RC522 Pins
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Sensor Pins
const int pirPin = 2;
const int doorPin = 3;
const int ledPin = 5;
const int buzzerPin = 6;

// Authorized RFID cards
String authorizedUIDs[] = {
  "55 6D 94 04",
  "49 42 8C 04",
  "43 29 88 F5",
  "E4 BB DB 05",
  "EA 5E D5 05",
  "93 8C 65 1A"
};

const int totalCards = 6;

bool systemArmed = false;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(pirPin, INPUT);
  pinMode(doorPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  Serial.println("Home Security System Ready");
  Serial.println("Scan RFID to ARM/DISARM");
}

void loop() {

  // ===== RFID CHECK =====
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {

    String readUID = "";

    for (byte i = 0; i < mfrc522.uid.size; i++) {
      if (mfrc522.uid.uidByte[i] < 0x10) readUID += "0";
      readUID += String(mfrc522.uid.uidByte[i], HEX);
      readUID += " ";
    }

    readUID.toUpperCase();
    readUID.trim();

    Serial.print("Scanned UID: ");
    Serial.println(readUID);

    bool accessGranted = false;

    for (int i = 0; i < totalCards; i++) {
      if (readUID == authorizedUIDs[i]) {
        accessGranted = true;
        break;
      }
    }

    if (accessGranted) {
      systemArmed = !systemArmed;

      if (systemArmed) {
        Serial.println("System ARMED");
      } else {
        Serial.println("System DISARMED");
        digitalWrite(ledPin, LOW);
        digitalWrite(buzzerPin, LOW);
      }

      delay(1000);
    } else {
      Serial.println("Unauthorized Card!");
      digitalWrite(buzzerPin, HIGH);
      delay(2000);
      digitalWrite(buzzerPin, LOW);
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  // ===== INTRUSION CHECK =====
  if (systemArmed) {

    int pirState = digitalRead(pirPin);
    int doorState = digitalRead(doorPin); // LOW = closed, HIGH = open (if wired that way)

    if (pirState == HIGH || doorState == HIGH) {
      digitalWrite(ledPin, HIGH);
      digitalWrite(buzzerPin, HIGH);
    } else {
      digitalWrite(ledPin, LOW);
      digitalWrite(buzzerPin, LOW);
    }
  }

  delay(100);
}