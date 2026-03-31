## ⚙️ Working Principle

The system continuously monitors water quality parameters such as pH, TDS, and temperature using dedicated sensors connected to ESP32 microcontrollers.

### 🔹 Step-by-Step Operation

1. **Data Acquisition**

   * pH and temperature are measured using Node 1
   * TDS is measured using Node 2

2. **Data Transmission**

   * Sensor data is transmitted to the main ESP32 node using ESP-NOW protocol every few seconds

3. **Data Processing**

   * The main node compares received values with predefined threshold limits

4. **Decision Making**

   * If pH < 5.9 or pH > 8 → system triggers correction
   * If TDS > 550 ppm → water replacement is initiated

5. **Automated Control**

   * Solenoid valve opens to drain water
   * Fresh water is refilled automatically
   * Oxygen pump operates every 5 minutes
   * Wave generator operates every 30 seconds

6. **Display & Monitoring**

   * Real-time data is displayed on a 20x4 LCD
   * Data is uploaded to cloud (Blynk) every hour

7. **Remote Access**

   * Users can monitor system remotely via mobile/web dashboard

---

### 🔹 Key Highlights

* Fully automated water regulation system
* Real-time monitoring and control
* Low latency communication using ESP-NOW
* Minimal human intervention required
