/*
 * ============================================================
 *  IoT-Based Pearl Farm Health Regulator
 *  File   : tds_only.ino
 *  Folder : reference_sketches/
 *  Board  : ESP32 DevKit V1
 * ============================================================
 *
 *  NOTE: This is a REFERENCE / DEVELOPMENT sketch.
 *  It was used during the early phase of the project to test
 *  the TDS sensor independently before combining all sensors.
 *  The final deployed firmware is: esp32_main_node.ino
 *
 *  Description:
 *    Reads TDS (Total Dissolved Solids) and EC (Electrical
 *    Conductivity) from the Keyestudio TDS sensor, displays
 *    values on a 16x2 LCD, and sends to Blynk + ThingSpeak.
 *
 *  Sensor:
 *    Keyestudio TDS Sensor --> GPIO 34 (ADC1 Ch6, analog)
 *    Output range: 0 – 2.3V, maps to 0 – 1000 ppm
 *
 *  Project : TIH Chanakya Fellowship, IIT Bombay
 *  Team    : Swaroop Thikane, Savita Totad, Atharav Todkar,
 *            Vaishnavi Pawar, Vedant Joshi
 *  Guide   : Dr. Jayashree P. Kharat, DKTE's TEI
 * ============================================================
 */

// ── Credentials ───────────────────────────────────────────────
#include "config.h"

// ── Blynk setup ───────────────────────────────────────────────
#define BLYNK_TEMPLATE_ID   CONF_BLYNK_TEMPLATE_ID
#define BLYNK_TEMPLATE_NAME CONF_BLYNK_TEMPLATE_NAME
#define BLYNK_AUTH_TOKEN    CONF_BLYNK_AUTH_TOKEN
#define BLYNK_PRINT Serial

// ── Libraries ─────────────────────────────────────────────────
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>

// ── LCD (16x2, I2C) ───────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ── ThingSpeak ────────────────────────────────────────────────
const char* thingSpeakServer = "api.thingspeak.com";
WiFiClient client;

// ── TDS Sensor Parameters ─────────────────────────────────────
namespace pin    { const byte tds_sensor = 34; }
namespace device { float aref = 3.3; }   // ESP32 = 3.3V (not 5V)
namespace sensor {
  float        ec            = 0;
  unsigned int tds           = 0;
  float        ecCalibration = 1;    // Adjust after calibration
}


// ============================================================
//  sendToThingSpeak() — uploads TDS and EC to ThingSpeak
// ============================================================
void sendToThingSpeak(float tds, float ec) {
  if (!client.connect(thingSpeakServer, 80)) {
    Serial.println("ThingSpeak: Connection failed.");
    return;
  }

  String body = "api_key=" + String(CONF_THINGSPEAK_API_KEY)
              + "&field1=" + String(tds)
              + "&field2=" + String(ec);

  client.print("POST /update HTTP/1.1\r\n");
  client.print("Host: api.thingspeak.com\r\n");
  client.print("Connection: close\r\n");
  client.print("Content-Type: application/x-www-form-urlencoded\r\n");
  client.print("Content-Length: " + String(body.length()) + "\r\n\r\n");
  client.print(body);
  client.stop();

  Serial.print("ThingSpeak sent — TDS: "); Serial.print(tds);
  Serial.print(" ppm, EC: "); Serial.print(ec); Serial.println(" mS/cm");
}


// ============================================================
//  readTDS()
//
//  Converts raw ADC voltage to TDS using a cubic polynomial.
//  A dry-air offset (0.14V) is subtracted to remove baseline.
//  Formula: TDS = (133.42*V^3 - 255.86*V^2 + 857.39*V) * 0.5
// ============================================================
void readTDS() {
  float rawVoltage = analogRead(pin::tds_sensor) * device::aref / 4096.0;

  Serial.print("Raw voltage: "); Serial.println(rawVoltage);

  float offset = 0.14;  // Dry-air baseline voltage
  sensor::ec = (rawVoltage * sensor::ecCalibration) - offset;
  if (sensor::ec < 0) sensor::ec = 0;

  // Cubic polynomial: voltage -> TDS (ppm)
  sensor::tds = (133.42 * pow(sensor::ec, 3)
                - 255.86 * sensor::ec * sensor::ec
                + 857.39 * sensor::ec) * 0.5;

  Serial.print("TDS: "); Serial.print(sensor::tds); Serial.println(" ppm");
  Serial.print("EC: ");  Serial.print(sensor::ec, 2); Serial.println(" mS/cm");

  // Display on LCD
  lcd.setCursor(0, 0);
  lcd.print("TDS: "); lcd.print(sensor::tds); lcd.print(" ppm  ");
  lcd.setCursor(0, 1);
  lcd.print("EC:  "); lcd.print(sensor::ec, 2); lcd.print("      ");

  // Send to Blynk
  Blynk.virtualWrite(V0, sensor::tds);
  Blynk.virtualWrite(V1, sensor::ec);

  // Send to ThingSpeak
  sendToThingSpeak(sensor::tds, sensor::ec);
}


void setup() {
  Serial.begin(115200);
  Blynk.begin(CONF_BLYNK_AUTH_TOKEN, CONF_WIFI_SSID, CONF_WIFI_PASS);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("TDS Test Node");
  delay(2000);
  lcd.clear();
}

void loop() {
  Blynk.run();
  readTDS();
  delay(1000);
}
