#include <Arduino.h>
#include <lvgl.h>
#include "sensors.h"
#include "weather.h"
#include "hardware.h"
#include "ui.h"

#include <TFT_eSPI.h>
extern TFT_eSPI tft;
extern lv_obj_t * tabview; 

// 1. Global Data Containers
IndoorData currentIndoor;
WeatherData currentOutdoor;

// 2. The Mutex (Our Data Lock)
SemaphoreHandle_t dataMutex;

// 3. Button Interrupt Variables
volatile bool buttonPressed = false;
unsigned long lastInterruptTime = 0;

void IRAM_ATTR handleButtonInterrupt() {
    unsigned long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200) { 
        buttonPressed = true;
        lastInterruptTime = interruptTime;
    }
}

// ==========================================
// --- CORE 0: THE WORKER (Sensors & API) ---
// ==========================================
void TaskWorker(void *pvParameters) {
    unsigned long lastSensorRead = 0;
    unsigned long lastWeatherFetch = 0;
    const unsigned long SENSOR_INTERVAL = 3000;
    const unsigned long WEATHER_INTERVAL = 900000;

    for (;;) {
        unsigned long currentMillis = millis();

        // TASK 1: Read Sensors
        if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
            lastSensorRead = currentMillis;
            
            // LOCK MUTEX: Update the data, then unlock
            if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
                read_sensors(&currentIndoor); 
                xSemaphoreGive(dataMutex);
            }
        }

        // TASK 2: Fetch Weather
        if (currentMillis - lastWeatherFetch >= WEATHER_INTERVAL) {
            lastWeatherFetch = currentMillis;
            
            // LOCK MUTEX: Update the data, then unlock
            if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
                fetch_weather_data(&currentOutdoor);
                xSemaphoreGive(dataMutex);
            }
        }

        // Yield the core to prevent Watchdog Timer crashes
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// ==========================================
// --- CORE 1: THE ARTIST (LVGL & UI) ---
// ==========================================
void TaskUI(void *pvParameters) {
    unsigned long lastUIUpdate = 0;
    const unsigned long UI_UPDATE_INTERVAL = 60000; // Update screen numbers once a minute

    for (;;) {
        unsigned long currentMillis = millis();

        // TASK 3: Update Screen UI Numbers
        if (currentMillis - lastUIUpdate >= UI_UPDATE_INTERVAL) {
            lastUIUpdate = currentMillis;
            
            // LOCK MUTEX: Read the data, draw to screen, then unlock
            if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
                update_ui(&currentIndoor, &currentOutdoor);
                xSemaphoreGive(dataMutex);
            }
        }

        // TASK 4: Process Instant Button Presses
        if (buttonPressed) {
            buttonPressed = false; 
            
            uint16_t current_tab = lv_tabview_get_tab_act(tabview);
            uint16_t next_tab = (current_tab + 1) % 3;
            lv_tabview_set_act(tabview, next_tab, LV_ANIM_OFF); 
            
            Serial.printf("Instant Button press! Cycling to tab %d\n", next_tab);
            
            // Force the screen data to refresh instantly when you switch tabs
            if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
                update_ui(&currentIndoor, &currentOutdoor);
                xSemaphoreGive(dataMutex);
            }
            lastUIUpdate = millis(); // Reset the UI timer
        }

        // TASK 5: The LVGL Heartbeat (Runs at maximum speed unhindered!)
        lv_timer_handler();  

        // Yield the core for 5ms
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

// ==========================================
// --- BOOT SEQUENCE ---
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(1000); 

    // Create the Mutex lock
    dataMutex = xSemaphoreCreateMutex();

    Serial.println("Starting IAQ Monitor Boot Sequence...");
    init_hardware(); 
    
    // --- BACKLIGHT OVERRIDE ---
    ledcSetup(0, 5000, 8);      // Channel 0, 5kHz, 8-bit resolution
    ledcAttachPin(32, 0);       // Attach GPIO 32 (LED pin) to Channel 0
    ledcWrite(0, 255);          // Force 100% brightness (255)
    // ------------------------------
    
    init_ui();         
    init_sensors();    
    init_weather_api(); 

    // Attach hardware interrupt
    attachInterrupt(digitalPinToInterrupt(27), handleButtonInterrupt, FALLING);

    // Initial Weather Fetch & UI Update securely
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    fetch_weather_data(&currentOutdoor);
    update_ui(&currentIndoor, &currentOutdoor);
    xSemaphoreGive(dataMutex);

    Serial.println("System Ready. Launching Dual Cores...");

    // Pin the Worker Task to Core 0 (Allocating 8KB of RAM)
    xTaskCreatePinnedToCore(TaskWorker, "Worker", 8192, NULL, 1, NULL, 0);
    
    // Pin the UI Task to Core 1 (Allocating 16KB of RAM for graphics)
    xTaskCreatePinnedToCore(TaskUI, "UI", 16384, NULL, 1, NULL, 1);
}
 
void loop() {
    vTaskDelete(NULL); 
}