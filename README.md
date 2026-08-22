# Dual-Core ESP32 IAQ & Weather Dashboard

<img width="800" height="450" alt="UIshowcase-optimized" src="https://github.com/user-attachments/assets/3fe5665f-257e-4bcc-98a6-26ae885cf0c6" />

A custom-built, multi-threaded environmental dashboard that merges highly accurate indoor air quality (IAQ) metrics with live API weather forecasts. Built on an ESP32 using FreeRTOS for task scheduling and LVGL for a responsive, hardware-accelerated user interface.

## 🚀 Objective
I wanted a reliable, local air quality monitor that didn't rely on closed commercial ecosystems. This project bridges the gap between low-level hardware design (soldering, power delivery) and embedded software engineering (multithreading, I2C/SPI protocols, and UI rendering).

## ⚡ Features
* **Real-Time IAQ Tracking:** Measures VOCs, Temperature, Barometric Pressure, and Humidity via a Bosch BME680 sensor.
* **Live Weather Integration:** Fetches current conditions and hourly forecasts via the Open-Meteo API.
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
* **BSEC Library** (Bosch Sensortec Environmental Cluster for VOC calculation)

---

## 🐛 Challenges & Engineering Solutions

Building this from scratch presented several hardware and software challenges that required active debugging:

**1. Mutex Contention & UI Freezing**
* **Issue:** Initially, the UI interactions felt sluggish.
* **Diagnosis:** Core 0 was struggling to update the UI based on user feedback while waiting for the Wi-Fi weather API response and the continuous BME680 sensor readings.
* **Solution:** Set up a dual-core system (Core 0 API/Sensor, Core 1 UI/LVGL). Refactored task scheduling and utilized Mutex semaphores. The UI is now instantly responsive.

**2. Power Delivery & Signal Integrity**
* **Issue:** Moving from the breadboard to the soldered perfboard resulted in a dark screen, despite successful touch inputs.
* **Diagnosis:** Used a multimeter to track voltage drops and resistance across the custom copper rails, identifying a cold solder joint on the main GND rail that was choking the power required for the backlight.
* **Solution:** Reflowed the main power rails for solid current delivery. Additionally, I daisy-chained the shared SPI bus lines (MOSI/SCK) between the high-speed display and low-speed touch components to maintain signal integrity and prevent echoing.

---

## ⚙️ How to Build & Flash

1. Clone this repository.
2. Uncomment the Wi-Fi credentials template at the top of weather.cpp. Remember to remove the #include "secrets.h" line too.
3. Add your 2.4GHz Wi-Fi credentials to the template.
4. Build and upload using PlatformIO.

*Note: The BME680 sensor requires a roughly 48-hour continuous "burn-in" period upon first boot to calibrate the VOC gas resistance baseline.*
