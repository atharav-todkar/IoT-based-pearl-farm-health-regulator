/*
 * ============================================================
 *  IoT-Based Pearl Farm Health Regulator
 *  File   : esp32_main_node.ino
 *  Board  : ESP32 DevKit V1
 * ============================================================
 *
 *  Description:
 *    Main sensor node of the pearl farm monitoring system.
 *    Reads pH, TDS, and temperature from three sensors,
 *    displays live values on a 16x2 LCD, and uploads data
 *    to both Blynk and ThingSpeak cloud platforms.
 *
 *  Sensors & GPIO:
 *    - DFRobot pH Sensor V2   --> GPIO 33 (ADC1 Ch5, analog)
 *    - Keyestudio TDS Sensor  --> GPIO 34 (ADC1 Ch6, analog)
 *    - DS18B20 Temp Sensor    --> GPIO 4  (One-Wire, digital)
 *
 *  NOTE: ADC1 pins (GPIO 32-39) are used intentionally.
 *  ADC2 pins conflict with Wi-Fi on ESP32 and give wrong readings.
 *
 *  Cloud:
 *    - Blynk  : V0=TDS, V1=EC, V2=pH, V3=Temperature
 *    - ThingSpeak : field1=TDS, field2=EC, field3=pH, field4=Temp
 *
 *  Thresholds for oyster health:
 *    pH  : 6.0 – 8.5
 *    TDS : 0   – 550 ppm
 *    Temp: 20  – 32 °C
 *
 *  Project : TIH Chanakya Fellowship, IIT Bombay
 *  Grant   : TIH-IOT/2024-11/HRD/CHANAKYA/SL/CFP/041
 *  Patent  : Design Patent No. 454051-001 (02/04/2025)
 *  Team    : Swaroop Thikane, Savita Totad, Atharav Todkar,
 *            Vaishnavi Pawar, Vedant Joshi
 *  Guide   : Dr. Jayashree P. Kharat, DKTE's TEI
 * ============================================================
 *
 *  SETUP BEFORE UPLOADING:
 *    1. Copy config.h.example -> config.h
 *    2. Fill your credentials in config.h
 *    3. config.h is in .gitignore — never gets committed
 * ============================================================
 */

// ── Credentials (loaded from config.h — never hardcode here) ──
#include "config.h"

// ── Blynk setup (defines must come before Blynk headers) ──────
#define BLYNK_TEMPLATE_ID   CONF_BLYNK_TEMPLATE_ID
#define BLYNK_TEMPLATE_NAME CONF_BLYNK_TEMPLATE_NAME
#define BLYNK_AUTH_TOKEN    CONF_BLYNK_AUTH_TOKEN
#define BLYNK_PRINT Serial   // Print Blynk debug info to Serial

// ── Libraries ─────────────────────────────────────────────────
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ── Pin Definitions ───────────────────────────────────────────
#define PH_SENSOR_PIN   33   // DFRobot pH sensor analog output
#define TDS_SENSOR_PIN  34   // Keyestudio TDS sensor analog output
#define ONE_WIRE_BUS     4   // DS18B20 data pin (One-Wire protocol)

// ── DS18B20 Temperature Sensor ────────────────────────────────
// One-Wire allows multiple sensors on a single data pin.
// DS18B20 outputs a 12-bit digital value — no ADC needed.
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Unique 64-bit address of your DS18B20 sensor.
// To find yours: run a OneWire address scanner sketch first,
// then paste the result here.
DeviceAddress tempSensor = {0x28, 0x44, 0x9B, 0x4D, 0xC0, 0x23, 0x09, 0x56};

// ── LCD (16 columns x 2 rows, I2C address 0x27) ───────────────
// I2C uses GPIO 21 (SDA) and GPIO 22 (SCL) on ESP32 by default
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ── ThingSpeak ────────────────────────────────────────────────
const char* thingSpeakServer = "api.thingspeak.com";
WiFiClient client;

