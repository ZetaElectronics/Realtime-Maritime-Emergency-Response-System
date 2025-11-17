#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// Define LoRa pins
#define SCK 18
#define MISO 19
#define MOSI 23
#define SS 5
#define RST 14
#define DIO0 2

// Buzzer pin
#define BUZZER_PIN 25

// LCD Setup
LiquidCrystal_I2C lcd(0x24, 20, 4); // Change address if needed

// GPS Setup
#define GPS_RX 16
#define GPS_TX 17
HardwareSerial gpsSerial(1);
TinyGPSPlus gps;

// GPS data variables
float remoteLatitude = 0;
float remoteLongitude = 0;
float localLatitude = 0;
float localLongitude = 0;
int satellites = 0;
float altitude = 0;
String status = "Waiting...";
unsigned long lastUpdate = 0;
int messageCount = 0;
bool collisionAlert = false;
bool displayCollisionScreen = false;

// Collision detection parameters
const float COLLISION_DISTANCE_METERS = 50.0; // 50 meters threshold
const float COLLISION_SCREEN_THRESHOLD = 20.0; // 20 meters threshold for collision screen

void setup() {
  Serial.begin(115200);
  Serial.println("LoRa GPS Receiver with Collision Detection - Starting...");
  
  // Initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize I2C for LCD
  Wire.begin();
  delay(100);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  
  // Initialize GPS
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("GPS Serial initialized");
  
  // Initialize LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa initialization failed!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LoRa FAILED!");
    while (1) {
      delay(1000);
    }
  }
  
  // LoRa settings
  LoRa.setSyncWord(0xF3);
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  
  Serial.println("System initialized successfully!");
  
  // Display ready message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GPS Collision Detect");
  lcd.setCursor(0, 1);
  lcd.print("Waiting for GPS...");
  lcd.setCursor(0, 2);
  lcd.print("Local: --");
  lcd.setCursor(0, 3);
  lcd.print("Remote: --");
}

void loop() {
  // Read local GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }
  
  // Update local position if available
  if (gps.location.isUpdated()) {
    localLatitude = gps.location.lat();
    localLongitude = gps.location.lng();
    updateLocalDisplay();
  }
  
  // Check for incoming LoRa packets
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    onReceive(packetSize);
  }
  
  // Check for collision
  checkCollision();
  
  // Update display based on collision status
  updateDisplayBasedOnCollision();
  
  // Check if data is stale
  if (millis() - lastUpdate > 30000 && status != "Waiting...") {
    status = "No signal";
    collisionAlert = false;
    updateNormalDisplay();
  }
  
  // Handle buzzer for collision alert
  handleBuzzer();
  
  delay(100);
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;
  
  messageCount++;
  String receivedData = "";
  
  // Read the packet
  while (LoRa.available()) {
    receivedData += (char)LoRa.read();
  }
  
  Serial.print("Message #");
  Serial.print(messageCount);
  Serial.print(": ");
  Serial.println(receivedData);
  Serial.print("RSSI: ");
  Serial.println(LoRa.packetRssi());
  
  processGPSData(receivedData);
  lastUpdate = millis();
}

void processGPSData(String data) {
  data.trim();
  
  if (data == "INVALID") {
    status = "INVALID GPS";
    remoteLatitude = 0;
    remoteLongitude = 0;
    satellites = 0;
    altitude = 0;
  } else {
    // Parse CSV data: LAT,LON,SAT,ALT
    int firstComma = data.indexOf(',');
    int secondComma = data.indexOf(',', firstComma + 1);
    int thirdComma = data.indexOf(',', secondComma + 1);
    
    if (firstComma != -1 && secondComma != -1 && thirdComma != -1) {
      remoteLatitude = data.substring(0, firstComma).toFloat();
      remoteLongitude = data.substring(firstComma + 1, secondComma).toFloat();
      satellites = data.substring(secondComma + 1, thirdComma).toInt();
      altitude = data.substring(thirdComma + 1).toFloat();
      status = "Valid GPS";
      Serial.println("Remote GPS Data Parsed Successfully");
    } else {
      status = "Data Error";
      Serial.println("Error: Could not parse GPS data");
    }
  }
  
  updateNormalDisplay();
}

