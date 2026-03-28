##  System Architecture

The system follows a distributed IoT architecture using multiple ESP32 microcontrollers for efficient monitoring and control.

###  Node Structure

* **Node 1 (Sensing Node):**
  Measures pH and temperature using DFRobot pH sensor and DS18B20 temperature sensor

* **Node 2 (Sensing Node):**
  Measures Total Dissolved Solids (TDS) using Keyestudio TDS sensor

* **Main Node (Processing Unit):**
  Receives data from all nodes, processes it, displays values on a 20x4 LCD, and uploads data to the cloud

* **Control Node (Actuation Unit):**
  Controls oxygen pump and wave generator using relay modules

---

###  Communication

* ESP-NOW protocol is used for communication between ESP32 nodes
* Provides fast, low-power, and connectionless data transmission
* No external Wi-Fi router required

---

###  Cloud Integration

* Data is sent to the Blynk IoT platform
* Enables real-time remote monitoring
* Provides alerts and visualization of parameters

---

###  Automation Logic

The system continuously monitors water parameters and performs automatic control actions:

* pH < 5.9 or pH > 8 → Activate water correction system
* TDS > 550 ppm → Drain and refill tank automatically
* Oxygen pump → Runs every 5 minutes
* Wave generator → Runs every 30 seconds

---

###  Key Advantages

* Distributed architecture ensures reliability
* Real-time processing and quick response
* Low power and cost-efficient design
* Scalable for multiple tanks