// ── TDS Sensor Variables ──────────────────────────────────────
namespace tds {
  const float  vref          = 3.3;   // ESP32 runs on 3.3V (NOT 5V like Arduino)
  float        ecCalibration = 1.0;   // Calibration factor — adjust after field testing
  float        dryOffset     = 0.14;  // Voltage reading with probe in air (no water)
  float        ec            = 0.0;   // Electrical conductivity in mS/cm
  unsigned int value         = 0;     // TDS in ppm
}

// ── Global Sensor Readings ────────────────────────────────────
float phValue   = 0.0;
float tempValue = 0.0;


// ============================================================
//  readTDS()
//
//  Reads TDS (Total Dissolved Solids) from Keyestudio sensor.
//
//  How it works:
//    The sensor measures water's electrical conductivity (EC).
//    Higher ion concentration = higher conductivity = higher TDS.
//    The raw ADC voltage is converted to TDS (ppm) using a
//    cubic polynomial equation from the sensor datasheet:
//
//    TDS = (133.42*V^3 - 255.86*V^2 + 857.39*V) * 0.5
//
//    A dry-air offset (0.14V) is subtracted first to remove
//    the sensor's baseline voltage when not in water.
// ============================================================
void readTDS() {
  // Read ADC and convert to voltage (12-bit ADC on ESP32 = 4096 steps)
  float rawVoltage = analogRead(TDS_SENSOR_PIN) * tds::vref / 4096.0;

  // Subtract baseline offset measured with probe in dry air
  tds::ec = (rawVoltage * tds::ecCalibration) - tds::dryOffset;
  if (tds::ec < 0) tds::ec = 0;  // Clamp negative values

  // Cubic polynomial: converts EC voltage -> TDS in ppm
  tds::value = (133.42 * pow(tds::ec, 3)
               - 255.86 * tds::ec * tds::ec
               + 857.39 * tds::ec) * 0.5;

  Serial.print("TDS: "); Serial.print(tds::value); Serial.print(" ppm | ");
  Serial.print("EC: ");  Serial.print(tds::ec, 2); Serial.println(" mS/cm");

  // Show on LCD row 0
  lcd.setCursor(0, 0);
  lcd.print("TDS:"); lcd.print(tds::value); lcd.print("ppm  ");

  // Send to Blynk
  Blynk.virtualWrite(V0, tds::value);
  Blynk.virtualWrite(V1, tds::ec);
}


// ============================================================
//  readPH()
//
//  Reads pH from DFRobot Gravity pH Sensor V2.
//
//  How it works:
//    The BNC glass electrode generates a voltage based on the
//    hydrogen ion concentration (Nernst equation):
//      E = E0 - (RT/nF) * ln[H+]
//    At 25°C, this is ~59 mV per pH unit.
//
//    The sensor linearises this to 0-3V output.
//    We take 10 samples, sort them, and average the middle 6
//    to remove noise spikes (outlier rejection filter).
//
//  Calibration:
//    Use buffer solutions (pH 4.0 and pH 7.0) with the
//    'enterph' and 'calph' serial commands. Calibration data
//    is stored in the sensor's EEPROM automatically.
// ============================================================
void readPH() {
  // Collect 10 samples for noise filtering
  int samples[10];
  for (int i = 0; i < 10; i++) {
    samples[i] = analogRead(PH_SENSOR_PIN);
    delay(10);
  }

  // Bubble sort samples from low to high
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (samples[i] > samples[j]) {
        int tmp = samples[i];
        samples[i] = samples[j];
        samples[j] = tmp;
      }
    }
  }

  // Average the middle 6 samples (discard lowest 2 and highest 2)
  long sum = 0;
  for (int i = 2; i < 8; i++) sum += samples[i];

  // Convert averaged ADC count to voltage, then to pH
  float voltage = (float)sum / 6 * 3.3 / 4096.0;
  phValue = (3.5 * voltage) - 1.3;  // Linear mapping from calibration

  // Clamp to valid pH range
  if (phValue < 0)  phValue = 0;
  if (phValue > 14) phValue = 14;

  Serial.print("pH: "); Serial.println(phValue, 2);

  // Show on LCD row 1
  lcd.setCursor(0, 1);
  lcd.print("pH:"); lcd.print(phValue, 1); lcd.print("       ");

  // Send to Blynk
  Blynk.virtualWrite(V2, phValue);
}