void checkCollision() {
  if (localLatitude == 0 || localLongitude == 0 || remoteLatitude == 0 || remoteLongitude == 0) {
    collisionAlert = false;
    displayCollisionScreen = false;
    return;
  }
  
  // Calculate distance between local and remote positions
  float distance = calculateDistance(localLatitude, localLongitude, remoteLatitude, remoteLongitude);
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" meters");
  
  if (distance <= COLLISION_DISTANCE_METERS) {
    collisionAlert = true;
    if (distance <= COLLISION_SCREEN_THRESHOLD) {
      displayCollisionScreen = true;
    } else {
      displayCollisionScreen = false;
    }
    Serial.println("COLLISION ALERT! Vehicles too close!");
  } else {
    collisionAlert = false;
    displayCollisionScreen = false;
  }
}

void updateDisplayBasedOnCollision() {
  static bool lastCollisionScreenState = false;
  
  // Only update the display when the state changes to prevent flickering
  if (displayCollisionScreen != lastCollisionScreenState) {
    if (displayCollisionScreen) {
      showCollisionScreen();
    } else {
      updateNormalDisplay();
    }
    lastCollisionScreenState = displayCollisionScreen;
  }
}

void showCollisionScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("INCOMING COLLISION");
  lcd.setCursor(0, 1);
  lcd.print("Vessel Collision");
  lcd.setCursor(0, 2);
  lcd.print("Detected!");
  lcd.setCursor(0, 3);
  lcd.print("TAKE EVASIVE ACTION");
}

void updateNormalDisplay() {
  if (displayCollisionScreen) return; // Don't update if collision screen is active
  
  lcd.clear();
  
  // Line 1: Local Latitude
  lcd.setCursor(0, 0);
  lcd.print("Lat1:");
  if (localLatitude != 0) {
    lcd.print(localLatitude, 6);
  } else {
    lcd.print("---");
  }
  
  // Line 2: Local Longitude
  lcd.setCursor(0, 1);
  lcd.print("Lon1:");
  if (localLongitude != 0) {
    lcd.print(localLongitude, 6);
  } else {
    lcd.print("---");
  }
  
  // Line 3: Remote Latitude
  lcd.setCursor(0, 2);
  lcd.print("Lat2:");
  if (remoteLatitude != 0) {
    lcd.print(remoteLatitude, 6);
    // Add spaces to clear any leftover characters
    lcd.print("  ");
  } else {
    lcd.print("---");
  }
  
  // Line 4: Remote Longitude
  lcd.setCursor(0, 3);
  lcd.print("Lon2:");
  if (remoteLongitude != 0) {
    lcd.print(remoteLongitude, 6);
  } else {
    lcd.print("---");
  }
  
  // Show status on second line if space
  lcd.setCursor(15, 1);
  lcd.print("S:");
  lcd.print(satellites);
}

float calculateDistance(float lat1, float lon1, float lat2, float lon2) {
  // Haversine formula to calculate distance between two coordinates
  const float R = 6371000; // Earth radius in meters
  float phi1 = lat1 * PI / 180;
  float phi2 = lat2 * PI / 180;
  float deltaPhi = (lat2 - lat1) * PI / 180;
  float deltaLambda = (lon2 - lon1) * PI / 180;
  
  float a = sin(deltaPhi/2) * sin(deltaPhi/2) +
            cos(phi1) * cos(phi2) *
            sin(deltaLambda/2) * sin(deltaLambda/2);
  float c = 2 * atan2(sqrt(a), sqrt(1-a));
  
  return R * c;
}

void handleBuzzer() {
  if (collisionAlert) {
    // Beep pattern for collision alert
    static unsigned long lastBeep = 0;
    static bool beepState = false;
    
    if (millis() - lastBeep > 200) { // Fast beeping (5 times per second)
      beepState = !beepState;
      digitalWrite(BUZZER_PIN, beepState ? HIGH : LOW);
      lastBeep = millis();
    }
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

void updateLocalDisplay() {
  // Update only local GPS part of display if normal display is active
  if (!displayCollisionScreen) {
    lcd.setCursor(7, 2);
    if (localLatitude != 0) {
      lcd.print("OK  ");
    } else {
      lcd.print("NO  ");
    }
  }
}

void serialPrintStatus() {
  Serial.println("=== GPS STATUS ===");
  Serial.print("Local - Lat: "); Serial.print(localLatitude, 6);
  Serial.print(" Lon: "); Serial.println(localLongitude, 6);
  Serial.print("Remote - Lat: "); Serial.print(remoteLatitude, 6);
  Serial.print(" Lon: "); Serial.println(remoteLongitude, 6);
  Serial.print("Collision Alert: "); Serial.println(collisionAlert ? "YES" : "NO");
  Serial.print("Collision Screen: "); Serial.println(displayCollisionScreen ? "YES" : "NO");
  Serial.println("==================");
}