/*
 * ============================================================
 *  IoT-Based Pearl Farm Health Regulator
 *  File   : esp32_actuator_node.ino
 *  Board  : ESP32 DevKit V1
 * ============================================================
 *
 *  Description:
 *    Actuator and display node of the pearl farm system.
 *    Receives sensor data wirelessly from two sensor nodes
 *    via ESP-NOW protocol, shows live readings on a 20x4 LCD,
 *    uploads data to Blynk cloud every 5 minutes, and
 *    automatically triggers solenoid valves (water exchange)
 *    when water quality goes outside safe limits.
 *
 *    Also controls:
 *      - Oxygen pump  : toggles every 5 minutes via relay
 *      - Wave generator: toggles every 30 seconds via relay
 *
 *  ESP-NOW Node Layout:
 *    Node 1 --> sends temperature + pH
 *    Node 2 --> sends TDS + turbidity
 *    This node (main) --> receives from both, controls relays
 *
 *  Relay GPIO:
 *    RELAY_1 (GPIO 12) --> Drain solenoid valve (water outlet)
 *    RELAY_2 (GPIO 14) --> Fill solenoid valve  (water inlet)
 *
 *  Water Exchange Logic:
 *    Triggered when: pH < 6.0 OR pH > 8.0 OR TDS > 550 ppm
 *    Sequence: Drain for 2.5 min -> Fill for 2.5 min -> Done
 *
 *  Blynk Virtual Pins:
 *    V1 = Temperature | V2 = pH | V3 = TDS | V4 = Turbidity
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
 *    2. Fill your credentials and MAC addresses in config.h
 *    3. config.h is in .gitignore — never gets committed
 * ============================================================
 */

// ── Credentials ───────────────────────────────────────────────
#include "config.h"

// ── Blynk setup (defines must come before Blynk headers) ──────
#define BLYNK_TEMPLATE_ID   CONF_BLYNK_TEMPLATE_ID
#define BLYNK_TEMPLATE_NAME CONF_BLYNK_TEMPLATE_NAME
#define BLYNK_AUTH_TOKEN    CONF_BLYNK_AUTH_TOKEN

// ── Libraries ─────────────────────────────────────────────────
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>

// ── LCD (20 columns x 4 rows, I2C address 0x27) ───────────────
LiquidCrystal_I2C lcd(0x27, 20, 4);

// ── Relay GPIO Pins ───────────────────────────────────────────
#define RELAY_1  12   // Drain valve — opens to empty tank
#define RELAY_2  14   // Fill valve  — opens to refill tank

// ── ESP-NOW Data Structures ───────────────────────────────────
// This struct must exactly match the one in the sender nodes.
// __attribute__((packed)) prevents compiler padding between fields.
typedef struct __attribute__((packed)) struct_message {
  float temperature;   // Water temperature (°C) from Node 1
  float ph;            // pH value from Node 1
  float tds;           // TDS (ppm) from Node 2
  float turbidity;     // Turbidity (NTU) from Node 2
  int   sensorID;      // 1 = Node1 data, 2 = Node2 data
} struct_message;

// Request packet sent TO sensor nodes to trigger a reading
typedef struct __attribute__((packed)) request_message {
  uint8_t requestCode;  // Code 1 = "please send your sensor data"
} request_message;

struct_message incomingData;
request_message reqMsg = {1};  // Pre-built request packet

// ── Live Sensor Values ────────────────────────────────────────
float temperature = 0.0;
float ph          = 0.0;
float tds         = 0.0;
float turbidity   = 0.0;

// ── Blynk Virtual Pin Mapping ─────────────────────────────────
#define VPIN_TEMP      V1
#define VPIN_PH        V2
#define VPIN_TDS       V3
#define VPIN_TURBIDITY V4

// ── Relay Timing Variables ────────────────────────────────────
unsigned long relay1OnTime  = 0;
unsigned long relay2OnTime  = 0;
bool          relay1Active  = false;   // Drain valve is running
bool          relay2Active  = false;   // Fill valve is running

// ── ESP-NOW Timing ────────────────────────────────────────────
unsigned long lastRequestTime    = 0;
const unsigned long REQUEST_INTERVAL = 2000;   // Poll sensors every 2 sec

// ── MAC Addresses of Sensor Nodes ────────────────────────────
// Replace with actual MAC addresses from your ESP32 boards.
// Find MAC: upload a sketch with Serial.println(WiFi.macAddress())
uint8_t node1Address[] = CONF_NODE1_MAC;
uint8_t node2Address[] = CONF_NODE2_MAC;

// ── Blynk Timer ───────────────────────────────────────────────
BlynkTimer timer;


// ============================================================
//  uploadToBlynk()
//
//  Pushes all sensor readings to Blynk cloud.
//  Called by BlynkTimer every 5 minutes to reduce bandwidth.
//  Farmers can view live graphs and history on the Blynk app.
// ============================================================
void uploadToBlynk() {
  Blynk.virtualWrite(VPIN_TEMP,      temperature);
  Blynk.virtualWrite(VPIN_PH,        ph);
  Blynk.virtualWrite(VPIN_TDS,       tds);
  Blynk.virtualWrite(VPIN_TURBIDITY, turbidity);
  Serial.println("Blynk: Data uploaded.");
}


