#include <Wire.h>
#include "DFRobot_RGBLCD1602.h"
#include <RTClib.h>
const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

// --- Pin Definitions ---
int LIGHT_PIN    = 7;    // LED = light/motor
int SECURITY_PIN = 5;    // LED = security alarm
int LDR_PIN      = A1;
int LM35_PIN     = A2;
int SERVO_PIN    = 3;    // LED = servo indicator
int FAN_PIN      = 6;    // LED = fan
int PIR_PIN      = A0;

// --- Thresholds ---
int LDR_THRESH = 20;
int PIR_THRESH = 512;

// --- LCD ---
DFRobot_RGBLCD1602 lcd(/*RGBAddr*/0x60 ,/*lcdCols*/16,/*lcdRows*/2);

// --- Variables ---
int ldrState, pirState, lm35_reading;
float temperature, voltage;

// --- Simulated Time ---
int sim_hours   = 10;
int sim_minutes = 30;
int sim_seconds = 0;
unsigned long lastTick = 0;

void updateSimTime() {
  if (millis() - lastTick >= 1000) {
    lastTick = millis();
    sim_seconds++;
    if (sim_seconds >= 60) { sim_seconds = 0; sim_minutes++; }
    if (sim_minutes >= 60) { sim_minutes = 0; sim_hours++;   }
    if (sim_hours   >= 24) { sim_hours   = 0;                }
  }
}

void setup() {
  Serial.begin(9600);
lcd.init();
lcd.setRGB(colorR, colorG, colorB);
    
    // Print a message to the LCD.
    lcd.print("hello, world!");
  pinMode(LIGHT_PIN,    OUTPUT);
  pinMode(SECURITY_PIN, OUTPUT);
  pinMode(FAN_PIN,      OUTPUT);
  pinMode(SERVO_PIN,    OUTPUT);

  digitalWrite(LIGHT_PIN,    LOW);
  digitalWrite(SECURITY_PIN, LOW);
  digitalWrite(FAN_PIN,      LOW);
  digitalWrite(SERVO_PIN,    LOW);

  lcd.init();
  //lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1500);
  lcd.clear();
}

void loop() {
  updateSimTime();

  ldrState = analogRead(LDR_PIN);
  pirState = analogRead(PIR_PIN);

  // --- Light/Motor LED ---
  if ((sim_hours > 18 || (sim_hours == 18 && sim_minutes >= 30)) ||
       sim_hours < 9) {
    digitalWrite(LIGHT_PIN, HIGH);   // ON = night time
  } else {
    digitalWrite(LIGHT_PIN, LOW);    // OFF = day time
  }

  // --- Servo LED ---
  if (sim_hours >= 18 || sim_hours < 8) {
    digitalWrite(SERVO_PIN, HIGH);   // ON = gate closed (night)
  } else {
    digitalWrite(SERVO_PIN, LOW);    // OFF = gate open (day)
  }

  // --- Security LED ---
  if ((ldrState < LDR_THRESH) || (pirState > PIR_THRESH)) {
    digitalWrite(SECURITY_PIN, HIGH);
  } else {
    digitalWrite(SECURITY_PIN, LOW);
  }

  // --- Temperature & Fan LED ---
  lm35_reading = analogRead(LM35_PIN);
  voltage      = lm35_reading * (5.0 / 1023.0);
  temperature  = voltage * 100;
  bool fan     = (temperature > 28);
  digitalWrite(FAN_PIN, fan);

  // --- LCD Row 0: Time ---
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  if (sim_hours   < 10) lcd.print("0");
  lcd.print(sim_hours);
  lcd.print(":");
  if (sim_minutes < 10) lcd.print("0");
  lcd.print(sim_minutes);
  lcd.print(":");
  if (sim_seconds < 10) lcd.print("0");
  lcd.print(sim_seconds);

  // --- LCD Row 1: Temp + Fan ---
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print("C F:");
  lcd.print(fan ? "ON " : "OFF");

  // --- Serial Debug ---
  Serial.print("Time: ");
  if (sim_hours   < 10) Serial.print("0");
  Serial.print(sim_hours);   Serial.print(":");
  if (sim_minutes < 10) Serial.print("0");
  Serial.print(sim_minutes); Serial.print(":");
  if (sim_seconds < 10) Serial.print("0");
  Serial.println(sim_seconds);

  Serial.print("Temp: ");     Serial.print(temperature, 1);
  Serial.print("C | Fan: ");  Serial.print(fan ? "ON" : "OFF");
  Serial.print(" | Light: "); Serial.print(digitalRead(LIGHT_PIN)     ? "ON"     : "OFF");
  Serial.print(" | Servo: "); Serial.print(digitalRead(SERVO_PIN)     ? "CLOSED" : "OPEN");
  Serial.print(" | LDR: ");   Serial.print(ldrState);
  Serial.print(" | PIR: ");   Serial.print(pirState);
  Serial.print(" | Alarm: "); Serial.println(digitalRead(SECURITY_PIN) ? "ALERT"  : "OK");

  delay(1000);
}