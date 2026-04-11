/*
 * ============================================================
 *  IoT-Based Pearl Farm Health Regulator
 *  File   : tds_ph_combined.ino
 *  Folder : reference_sketches/
 *  Board  : ESP32 DevKit V1
 * ============================================================
 *
 *  NOTE: This is a REFERENCE / DEVELOPMENT sketch.
 *  This was the mid-iteration version combining TDS and pH
 *  before the DS18B20 temperature sensor was integrated.
 *  The final deployed firmware is: esp32_main_node.ino
 *
 *  Description:
 *    Reads TDS (GPIO 34) and pH (GPIO 33) simultaneously,
 *    displays on 16x2 LCD, uploads to Blynk + ThingSpeak.
 *
 *  Blynk Virtual Pins: V0=TDS, V1=EC, V2=pH
 *  ThingSpeak Fields:  field1=TDS, field2=EC, field3=pH
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

// ── Pin Definitions ───────────────────────────────────────────
#define PH_SENSOR_PIN   33   // DFRobot pH sensor (ADC1 Ch5)
#define TDS_SENSOR_PIN  34   // Keyestudio TDS sensor (ADC1 Ch6)

// ── LCD (16x2, I2C) ───────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ── ThingSpeak ────────────────────────────────────────────────
const char* thingSpeakServer = "api.thingspeak.com";
WiFiClient client;

// ── TDS Variables ─────────────────────────────────────────────
namespace device { float aref = 3.3; }
namespace sensor {
  float        ec            = 0;
  unsigned int tds           = 0;
  float        ecCalibration = 1;
}

// ── pH Variable ───────────────────────────────────────────────
float phValue = 0.0;


// ============================================================
//  sendToThingSpeak() — uploads TDS, EC and pH
// ============================================================
void sendToThingSpeak(float tds, float ec, float pH) {
  if (!client.connect(thingSpeakServer, 80)) {
    Serial.println("ThingSpeak: Connection failed.");
    return;
  }

  String body = "api_key=" + String(CONF_THINGSPEAK_API_KEY)
              + "&field1=" + String(tds)
              + "&field2=" + String(ec)
              + "&field3=" + String(pH);

  client.print("POST /update HTTP/1.1\r\n");
  client.print("Host: api.thingspeak.com\r\n");
  client.print("Connection: close\r\n");
  client.print("Content-Type: application/x-www-form-urlencoded\r\n");
  client.print("Content-Length: " + String(body.length()) + "\r\n\r\n");
  client.print(body);
  client.stop();

  Serial.printf("ThingSpeak sent — TDS:%d ppm, EC:%.2f, pH:%.2f\n",
                tds, ec, pH);
}


// ============================================================
//  readTDS() — see tds_only.ino or esp32_main_node.ino
//  for detailed explanation of the conversion formula.
// ============================================================
void readTDS() {
  float rawVoltage = analogRead(TDS_SENSOR_PIN) * device::aref / 4096.0;
  float offset = 0.14;
  sensor::ec = (rawVoltage * sensor::ecCalibration) - offset;
  if (sensor::ec < 0) sensor::ec = 0;

  sensor::tds = (133.42 * pow(sensor::ec, 3)
                - 255.86 * sensor::ec * sensor::ec
                + 857.39 * sensor::ec) * 0.5;

  Serial.printf("TDS: %d ppm | EC: %.2f mS/cm\n", sensor::tds, sensor::ec);

  lcd.setCursor(0, 0);
  lcd.print("TDS:"); lcd.print(sensor::tds); lcd.print("ppm  ");

  Blynk.virtualWrite(V0, sensor::tds);
  Blynk.virtualWrite(V1, sensor::ec);
}


// ============================================================
//  readPH() — see esp32_main_node.ino for full explanation.
//  Samples 10 readings, drops outliers, averages middle 6.
// ============================================================
void readPH() {
  int samples[10];
  for (int i = 0; i < 10; i++) {
    samples[i] = analogRead(PH_SENSOR_PIN);
    delay(10);
  }

  // Sort ascending
  for (int i = 0; i < 9; i++)
    for (int j = i + 1; j < 10; j++)
      if (samples[i] > samples[j]) {
        int tmp = samples[i]; samples[i] = samples[j]; samples[j] = tmp;
      }

  // Average middle 6
  long sum = 0;
  for (int i = 2; i < 8; i++) sum += samples[i];

  float voltage = (float)sum / 6 * 3.3 / 4096.0;
  phValue = (3.5 * voltage) - 1.3;
  if (phValue < 0)  phValue = 0;
  if (phValue > 14) phValue = 14;

  Serial.printf("pH: %.2f\n", phValue);

  lcd.setCursor(0, 1);
  lcd.print("pH: "); lcd.print(phValue, 2); lcd.print("       ");

  Blynk.virtualWrite(V2, phValue);
}


void setup() {
  Serial.begin(115200);
  Blynk.begin(CONF_BLYNK_AUTH_TOKEN, CONF_WIFI_SSID, CONF_WIFI_PASS);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("TDS + pH Node");
  delay(2000);
  lcd.clear();
}

void loop() {
  Blynk.run();
  readTDS();
  readPH();
  sendToThingSpeak(sensor::tds, sensor::ec, phValue);
  delay(2000);
}