// ============================================================
//  onReceive()
//
//  ESP-NOW callback — called automatically whenever this node
//  receives a packet from Node 1 or Node 2.
//
//  How ESP-NOW works:
//    - No router required. Direct peer-to-peer Wi-Fi between
//      ESP32 boards on the same channel.
//    - Packets are identified by sender MAC address or by the
//      sensorID field inside the data struct.
//    - Latency is very low (~1 ms) compared to MQTT over Wi-Fi.
//
//  After updating readings, this function:
//    1. Refreshes the 20x4 LCD display
//    2. Checks if water quality is out of safe range
//    3. Triggers drain valve (RELAY_1) if needed
// ============================================================
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  // Reject packets that don't match our expected struct size
  if (len != sizeof(incomingData)) return;

  memcpy(&incomingData, data, sizeof(incomingData));

  // Route data to correct variable based on which node sent it
  if (incomingData.sensorID == 1) {
    temperature = incomingData.temperature;
    ph          = incomingData.ph;
  } else if (incomingData.sensorID == 2) {
    tds         = incomingData.tds;
    turbidity   = incomingData.turbidity;
  }

  // Update 20x4 LCD with all four readings
  lcd.setCursor(0, 0); lcd.printf("Temp: %.1f C      ", temperature);
  lcd.setCursor(0, 1); lcd.printf("pH:   %.2f        ", ph);
  lcd.setCursor(0, 2); lcd.printf("TDS:  %.0f ppm    ", tds);
  lcd.setCursor(0, 3); lcd.printf("Turb: %.1f NTU    ", turbidity);

  // ── Water Exchange Automation ──────────────────────────────
  // If water quality is outside safe range AND no exchange
  // is already happening, start the drain cycle.
  // Safe range: 6.0 <= pH <= 8.0 AND TDS <= 550 ppm
  if ((ph < 6.0 || ph > 8.0 || tds > 550) && !relay1Active && !relay2Active) {
    Serial.println("ALERT: Water quality out of range. Starting drain...");
    digitalWrite(RELAY_1, HIGH);  // Open drain valve
    relay1Active  = true;
    relay1OnTime  = millis();
  }
}


// ============================================================
//  setup()
// ============================================================
void setup() {
  Serial.begin(115200);
  Serial.println("=== Pearl Farm — Actuator Node ===");

  // Initialise relay pins — LOW = valve closed (safe default)
  pinMode(RELAY_1, OUTPUT); digitalWrite(RELAY_1, LOW);
  pinMode(RELAY_2, OUTPUT); digitalWrite(RELAY_2, LOW);

  // Initialise LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("Connecting WiFi...");

  // Connect Wi-Fi in Station mode (needed for both Blynk + ESP-NOW)
  WiFi.mode(WIFI_STA);
  Blynk.begin(CONF_BLYNK_AUTH_TOKEN, CONF_WIFI_SSID, CONF_WIFI_PASS);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("WiFi Connected");
  Serial.println("WiFi + Blynk connected.");

  // ── ESP-NOW Setup ──────────────────────────────────────────
  // ESP-NOW and Wi-Fi must use the SAME channel.
  // Read the current router channel and lock ESP-NOW to it.
  uint8_t primaryChan;
  wifi_second_chan_t secondChan;
  esp_wifi_get_channel(&primaryChan, &secondChan);
  Serial.printf("Router channel: %d\n", primaryChan);
  esp_wifi_set_channel(primaryChan, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: ESP-NOW init failed.");
    lcd.setCursor(0, 1); lcd.print("ESP-NOW Failed!");
    return;
  }

  // Register callback for incoming ESP-NOW packets
  esp_now_register_recv_cb(onReceive);

  // Add Node 1 and Node 2 as ESP-NOW peers
  esp_now_peer_info_t peerInfo = {};
  peerInfo.channel = primaryChan;
  peerInfo.encrypt = false;

  memcpy(peerInfo.peer_addr, node1Address, 6);
  esp_now_add_peer(&peerInfo);

  memcpy(peerInfo.peer_addr, node2Address, 6);
  esp_now_add_peer(&peerInfo);

  Serial.println("ESP-NOW peers registered.");

  // Upload to Blynk every 5 minutes (300,000 ms)
  timer.setInterval(300000L, uploadToBlynk);

  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("System Ready");
  delay(1000);
  lcd.clear();
}


// ============================================================
//  loop()
// ============================================================
void loop() {
  Blynk.run();   // Maintain Blynk connection
  timer.run();   // Run scheduled Blynk upload

  // ── Poll Sensor Nodes via ESP-NOW every 2 seconds ─────────
  if (millis() - lastRequestTime >= REQUEST_INTERVAL) {
    lastRequestTime = millis();
    // Send request packets to both sensor nodes
    esp_now_send(node1Address, (uint8_t*)&reqMsg, sizeof(reqMsg));
    esp_now_send(node2Address, (uint8_t*)&reqMsg, sizeof(reqMsg));
  }

  // ── Relay 1: Drain valve timeout (2.5 minutes) ────────────
  // After draining for 150 seconds, stop drain and start fill.
  if (relay1Active && millis() - relay1OnTime >= 150000) {
    digitalWrite(RELAY_1, LOW);    // Close drain valve
    relay1Active = false;
    Serial.println("Drain complete. Starting fill...");

    digitalWrite(RELAY_2, HIGH);   // Open fill valve
    relay2Active = true;
    relay2OnTime = millis();
  }

  // ── Relay 2: Fill valve timeout (2.5 minutes) ─────────────
  // After filling for 150 seconds, close fill valve. Done.
  if (relay2Active && millis() - relay2OnTime >= 150000) {
    digitalWrite(RELAY_2, LOW);    // Close fill valve
    relay2Active = false;
    Serial.println("Fill complete. Water exchange done.");
  }
}
