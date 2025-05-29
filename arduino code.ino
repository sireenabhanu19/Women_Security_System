#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <TinyGPSPlus.h>

#define PANIC_BUTTON_PIN 2

// GSM module on SoftwareSerial (Pin 10 = TX, 11 = RX)
SoftwareSerial sim800(10, 11);

// GPS module on AltSoftSerial (Pin 8 = RX, 9 = TX)
AltSoftSerial gpsSerial;
TinyGPSPlus gps;

String latitude = "";
String longitude = "";

void setup() {
  pinMode(PANIC_BUTTON_PIN, INPUT_PULLUP); // Panic button
  Serial.begin(9600);
  sim800.begin(9600);
  gpsSerial.begin(9600);

  Serial.println("System Initialized");
}

void loop() {
  // Read GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Check panic button
  if (digitalRead(PANIC_BUTTON_PIN) == LOW) {
    Serial.println("Panic button pressed");

    if (gps.location.isValid()) {
      latitude = String(gps.location.lat(), 6);
      longitude = String(gps.location.lng(), 6);
    } else {
      latitude = "0.000000";
      longitude = "0.000000";
    }

    sendSMS("I am in danger! Please help me!\nLocation: https://www.google.com/maps/search/?api=1&query=" + latitude + "," + longitude);
    
    delay(10000); // Prevent repeated sending
  }

  delay(100);
}

void sendSMS(String message) {
  sim800.println("AT");
  delay(500);
  sim800.println("AT+CMGF=1"); // Set text mode
  delay(500);
  sim800.println("AT+CMGS=\"+918790132148\""); // Change to your number
  delay(500);
  sim800.print(message);
  sim800.write(26); // End message (Ctrl+Z)
  delay(5000);
}
