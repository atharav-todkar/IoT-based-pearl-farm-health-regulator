#  IoT Based Pearl Farm Health Regulator

## 📄 IEEE Publication

🔗 https://ieeexplore.ieee.org/document/11336776

 Published in IEEE Xplore Digital Library (ICFT 2025)
 Download Paper: docs/ieee-paper.docx
---

##  Project Overview

The **IoT Based Pearl Farm Health Regulator** is a smart aquaculture system designed to monitor and control critical water parameters in pearl farming.

The system uses multiple ESP32 microcontrollers and sensors to ensure optimal environmental conditions, improving pearl quality, reducing manual effort, and enabling sustainable farming practices.

---
##  Problem Statement

Traditional pearl farming methods lack real-time monitoring and automation, which leads to unstable water conditions and poor productivity.

Key challenges include:

* No continuous monitoring of water parameters like pH, TDS, and temperature
* Sudden environmental changes affecting oyster health
* High mortality rates and inconsistent pearl quality
* Dependence on manual labor and lack of data-driven decisions

These limitations reduce efficiency and make it difficult for farmers to maintain optimal conditions for pearl cultivation.

---

##  Objectives

The main objectives of this project are:

* To design an IoT-based system for real-time monitoring of water quality parameters
* To maintain optimal conditions for pearl oyster growth
* To automate water regulation using sensor-based control
* To reduce manual effort and improve system efficiency
* To provide remote monitoring using a cloud-based dashboard
* To promote sustainable and smart aquaculture practices



---

##  System Architecture

The system is based on a distributed IoT architecture using multiple ESP32 nodes:

* Node 1: pH and Temperature monitoring
* Node 2: TDS monitoring
* Main Node: Data processing and control
* Control Node: Oxygen pump and wave generator

Communication is done using ESP-NOW protocol, and data is sent to the cloud (Blynk) for remote monitoring.

---

##  Hardware Components

* ESP32 Microcontroller
* pH Sensor (DFRobot)
* TDS Sensor (Keyestudio)
* DS18B20 Temperature Sensor
* 20x4 LCD Display (I2C)
* Solenoid Valves
* Relay Modules
* Oxygen Pump
* Wave Generator
* Submersible Pump

 Total Project Cost: ₹11,281

---

## ⚙️ Working Principle

* Sensors continuously monitor water parameters
* Data is sent to the main ESP32 node
* System checks threshold conditions:

  * pH < 5.9 or pH > 8 → Water correction
  * TDS > 550 ppm → Water drain and refill
* Solenoid valves automate water exchange
* Oxygen pump runs every 5 minutes
* Wave generator runs every 30 seconds
* Data is displayed on LCD and sent to cloud dashboard

---

##  Results & Analysis

* Stable pH range: 7.35 – 7.52
* Manual system pH range: 7.11 – 8.33
* Reduced manual effort by ~60%
* Faster response to environmental changes
* Improved water clarity and stability

---

##  Comparison Study

| Parameter     | IoT System | Manual System |
| ------------- | ---------- | ------------- |
| pH Stability  | Stable     | Fluctuating   |
| Maintenance   | Low        | High          |
| Water Quality | Clear      | Algae Growth  |
| Efficiency    | High       | Low           |

---

##  Key Features

* Real-time monitoring
* Automated water regulation
* Remote monitoring via cloud
* Low-cost implementation
* Scalable architecture
* Energy efficient system

---

##  Funding & Achievements

*  Funded by IIT Bombay (TIH) – ₹6,00,000
*  Design Patent Registered (Govt. of India)

  * Design No: 454051-001
* 📄 IEEE Publication
* 🧪 Successfully deployed in real pearl farm (Kolhapur)

---

##  Future Scope

* Machine Learning integration for predictive analysis
* Automatic dosing pump for pH control
* Water heater for temperature regulation
* Advanced filtration system
* Multi-tank monitoring system

---

##  Project Documentation

* IEEE Paper
* Project Report
* Presentation
* Block Diagram
* Flowchart
* Hardware Setup

---

##  Authors

* Atharav Ramchandra Todkar
* Swaroop Bharat Thikane
* Vedant Vishwanath Joshi
* Savita Sidram Totad
* Vaishnavi Raju Pawar

 Mentor: Dr. Jayashree P. Kharat

 DKTE Textile and Engineering Institute, Ichalkaranji

---

##  Conclusion

This project demonstrates how IoT can transform traditional pearl farming into a smart, automated, and sustainable system, improving productivity and reducing operational effort.

---
