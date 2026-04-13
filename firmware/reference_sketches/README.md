# Reference Sketches

These sketches are **not part of the final deployed system**. They were developed during the iterative design phase of the project and are kept here to document the development journey.

---

## Development Timeline

| Sketch | Board | Purpose |
|---|---|---|
| `ph_calibration_test.ino` | Arduino Uno | First test — verified DFRobot pH sensor readings against pH 4.0 and 7.0 buffer solutions |
| `tds_only.ino` | ESP32 | Second iteration — tested TDS sensor independently with Blynk and ThingSpeak upload |
| `tds_ph_combined.ino` | ESP32 | Third iteration — combined TDS and pH on one node before temperature was added |

---

## Why We Kept These

Each sketch represents a real problem we solved:

- **`ph_calibration_test.ino`** — helped us understand the Nernst-based voltage-to-pH conversion and validate our calibration procedure before integrating into the main system
- **`tds_only.ino`** — isolated TDS testing revealed the ADC2/Wi-Fi conflict on ESP32, which led to our decision to use only ADC1 channels in the final design
- **`tds_ph_combined.ino`** — combining TDS and pH on one ESP32 showed us timing and noise issues that motivated the final two-node ESP-NOW architecture

---

## Final Deployed Firmware

See [`firmware/esp32_main_node/`](../esp32_main_node/) and [`firmware/esp32_actuator_node/`](../esp32_actuator_node/) for the production code.
