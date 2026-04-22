//Iot_Base_Smart_Energy_Meter_code_By_Rahul
// ================= BLYNK CONFIG =================
#define BLYNK_TEMPLATE_ID "TMPLxxxx"
#define BLYNK_TEMPLATE_NAME "Smart Energy Meter"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"
// ================= LIBRARIES =================
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "ACS712.h"
#include <ZMPT101B.h>
#include <EEPROM.h>
// ================= WIFI =================
char ssid[] = "XXXXXXXXXX";
char pass[] = "XXXXXXXXXX";
// ================= HARDWARE =================
ACS712 ACS(34, 3.3, 4095, 185);     // ACS712-05B
ZMPT101B voltageSensor(35, 50.0);
LiquidCrystal_I2C lcd(0x27, 16, 2);
// ================= VARIABLES =================
float voltage = 0, current = 0, power = 0;
float energy = 0, cost = 0;
float rate = 7.0;              // per unit
float calibrationFactor = 0.035; // FINAL calibrated
unsigned long lastTime = 0;
unsigned long lastEEPROM = 0;
#define EEPROM_SIZE 512
#define ADDR 0
// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("WELCOME");
  delay(1500);
  lcd.clear();
  lcd.print("SMART ENERGY");
  lcd.setCursor(0,1);
  lcd.print("METER");
  delay(1500);
  lcd.clear();
  lcd.print("BY RAHUL");
  delay(1500);
  energy = EEPROM.readFloat(ADDR);
  if (isnan(energy)) energy = 0;
  ACS.autoMidPoint();
  voltageSensor.setSensitivity(520.0f);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}
// ================= LOOP =================
void loop() {
  Blynk.run();
  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  lastTime = now;
  // ===== CURRENT (Filtered + Noise Removed) =====
  float avg = 0;
  for (int i = 0; i < 300; i++) {
    avg += ACS.mA_AC();
  }
  float mA = avg / 300.0;
  // noise cutoff
  if (mA < 120) mA = 0;
  current = (mA / 1000.0) * calibrationFactor;
  // ===== VOLTAGE =====
  voltage = voltageSensor.getRmsVoltage();
  // ===== POWER =====
  if (current == 0) power = 0;
  else power = voltage * current;
  // ===== ENERGY (kWh) =====
  energy += (power * dt) / 3600000.0;
  // ===== COST =====
  cost = energy * rate;
  // ===== LCD DISPLAY =====
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("V:");
  lcd.print(voltage,1);
  lcd.print(" I:");
  lcd.print(current,2);
  lcd.setCursor(0,1);
  lcd.print("P:");
  lcd.print(power,0);
  lcd.print(" U:");
  lcd.print(energy,2);
  lcd.print(" Rs:");
  lcd.print(cost,1);
  // ===== EEPROM SAVE =====
  if (millis() - lastEEPROM > 60000) {
    EEPROM.writeFloat(ADDR, energy);
    EEPROM.commit();
    lastEEPROM = millis();
  }
  // ===== BLYNK =====
  Blynk.virtualWrite(V0, voltage);
  Blynk.virtualWrite(V1, current);
  Blynk.virtualWrite(V2, power);
  Blynk.virtualWrite(V3, energy);
  Blynk.virtualWrite(V4, cost);
  delay(500);
}