// ============================================================
//  readTemperature()
//
//  Reads temperature from DS18B20 over One-Wire protocol.
//
//  How it works:
//    DS18B20 uses a single-wire bus. The master (ESP32) sends
//    a conversion command; the sensor responds with a 12-bit
//    digital temperature value (resolution = 0.0625°C).
//    Accuracy is ±0.5°C across -10°C to +85°C.
//    Multiple DS18B20 sensors can share the same data pin,
//    each identified by its unique 64-bit ROM address.
// ============================================================
void readTemperature() {
  sensors.requestTemperatures();  // Send conversion command
  tempValue = sensors.getTempC(tempSensor);

  // -127°C means sensor is disconnected or wiring issue
  if (tempValue == DEVICE_DISCONNECTED_C) {
    Serial.println("Temp ERROR: DS18B20 not responding. Check wiring.");
    return;
  }

  Serial.print("Temp: "); Serial.print(tempValue); Serial.println(" C");

  // Send to Blynk
  Blynk.virtualWrite(V3, tempValue);
}


// ============================================================
//  sendToThingSpeak()
//
//  Uploads all sensor values to ThingSpeak via HTTP POST.
//  ThingSpeak stores time-series data and generates graphs
//  for long-term water quality trend analysis.
//
//  Channel field mapping:
//    field1 = TDS (ppm)
//    field2 = EC  (mS/cm)
//    field3 = pH
//    field4 = Temperature (°C)
// ============================================================
void sendToThingSpeak() {
  if (!client.connect(thingSpeakServer, 80)) {
    Serial.println("ThingSpeak: Connection failed.");
    return;
  }

  // Build POST body
  String body = "api_key=" + String(CONF_THINGSPEAK_API_KEY)
              + "&field1=" + String(tds::value)
              + "&field2=" + String(tds::ec)
              + "&field3=" + String(phValue)
              + "&field4=" + String(tempValue);

  // Send HTTP POST request
  client.print("POST /update HTTP/1.1\r\n");
  client.print("Host: api.thingspeak.com\r\n");
  client.print("Connection: close\r\n");
  client.print("Content-Type: application/x-www-form-urlencoded\r\n");
  client.print("Content-Length: " + String(body.length()) + "\r\n\r\n");
  client.print(body);
  client.stop();

  Serial.println("ThingSpeak: Data sent successfully.");
}


// ============================================================
//  setup()
// ============================================================
void setup() {
  Serial.begin(115200);
  Serial.println("=== Pearl Farm Health Regulator — Main Node ===");

  // Initialise LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Pearl Farm IoT");
  lcd.setCursor(0, 1); lcd.print("Starting...");
  delay(1500);
  lcd.clear();

  // Initialise DS18B20 temperature sensor
  sensors.begin();
  if (!sensors.validAddress(tempSensor)) {
    Serial.println("WARNING: DS18B20 address not found. Check sensor wiring.");
  } else {
    sensors.setResolution(tempSensor, 12);  // 12-bit = 0.0625°C resolution
    Serial.println("DS18B20 OK.");
  }

  // Connect to Blynk (also handles Wi-Fi connection)
  Blynk.begin(CONF_BLYNK_AUTH_TOKEN, CONF_WIFI_SSID, CONF_WIFI_PASS);
  Serial.println("Blynk connected.");

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("System Ready");
  delay(1000);
  lcd.clear();
}


// ============================================================
//  loop() — runs every 2 seconds
// ============================================================
void loop() {
  Blynk.run();       // Keep Blynk connection alive

  readTDS();
  readPH();
  readTemperature();
  sendToThingSpeak();

  delay(2000);       // 2-second sampling interval
}
