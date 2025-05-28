#include <Wire.h>
#include <GyverOLED.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <AltSoftSerial.h> // Use AltSoftSerial for GPS
#include <Wire.h>

// OLED instance
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

MAX30105 particleSensor;
// GSM setup
SoftwareSerial sim800(10, 11); // TX, RX
String Lat = "";
String Lng = "";

const int panic = 2;
//const int metal = 3;
//const int vibration = 2;
//const int lm35 = A0;
const int buzzer = 6;

// GPS setup with AltSoftSerial
AltSoftSerial neogps; // RX = Pin 8, TX = Pin 9
int count = 1;
TinyGPSPlus gps;
const byte RATE_SIZE = 4; // Array size for averaging BPM
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0; // Time of last beat

float beatsPerMinute;
int beatAvg;
int flag = 1;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Initialize OLED
  
  // Initialize MAX30105 sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 not found. Check wiring.");
    //while (1);
  }
  
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);
  sim800.begin(9600);
  oled.init();
  oled.clear();
  oled.setScale(2);
  oled.setCursor(0, 0);
  oled.print("BPM Monitor");

 // Serial.println("GSM: Ready");

  // Initialize GPS module
  neogps.begin(9600);
 // Serial.println("GPS: Ready");

  // Initialize buzzer and panic button
//  pinMode(vibration, INPUT);
  pinMode(panic, INPUT_PULLUP);
  //pinMode(metal, INPUT);
  //pinMode(buzzer, OUTPUT);

}

void loop() {

  while (neogps.available() > 0) {
    if (gps.encode(neogps.read()))
      displayInfo();
  }
  // Check panic button
  int btn = digitalRead(panic);
  //Serial.println(btn);
  if(btn == 0) {
    Serial.println("-1");
    digitalWrite(buzzer, HIGH);
     sendSMS("I am in Danger");
        count = 1;
  }
    

  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];

      beatAvg += 25;
      beatAvg /= RATE_SIZE;
      if(flag == 0){
        //Serial.print("Beat,");
        Serial.println(beatAvg);
        oled.clear();
        oled.setCursor(0, 1);
        //oled.print("BPM: ");
        oled.print(beatAvg);
      }
      //Serial.print("Beat,");
      //Serial.println(beatAvg); 
     
      //oled.print(beatsPerMinute);
    }
  }
  if (irValue < 50000){
    //Serial.print(" No finger?");
    oled.setCursor(0, 4);
    oled.print("No Finger");
    flag = 1;
    //Serial.println(-1);
    //oled.print(beatAvg);
  }else{
    flag = 0;
  }
  delay(50);
}

void displayInfo() {
  if (gps.location.isValid()) {
    Lat = String(gps.location.lat(), 6);
    Lng = String(gps.location.lng(), 6);
   //Serial.print("Lat: ");
   Serial.print(Lat);
   Serial.print(",");
   Serial.println(Lng);
   oled.setCursor(0, 6);
   oled.println("GPS        ");
  } else {
     //Serial.println("GPS: No Signal");
    oled.setCursor(0, 6);
    oled.print("No GPS");
  }
}

void sendSMS(String mesg) {
  //Serial.println("Sending SMS...");
  sim800.println("AT"); // Check if GSM module is responding
  updateSerial();
 // sim800.println("AT+CBAND?");
  //updateSerial();
  // sim800.println("AT+CBAND=DCS_MODE");
  //updateSerial();
  sim800.println("AT+CMGF=1"); // Set SMS mode to text
  updateSerial();
  sim800.println("AT+CMGS=\"+918790132148\""); // Set recipient number
  updateSerial();
  sim800.print(mesg + " please help me! \n tap the link to save me :\nhttps://www.google.com/maps/search/?api=1&query=" + Lat + "," + Lng); // SMS content
  updateSerial();
  sim800.write(26); // Send CTRL+Z to indicate end of message
  delay(5000);

  sim800.println("AT"); // Check if GSM module is responding
  updateSerial();
  sim800.println("AT+CMGF=1"); // Set SMS mode to text
  updateSerial();
  sim800.println("AT+CMGS=\"+918688190131\""); // Set recipient number
  updateSerial();
  sim800.print(mesg + " please help me! \n tap the link to save me :\nhttps://www.google.com/maps/search/?api=1&query=" + Lat + "," + Lng); // SMS content
  updateSerial();
  sim800.write(26); // Send CTRL+Z to indicate end of message
  delay(5000);
}

void updateSerial() {
  delay(500);
  //while (Serial.available()) {
    //sim800.write(Serial.read()); // Forward Serial data to GSM module
  //}
  //while (sim800.available()) {
    //Serial.write(sim800.read()); // Forward GSM module data to Serial
 // }
}
