/*
 * ============================================================
 *  IoT-Based Pearl Farm Health Regulator
 *  File   : ph_calibration_test.ino
 *  Folder : reference_sketches/
 *  Board  : Arduino Uno
 * ============================================================
 *
 *  NOTE: This is a REFERENCE / CALIBRATION sketch.
 *  This runs on Arduino Uno (NOT ESP32).
 *  It was used in the early stage to verify pH sensor
 *  readings against known buffer solutions (pH 4.0 and 7.0).
 *  No Wi-Fi or cloud — purely for bench calibration.
 *
 *  The final deployed firmware is: esp32_main_node.ino
 *
 *  Description:
 *    Reads the DFRobot pH sensor on Arduino Analog Pin A0.
 *    Takes 10 samples, sorts them, and averages the middle 6
 *    (outlier rejection) for a stable reading.
 *    Prints pH value to Serial Monitor at 9600 baud.
 *
 *  Calibration Procedure:
 *    1. Upload this sketch to Arduino Uno
 *    2. Open Serial Monitor at 9600 baud
 *    3. Place probe in pH 7.0 buffer solution
 *       Wait for readings to stabilise
 *    4. Note the voltage — adjust the formula constants if needed
 *    5. Repeat with pH 4.0 buffer solution
 *    For the DFRobot V2 sensor, use the 'enterph' / 'calph'
 *    serial commands instead — they store calibration in EEPROM.
 *
 *  Sensor:
 *    DFRobot Gravity pH Sensor V2
 *    Range    : 0 – 14 pH
 *    Accuracy : ±0.1 pH
 *    Output   : 0 – 5V analog (for Arduino 5V)
 *               0 – 3V analog (for ESP32 3.3V)
 *    Arduino pin: A0 (5V ADC, 10-bit = 1024 steps)
 *
 *  Project : TIH Chanakya Fellowship, IIT Bombay
 *  Team    : Swaroop Thikane, Savita Totad, Atharav Todkar,
 *            Vaishnavi Pawar, Vedant Joshi
 *  Guide   : Dr. Jayashree P. Kharat, DKTE's TEI
 * ============================================================
 */

// ── Pin Definition ────────────────────────────────────────────
#define PH_SENSOR_PIN  A0   // DFRobot pH sensor analog output -> A0

// ── Variables ─────────────────────────────────────────────────
int           sampleBuf[10];   // Buffer to hold 10 raw ADC readings
int           tempVal;         // Temporary variable for bubble sort
unsigned long avgValue;        // Sum of the middle 6 samples


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);   // Built-in LED used as "reading taken" indicator
  Serial.begin(9600);
  Serial.println("pH Calibration Test — Ready");
  Serial.println("Place probe in buffer solution and observe readings.");
}


void loop() {
  // ── Step 1: Collect 10 samples ────────────────────────────
  // Multiple samples smooth out short-term electrical noise
  // on the analog line (capacitive coupling, EMI, etc.)
  for (int i = 0; i < 10; i++) {
    sampleBuf[i] = analogRead(PH_SENSOR_PIN);
    delay(10);   // 10ms between samples = 100ms total collection window
  }

  // ── Step 2: Sort samples ascending (bubble sort) ──────────
  // Sorting allows us to easily discard the extreme outliers
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (sampleBuf[i] > sampleBuf[j]) {
        tempVal      = sampleBuf[i];
        sampleBuf[i] = sampleBuf[j];
        sampleBuf[j] = tempVal;
      }
    }
  }

  // ── Step 3: Average the middle 6 (reject lowest 2, highest 2) ──
  // This is a simple outlier rejection filter. The 2 lowest
  // readings (likely noise spikes downward) and 2 highest
  // (noise spikes upward) are discarded before averaging.
  avgValue = 0;
  for (int i = 2; i < 8; i++) avgValue += sampleBuf[i];

  // ── Step 4: Convert ADC count -> voltage -> pH ────────────
  // Arduino Uno: 5V reference, 10-bit ADC (1024 steps)
  // Voltage = avgValue/6 samples * (5V / 1024)
  // pH formula from DFRobot datasheet:
  //   pH = 3.5 * voltage  (calibrated linear approximation)
  float voltage = (float)avgValue * 5.0 / 1024 / 6;
  float phValue = 3.5 * voltage;

  // Print to Serial Monitor
  Serial.print("pH: ");
  Serial.println(phValue, 2);   // 2 decimal places

  // Blink LED to confirm each reading cycle
  digitalWrite(LED_BUILTIN, HIGH);
  delay(800);
  digitalWrite(LED_BUILTIN, LOW);
}
