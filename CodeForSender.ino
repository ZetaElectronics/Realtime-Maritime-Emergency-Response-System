#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// Define LoRa pins
#define SCK 18
#define MISO 19
#define MOSI 23
#define SS 5
#define RST 14
#define DIO0 2

// GPS Setup
#define GPS_RX 16
#define GPS_TX 17
HardwareSerial gpsSerial(1);
TinyGPSPlus gps;

// Variables
int counter = 0;
unsigned long lastSend = 0;
const unsigned long sendInterval = 5000; // Send every 5 seconds

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  Serial.println("LoRa GPS Sender Starting...");
  
  // Initialize GPS
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("GPS Serial initialized");
  
  // Initialize LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed!");
    while (1) {
      delay(1000);
    }
  }
  
  // LoRa settings
  LoRa.setSyncWord(0xF3);
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  
  Serial.println("LoRa Sender Ready!");
  Serial.println("Waiting for GPS signal...");
}

void loop() {
  // Read GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }
  
  // Send data every interval
  if (millis() - lastSend >= sendInterval) {
    sendGPSData();
    lastSend = millis();
  }
  
  delay(100);
}

void sendGPSData() {
  String gpsData = "";
  
  if (gps.location.isValid() && gps.satellites.value() > 0) {
    // Format: LAT,LON,SAT,ALT
    gpsData = String(gps.location.lat(), 6) + "," +
              String(gps.location.lng(), 6) + "," +
              String(gps.satellites.value()) + "," +
              String(gps.altitude.meters(), 1);
    
    Serial.print("Valid GPS - ");
  } else {
    gpsData = "INVALID";
    Serial.print("Invalid GPS - ");
  }
  
  Serial.print("Sending: ");
  Serial.println(gpsData);
  
  // Display local GPS info
  Serial.print("Local GPS - ");
  if (gps.location.isValid()) {
    Serial.print("Lat: "); Serial.print(gps.location.lat(), 6);
    Serial.print(" Lon: "); Serial.print(gps.location.lng(), 6);
    Serial.print(" Sats: "); Serial.println(gps.satellites.value());
  } else {
    Serial.println("No GPS fix");
  }
  
  // Send via LoRa
  LoRa.beginPacket();
  LoRa.print(gpsData);
  LoRa.endPacket();
  
  counter++;
  Serial.println("Packet sent!");
  Serial.println("--------------------");
}