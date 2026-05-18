#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include "SinricPro.h"
#include "SinricProSwitch.h"

// --- WiFi Credentials ---
#define WIFI_SSID         "Lemon"
#define WIFI_PASS         "6add5acd"

// --- Sinric Pro Credentials ---
#define APP_KEY           "166ccebb-455c-49fb-80c5-f67f3a40b044"
#define APP_SECRET        "ab6d35b7-ad19-48d9-bee8-715514821d87-5a4cc226-4717-4098-ae20-b9b67843647f"
#define CASA_SWITCH_ID    "6a0953a9baa50bf9bf35869c"
#define PARKING_SWITCH_ID "6a09544abaa50bf9bf358723"

// --- Hardware Pin Definitions ---
#define RELAY_CASA_PIN    23  // Controls the Relay (12V Bulb + 12V Exhaust Fan)
#define SERVO_GATE_PIN    18  // Controls the Parking Arm Gate

// --- Object Instances ---
Servo parkingServo;

// --- Servo Angles ---
const int GATE_CLOSE_ANGLE = 0;   // Angle when gate is down (Adjust if needed)
const int GATE_OPEN_ANGLE  = 90;  // Angle when gate is up (Adjust if needed)

// ==========================================
//   Sinric Pro Callback: Casa Control
// ==========================================
bool onCasaStateChange(const String &deviceId, bool &state) {
  Serial.printf("Casa Device %s turned %s\r\n", deviceId.c_str(), state ? "ON" : "OFF");
  
  if (state) {
    digitalWrite(RELAY_CASA_PIN, HIGH); // Turns ON Bulb & Fan together
  } else {
    digitalWrite(RELAY_CASA_PIN, LOW);  // Turns OFF Bulb & Fan together
  }
  return true; 
}

// ==========================================
//   Sinric Pro Callback: Parking Control
// ==========================================
bool onParkingStateChange(const String &deviceId, bool &state) {
  Serial.printf("Parking Device %s turned %s\r\n", deviceId.c_str(), state ? "OPEN" : "CLOSE");
  
  if (state) {
    parkingServo.write(GATE_OPEN_ANGLE);  // Command servo to open angle
  } else {
    parkingServo.write(GATE_CLOSE_ANGLE); // Command servo to close angle
  }
  return true; 
}

// ==========================================
//   Setup
// ==========================================
void setup() {
  Serial.begin(115200);
  
  // Initialize Pins
  pinMode(RELAY_CASA_PIN, OUTPUT);
  digitalWrite(RELAY_CASA_PIN, LOW); // Default OFF
  
  // Attach Servo
  ESP32PWM::allocateTimer(0); // Required for ESP32Servo library
  parkingServo.setPeriodHertz(50); 
  parkingServo.attach(SERVO_GATE_PIN, 500, 2400);
  parkingServo.write(GATE_CLOSE_ANGLE); // Default gate to closed on startup

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Initialize Sinric Pro Switches
  SinricProSwitch& casaSwitch = SinricPro[CASA_SWITCH_ID];
  casaSwitch.onPowerState(onCasaStateChange);

  SinricProSwitch& parkingSwitch = SinricPro[PARKING_SWITCH_ID];
  parkingSwitch.onPowerState(onParkingStateChange);

  // Start SinricPro
  SinricPro.begin(APP_KEY, APP_SECRET);
}

// ==========================================
//   Main Loop
// ==========================================
void loop() {
  SinricPro.handle(); // Keeps connection active
}