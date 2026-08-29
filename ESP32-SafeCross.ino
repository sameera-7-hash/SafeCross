#include <WiFi.h>
#include <HTTPClient.h>

// ======================================
// 📶 WIFI
// ======================================

const char* ssid = "POCO X7 Pro";
const char* password = "123456789";

// ======================================
// 🔥 FIREBASE
// ======================================

String firebaseURL =
"https://zebra-crossing-cae7f-default-rtdb.firebaseio.com/zebra.json";

// ======================================
// 🔌 ORIGINAL PINS
// ======================================

// Ultrasonic
#define TRIG 14
#define ECHO 15

// IR
#define IR 13

// LEDs
#define PED_RED 12
#define PED_GREEN 2 //VIOLET

#define CAR_RED 4
#define CAR_GREEN 18

// Buzzer
#define BUZZER 23

// ======================================
// VARIABLES
// ======================================

long duration;
int distance;

int vehicleCount = 0;
int pedCount = 0;
int violationCount = 0;

bool lastVehicle = false;
bool lastPed = false;
bool lastViolation = false;

unsigned long lastSendTime = 0;

// ======================================
// 🚗 VEHICLE BUZZER
// ======================================

void vehicleBeep() {

  digitalWrite(BUZZER, HIGH);
  delay(70);

  digitalWrite(BUZZER, LOW);
  delay(120);
}

// ======================================
// 🚶 PEDESTRIAN BUZZER
// ======================================

void pedestrianBeep() {

  digitalWrite(BUZZER, HIGH);
  delay(40);

  digitalWrite(BUZZER, LOW);
  delay(40);

  digitalWrite(BUZZER, HIGH);
  delay(40);

  digitalWrite(BUZZER, LOW);
  delay(120);
}

// ======================================
// ⚠️ VIOLATION BUZZER
// ======================================

void violationBeep() {

  for(int i=0; i<4; i++){

    digitalWrite(BUZZER, HIGH);
    delay(35);

    digitalWrite(BUZZER, LOW);
    delay(35);
  }
}

// ======================================
// 🌐 WIFI CONNECT
// ======================================

void connectWiFi() {

  Serial.print("Connecting WiFi");

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){

    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi Connected");
  Serial.println(WiFi.localIP());
}

// ======================================
// 🔥 FIREBASE FUNCTION
// ======================================

void sendToFirebase() {

  if(WiFi.status() == WL_CONNECTED){

    HTTPClient http;

    http.setTimeout(2000);

    http.begin(firebaseURL);

    http.addHeader("Content-Type", "application/json");

    String json = "{";

    json += "\"vehicles\":" + String(vehicleCount) + ",";
    json += "\"pedestrians\":" + String(pedCount) + ",";
    json += "\"violations\":" + String(violationCount);

    json += "}";

    int response = http.PUT(json);

    Serial.print("🔥 Firebase Response: ");
    Serial.println(response);

    http.end();
  }
}

// ======================================
// SETUP
// ======================================

void setup() {

  Serial.begin(115200);

  // Ultrasonic
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // IR
  pinMode(IR, INPUT_PULLUP);

  // LEDs
  pinMode(PED_RED, OUTPUT);
  pinMode(PED_GREEN, OUTPUT);

  pinMode(CAR_RED, OUTPUT);
  pinMode(CAR_GREEN, OUTPUT);

  // Buzzer
  pinMode(BUZZER, OUTPUT);

  // Initially OFF
  digitalWrite(PED_RED, LOW);
  digitalWrite(PED_GREEN, LOW);

  digitalWrite(CAR_RED, LOW);
  digitalWrite(CAR_GREEN, LOW);

  digitalWrite(BUZZER, LOW);

  // WiFi
  connectWiFi();

  Serial.println("🚦 SMART ZEBRA SYSTEM READY");
}

// ======================================
// LOOP
// ======================================

void loop() {

  // ======================================
  // ULTRASONIC SENSOR
  // ======================================

  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG, LOW);

  duration = pulseIn(ECHO, HIGH, 10000);

  if(duration == 0){

    distance = 999;

  } else {

    distance = duration * 0.034 / 2;
  }

  // ======================================
  // IR SENSOR
  // ======================================

  int pedestrian = digitalRead(IR);

  bool vehicleDetected = (distance > 0 && distance < 70);

  bool pedDetected = (pedestrian == LOW);

  // ======================================
  // DEBUG
  // ======================================

  Serial.println("----------------");

  Serial.print("Distance: ");
  Serial.println(distance);

  Serial.print("IR: ");
  Serial.println(pedestrian);

  // ======================================
  // ⚠️ VIOLATION MODE
  // ======================================

  if(vehicleDetected && pedDetected){

    Serial.println("⚠️ VIOLATION DETECTED");

    // Count once
    if(!lastViolation){

      violationCount++;

      lastViolation = true;
    }

    lastVehicle = false;
    lastPed = false;

    // ❌ ALL LIGHTS OFF
    digitalWrite(PED_RED, LOW);
    digitalWrite(PED_GREEN, LOW);

    digitalWrite(CAR_RED, LOW);
    digitalWrite(CAR_GREEN, LOW);

    // ⚠️ ALERT SOUND
    violationBeep();
  }

  // ======================================
  // 🚶 PEDESTRIAN MODE
  // ======================================

  else if(pedDetected){

    Serial.println("🚶 Pedestrian Crossing");

    lastViolation = false;
    lastVehicle = false;

    // 🚗 RED ON
    digitalWrite(CAR_RED, HIGH);
    digitalWrite(CAR_GREEN, LOW);

    // 🚶 GREEN ON
    digitalWrite(PED_GREEN, HIGH);
    digitalWrite(PED_RED, LOW);

    // Count once
    if(!lastPed){

      pedCount++;

      lastPed = true;
    }

    // 🚶 CONTINUOUS SOUND
    pedestrianBeep();
  }

  // ======================================
  // 🚗 VEHICLE MODE
  // ======================================

  else if(vehicleDetected){

    Serial.println("🚗 Vehicle Detected");

    lastViolation = false;
    lastPed = false;

    // 🚶 RED ON
    digitalWrite(PED_RED, HIGH);
    digitalWrite(PED_GREEN, LOW);

    // 🚗 GREEN ON
    digitalWrite(CAR_GREEN, HIGH);
    digitalWrite(CAR_RED, LOW);

    // Count once
    if(!lastVehicle){

      vehicleCount++;

      lastVehicle = true;
    }

    // 🚗 CONTINUOUS SOUND
    vehicleBeep();
  }

  // ======================================
  // ⚫ NO ACTIVITY
  // ======================================

  else {

    Serial.println("⚫ No Activity");

    // ALL OFF
    digitalWrite(PED_RED, LOW);
    digitalWrite(PED_GREEN, LOW);

    digitalWrite(CAR_RED, LOW);
    digitalWrite(CAR_GREEN, LOW);

    digitalWrite(BUZZER, LOW);

    lastVehicle = false;
    lastPed = false;
    lastViolation = false;
  }

  // ======================================
  // 🔥 FIREBASE UPDATE
  // ======================================

  if(millis() - lastSendTime > 5000){

    sendToFirebase();

    lastSendTime = millis();
  }

  // FAST LOOP
  delay(20);
}