# 🌍 Dual-Core ESP32 IAQ & Weather Dashboard

<img width="4624" height="3468" alt="20260818_174405" src="https://github.com/user-attachments/assets/689c4f44-4fb4-43fe-952b-f6ec5833952e" />

A custom-built, multi-threaded environmental dashboard that merges highly accurate indoor air quality (IAQ) metrics with live API weather forecasts. Built on an ESP32 using FreeRTOS for task scheduling and LVGL for a responsive, hardware-accelerated user interface.

## 🚀 Objective
I wanted a reliable, local air quality monitor that didn't rely on closed commercial ecosystems. This project bridges the gap between low-level hardware design (soldering, power delivery) and embedded software engineering (multithreading, I2C/SPI protocols, and UI rendering).

## ⚡ Features
* **Real-Time IAQ Tracking:** Measures VOCs, Temperature, and Humidity via a Bosch BME680 sensor.
* **Live Weather Integration:** Fetches current conditions and hourly forecasts via the Open-Meteo REST API.
* **Dual-Core Processing:** Utilizes FreeRTOS to pin heavy network/sensor tasks to Core 0, keeping the LVGL graphics engine running unhindered at maximum framerate on Core 1.
* **Hardware Interrupts:** Physical tactile button for instant UI tab switching, bypassing software delays.

---

## 🛠️ System Architecture

### Hardware Stack
* **Microcontroller:** ESP32 NodeMCU-32S
* **Sensors:** Bosch BME680 (I2C interface)
* **Display:** 3.5" SPI TFT LCD with Touch Controller
* **Power Delivery:** Custom perfboard motherboard with dedicated 3.3V and GND copper rails.

<img width="6936" height="4624" alt="merged" src="https://github.com/user-attachments/assets/1faac614-a33c-488c-9185-73ea81b2cb8c" />

### Software Stack
* **C++ / PlatformIO**
* **FreeRTOS** (Task scheduling & Mutex semaphores)
* **LVGL** (Light and Versatile Graphics Library)
* **BSEC Library** (Bosch Sensortec Environmental Cluster for VOC calculation and thermal offsets)

---

## 🐛 Challenges & Engineering Solutions

Building this from scratch presented several hardware and software challenges that required active debugging:

**1. Mutex Contention & UI Freezing**
* **Issue:** Initially, the physical UI button felt sluggish. 
* **Diagnosis:** Core 0 was locking the shared data Mutex while waiting 2-3 seconds for the Wi-Fi weather API response and the BME680 sensor readings. When the hardware interrupt triggered, Core 1 was forced to wait for the Mutex to unlock before redrawing the screen.
* **Solution:** Refactored the network task to fetch data into local variables *first*, only locking the Mutex for ~1 microsecond to copy the final data over. The UI is now instantly responsive.

**2. Power Delivery & Signal Integrity**
* **Issue:** Moving from the breadboard to the soldered perfboard resulted in a dark screen, despite successful touch inputs.
* **Diagnosis:** Used a multimeter to track voltage drops and resistance across the custom copper rails, identifying a cold solder joint on the main GND rail that was choking the 80mA required for the backlight.
* **Solution:** Reflowed the main power rails for solid current delivery. Additionally, I daisy-chained the shared SPI bus lines (MOSI/SCK) between the high-speed display and low-speed touch components to maintain signal integrity and prevent echoing.

---

## ⚙️ How to Build & Flash

1. Clone this repository.
2. Uncomment the WiFi credentials template at the top of weather.cpp. Remember to remove the #include "secrets.h" line too.
3. Add your 2.4GHz Wi-Fi credentials to the template.
4. Build and upload using PlatformIO.

*Note: The BME680 sensor requires a roughly 48-hour continuous "burn-in" period upon first boot to calibrate the VOC gas resistance baseline.*
