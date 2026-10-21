#include <Wire.h>
#include <RTClib.h>
#include <Servo.h>
#include <LiquidCrystal.h>

// --- Pin Definitions 
const int PIR_PIN       = 2;   // PIR motion sensor 
const int SERVO_PIN     = 6;   // Curtain Servo 
const int NIGHT_LED_PIN = 7;   // Additional LED: On all night, off in light
const int SEC_LIGHT_PIN = 5;   // Security Light 
const int FAN_PIN       = 13;  // Fan Control (Moved to avoid LCD conflict) 
const int LIGHTS_PIN    = A2;  // Main Lights (Moved to avoid LCD conflict) 
const int LDR_PIN       = A0;  // LDR Sensor 
const int LM35_PIN      = A1;  // Temperature Sensor 

// --- LCD Pin Mapping (Standard 16-pin Parallel) 
// RS=12, En=11, D4=10, D5=9, D6=8, D7=4
LiquidCrystal lcd(12, 11, 10, 9, 8, 4);

RTC_DS1307 rtc;
Servo curtainServo;

void setup() {
  Serial.begin(9600);
  
  pinMode(PIR_PIN, INPUT);
  pinMode(LIGHTS_PIN, OUTPUT);
  pinMode(SEC_LIGHT_PIN, OUTPUT);
  pinMode(NIGHT_LED_PIN, OUTPUT); // Initialize new LED pin
  pinMode(FAN_PIN, OUTPUT);
  
  curtainServo.attach(SERVO_PIN);
  lcd.begin(16, 2);

  // Initialize RTC with Error Check
  if (!rtc.begin()) {
    lcd.print("RTC Error!");
    while(1); // Do not proceed if RTC is missing
  }

  // Set time automatically to compilation time (Uncomment if needed for physical build)
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  lcd.print("System Ready");
  delay(1500);
  lcd.clear();
}

void loop() {
  DateTime now = rtc.now(); // Acquisition of current time 
  
  // --- 1. DATA ACQUISITION ---
  int ldrValue = analogRead(LDR_PIN);
  int tempReading = analogRead(LM35_PIN);
  // Accurate Temp Conversion: (Reading * 5V / 1024) * 100 for LM35
  float temperature = (tempReading * 5.0 * 100.0) / 1024.0; 
  bool motion = digitalRead(PIR_PIN);

  // --- 2. AUTOMATIC MAIN LIGHTS (6:30 PM - 9:00 PM) ---
  if ((now.hour() == 18 && now.minute() >= 30) || (now.hour() > 18 && now.hour() < 21)) {
    digitalWrite(LIGHTS_PIN, HIGH);
  } else {
    digitalWrite(LIGHTS_PIN, LOW);
  }

  // --- 3. CURTAIN CONTROL (8 AM Open, 6 PM Close) ---
  if (now.hour() >= 8 && now.hour() < 18) {
    curtainServo.write(90); 
  } else {
    curtainServo.write(0);
  }

  // --- 4. SECURITY LIGHTS (Trigger on Motion + Dark) ---
  if (ldrValue < 300 && motion == HIGH) {
    digitalWrite(SEC_LIGHT_PIN, HIGH);
  } else {
    digitalWrite(SEC_LIGHT_PIN, LOW);
  }

  // --- 5. CLIMATE CONTROL (Temp > 28°C) ---
  if (temperature > 28.0) {
    digitalWrite(FAN_PIN, HIGH);
  } else {
    digitalWrite(FAN_PIN, LOW);
  }

  // --- 6. LCD STATUS DISPLAY ---
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(temperature, 1); lcd.print("C  ");
  
  if(now.hour() < 10) lcd.print("0");
  lcd.print(now.hour());
  lcd.print(":");
  if(now.minute() < 10) lcd.print("0");
  lcd.print(now.minute());

  lcd.setCursor(0, 1);
  lcd.print("F:"); lcd.print(digitalRead(FAN_PIN) ? "ON " : "OFF");
  lcd.print(" M:"); lcd.print(motion ? "DET" : "OK ");
  lcd.print(" L:"); lcd.print(ldrValue/10); 

  // --- 7. NIGHT LED LOGIC (On when dark, Off when light) ---
  if (ldrValue < 300) {
    digitalWrite(NIGHT_LED_PIN, HIGH);
  } else {
    digitalWrite(NIGHT_LED_PIN, LOW);
  }

  delay(1000); 
}