# 🦪 IoT-Based Pearl Farm Health Regulator

<div align="center">

![Project Banner](docs/Block_Diagram.jpg)

[![Patent Granted](https://img.shields.io/badge/Design%20Patent-454051--001%20Granted-gold?style=for-the-badge&logo=gov.uk)](docs/patent.pdf)
[![TIH Fellowship](https://img.shields.io/badge/TIH%20Chanakya%20Fellowship-IIT%20Bombay%20%E2%82%B96L-blue?style=for-the-badge)](docs/IIT%20BOMBAY-Sanction%20letter.pdf)
[![IEEE Paper](https://img.shields.io/badge/IEEE%20Xplore-ICFT%202025-red?style=for-the-badge&logo=ieee)](docs/ieee%20paper.docx)
[![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)](LICENSE)

**An intelligent IoT system for real-time water quality monitoring and automated regulation in freshwater pearl aquaculture.**

*Awarded the TIH Chanakya Fellowship (₹6,00,000) by IIT Bombay | Design Patent Granted | Field-Validated in Real Pearl Farm*

[📄 IEEE Paper](#publication) · [🔬 System Architecture](#system-architecture) · [📊 Results](#results--validation) · [🚀 Getting Started](#getting-started)

</div>

---

## 🧠 Project Overview

Pearl farming is a precision-sensitive aquaculture process where water quality parameters — pH, Total Dissolved Solids (TDS), and temperature — directly determine oyster survival, nacre formation, and pearl quality. Manual monitoring is slow, inaccurate, and scales poorly.

This project delivers a **deployment-ready IoT solution** that automates water quality monitoring and regulation for freshwater pearl farms. The system integrates embedded sensing, edge computing, cloud telemetry, and electromechanical actuation into a compact, farmer-operable device — validated in a live 10,000 L pearl farming tank with 3,000 oyster shells.

> **Key Insight:** The system reduced manual monitoring effort by ~70%, maintained water parameters within ±5% of target, and autonomously triggered water exchange cycles without human intervention.

---

## ✨ Key Achievements

| Achievement | Details |
|---|---|
| 🏛️ **Design Patent Granted** | Patent No. 454051-001 · Filed & Granted 02/04/2025 |
| 🏆 **TIH Chanakya Fellowship** | ₹6,00,000 grant from IIT Bombay's Technology Innovation Hub |
| 📡 **IEEE Publication** | Accepted at ICFT 2025 · To be indexed in IEEE Xplore Digital Library |
| 🌾 **Field Deployment** | Validated in real pearl farm · Kolhapur, Maharashtra · Dec 2024 – Oct 2025 |
| ⚙️ **Automation Achieved** | Automated water replacement, algae feeding scheduling, oxygen regulation |

---

## 🔬 System Architecture

The system is built on a **three-layer IoT architecture** designed for reliability and modularity:

```
┌─────────────────────────────────────────────────────────────────┐
│                        SENSING LAYER                            │
│   DFRobot pH Sensor  │  Keyestudio TDS Sensor  │  DS18B20 Temp │
│         (0-14 pH)    │      (0-1000 ppm)        │  (-55 to 125°C)│
└──────────────┬──────────────────────────────────┬───────────────┘
               │  Analog/Digital Signal            │ One-Wire
               ▼                                  ▼
┌─────────────────────────────────────────────────────────────────┐
│                    PROCESSING LAYER                             │
│        Arduino Uno (Sensor Data Acquisition)                    │
│        Serial UART (9600 bps) ──► ESP32 (Main Node)            │
│        • ADC Noise Isolation    • Wi-Fi Communication           │
│        • Averaging Filter       • MQTT / Google Sheets          │
│        • Temperature Compensation │ LCD Display (20×4 I2C)     │
└──────────────────────────────────┬──────────────────────────────┘
                                   │ Wi-Fi / MQTT
                                   ▼
┌─────────────────────────────────────────────────────────────────┐
│                  CLOUD & CONTROL LAYER                          │
│   Web Dashboard ◄── Google Sheets ◄── Blynk / ThingSpeak       │
│                                                                 │
│   Automation Logic:                                             │
│   pH > 8.5 or TDS > 550 ppm  ──► Solenoid Valves (12V DC)      │
│   Oxygen Pump (every 5 min)  │  Wave Generator (every 30 sec)  │
└─────────────────────────────────────────────────────────────────┘
```

### Hardware Architecture Decision — Why Arduino + ESP32?

> **Engineering Challenge Solved:** Initial design used 3 separate ESP32 boards (one per sensor). During testing, simultaneous Wi-Fi + ADC usage caused ADC2 conflicts, MQTT packet collisions, and system instability.
>
> **Solution:** Introduced Arduino Uno as a dedicated sensor acquisition node. Arduino handles all analog ADC operations (ADC1 only, avoiding Wi-Fi conflicts), serializes the data, and sends it to a single ESP32 over UART. This eliminated all instability and is the architecture now used in field deployment.

---

## 🛠️ Hardware Components

| Component | Specification | Role |
|---|---|---|
| **ESP32** | Dual-core, Wi-Fi + BT, 3.3V, multiple GPIOs | Main IoT node, cloud comm, LCD, relay control |
| **Arduino Uno** | ATmega328P, stable ADC | Sensor data acquisition, serial transmission |
| **DFRobot pH Sensor V2** | 0–14 pH, ±0.1, BNC probe, internal EEPROM | Water acidity monitoring |
| **Keyestudio TDS Sensor** | 0–1000 ppm, ±10% FS, 0–2.3V output | Dissolved solids measurement |
| **DS18B20** | -55°C to +125°C, ±0.5°C, One-Wire | Water temperature monitoring |
| **Solenoid Valves** | 12V DC, normally closed | Automated water inlet/outlet control |
| **20×4 I2C LCD** | I2C, 20 columns × 4 rows | On-field real-time display |
| **Relay Module** | 5V trigger, 12V/230V load switching | Controls pumps, valves, wave generator |
| **Oxygen Pump** | Submersible, relay-controlled | Aeration for oyster survival |
| **Wave Generator** | Timer-controlled, 30s interval | Water circulation simulation |

📋 [Full Component List with specifications and datasheets →](docs/component.pdf)

---

## 💻 Software Stack

- **Embedded Firmware:** Arduino IDE · Embedded C/C++
- **Communication Protocols:** UART Serial (9600 bps) · MQTT · ESP-NOW · One-Wire
- **Cloud Platform:** Google Sheets (auto-logging every 2 hrs) · Blynk / ThingSpeak
- **Signal Processing:** Moving Average Filter · Temperature Compensation for pH/TDS
- **Automation Logic:** Threshold-based if-else control with hysteresis
- **Future:** Machine Learning models for predictive water quality forecasting

---

## 🔁 Control Logic & Automation

```
Sensor Reading (every 10 sec)
        │
        ▼
 ┌─────────────────────────────┐
 │  Is pH < 5.9 or pH > 8.5?  │──YES──► Activate Drain Valve
 │  Is TDS > 550 ppm?         │         Wait for drain completion
 └─────────────────────────────┘         Activate Fill Valve
        │ NO                             Restore optimal conditions
        ▼
 Continue monitoring
 Upload to cloud every 2 hours
 Display on LCD in real time
```

**Hysteresis prevents valve chattering:**
- Outlet opens when TDS > 550 ppm; closes only when TDS < 500 ppm
- pH correction triggers at pH > 8.5; resets below pH 8.0

---

## 📊 Results & Validation

The prototype was deployed in a **10,000 L freshwater pearl farming tank** with **3,000 oyster shells** in Kolhapur, Maharashtra. Four distributed sensor nodes were installed for spatial coverage.

| Metric | Result |
|---|---|
| **Monitoring Accuracy** | ±5% error across all three parameters |
| **pH Control** | Maintained between 5.9 – 8.5 autonomously |
| **TDS Control** | Maintained below 550 ppm via automated exchange |
| **Manual Monitoring Reduction** | ~70% reduction in farmer intervention |
| **System Uptime** | Continuous operation Dec 2024 – Oct 2025 |
| **Data Logging** | Automatic every 2 hours into Google Sheets |
| **Remote Access** | Live dashboard accessible by farmer on phone |

📈 [Detailed results and data analysis →](docs/results.md)

---

## 🧪 Sensor Calibration Methodology

pH sensor calibration was performed using **two-point calibration** with standard buffer solutions:

```
Step 1: Place probe in pH 7.0 buffer solution
        Send command: "enterph" → calibration mode
        Send command: "calph"   → stores slope/offset in EEPROM

Step 2: Repeat with pH 4.0 buffer solution
        Calibration data persists across power cycles

Step 3: Verify — compare against reference instrument
        Acceptable error: ±0.1 pH
```

DS18B20 temperature readings were cross-validated against a mercury thermometer reference. TDS readings include temperature compensation factor to correct for ionic mobility variation.

---

## 📐 System Flowchart

![Flowchart](docs/Flowchart.jpg)

📋 [Full flowchart PDF →](docs/final_flowq_chart.pdf)

---

## 🚀 Getting Started

### Prerequisites

- Arduino IDE 2.x
- ESP32 Board Package for Arduino IDE
- Required Libraries:
  - `DFRobot_PH` (pH sensor)
  - `OneWire` + `DallasTemperature` (DS18B20)
  - `MQTT` or `PubSubClient` (cloud upload)
  - `LiquidCrystal_I2C` (LCD display)

### Installation

```bash
git clone https://github.com/atharav-todkar/IoT-based-pearl-farm-health-regulator.git
cd IoT-based-pearl-farm-health-regulator
```

1. Open `arduino/sensor_node/sensor_node.ino` in Arduino IDE — flash to **Arduino Uno**
2. Open `esp32/main_node/main_node.ino` in Arduino IDE (with ESP32 board package) — flash to **ESP32**
3. Update Wi-Fi credentials and MQTT/Blynk tokens in `esp32/config.h`
4. Calibrate pH sensor using serial monitor commands (`enterph`, `calph`)
5. Power on, verify LCD output, check cloud dashboard

📖 [Detailed setup guide →](docs/working_principle.md)

---

## 🗂️ Repository Structure

```
IoT-based-pearl-farm-health-regulator/
├── docs/
│   ├── Block_Diagram.jpg          # System architecture diagram
│   ├── Flowchart.jpg              # Control logic flowchart
│   ├── component.pdf              # Full BOM with datasheets
│   ├── final_flowq_chart.pdf      # Detailed flowchart
│   ├── patent.pdf                 # Granted design patent
│   ├── IIT BOMBAY-Sanction letter.pdf  # TIH fellowship proof
│   ├── ieee paper.docx            # ICFT 2025 IEEE paper
│   ├── system_architecture.md     # Architecture description
│   ├── working_principle.md       # Sensor & circuit working
│   ├── features.md                # Feature list
│   ├── objectives.md              # Project objectives
│   ├── results.md                 # Test results & data
│   ├── future_scope.md            # Planned enhancements
│   ├── comparison.md              # Comparison with existing systems
│   └── problem_statement.md       # Problem & motivation
├── arduino/
│   └── sensor_node/               # Arduino Uno firmware
├── esp32/
│   └── main_node/                 # ESP32 firmware
└── README.md
```

---

## 📄 Publication

> **"IoT-Based Pearl Farm Health Regulator"**
> Swaroop Bharat Thikane, Savita Sidram Totad, Atharav Ramchandra Todkar, Vaishnavi Raju Pawar, Vedant Vishwanath Joshi, Dr. Jayashree P. Kharat
> *International Conference on Future Technologies (ICFT 2025)*
> **To be published in IEEE Xplore Digital Library** · November 7–8, 2025

---

## 🔮 Future Scope

- [ ] **Dosing Pump Integration** — automated pH correction without water replacement
- [ ] **Dissolved Oxygen (DO) & Turbidity Sensors** — expanded parameter monitoring
- [ ] **Solar Power Integration** — energy-autonomous field deployment
- [ ] **Machine Learning** — predictive water quality forecasting using logged data
- [ ] **Mobile App** — push notifications and farmer alert system
- [ ] **Multi-Tank Scaling** — distributed monitoring across interconnected ponds
- [ ] **Chiller/Heater Integration** — precise thermal regulation under seasonal variation

📋 [Full future scope document →](docs/future_scope.md)

---

## 👥 Team

| Name | Role | Institution |
|---|---|---|
| **Atharav Ramchandra Todkar** | Hardware Design, System Integration | DKTE's Textile & Engineering Institute |
| **Swaroop Bharat Thikane** | IoT Communication, Cloud Integration | DKTE's Textile & Engineering Institute |
| **Savita Sidram Totad** | Sensor Calibration, Testing | DKTE's Textile & Engineering Institute |
| **Vaishnavi Raju Pawar** | Software Development, Dashboard | DKTE's Textile & Engineering Institute |
| **Vedant Vishwanath Joshi** | Field Deployment, Documentation | DKTE's Textile & Engineering Institute |

**Faculty Guide:** Dr. Mrs. Jayashree Prashant Kharat, Professor — DKTE's Textile & Engineering Institute, Ichalkaranji *(Shivaji University, Kolhapur)*

---

## 📜 Patent

**Design Patent No. 454051-001** — *"IoT-Based Pearl Farming Device"*
Granted: 02 April 2025 · Inventors: J.P. Kharat, S. Thikane, S. Totad, A. Todkar, V. Pawar, V. Joshi, DKTE Society
[View Patent Document →](docs/patent.pdf)

---

## 💰 Funding

This project was supported by the **Technology Innovation Hub (TIH) for IoT & IoE, IIT Bombay** under the **Chanakya Fellowship Programme**.

- **Grant Number:** TIH-IOT/2024-11/HRD/CHANAKYA/SL/CFP/041
- **Funding Amount:** ₹6,00,000
- **Duration:** December 2024 – October 2025

[View Sanction Letter →](docs/IIT%20BOMBAY-Sanction%20letter.pdf)

---

## 📬 Contact

For collaboration, replication, or questions:

**Atharav Ramchandra Todkar**
B.E. Electronics & Telecommunication Engineering
DKTE's Society's Textile and Engineering Institute, Ichalkaranji
📧 etcartodkar@dkte.ac.in
🔗 [GitHub](https://github.com/atharav-todkar)

---

<div align="center">

*Built with precision for real-world aquaculture · Validated in field · Funded by IIT Bombay*

⭐ If this project helped your research or inspired your work, please consider starring the repository.

</div>
