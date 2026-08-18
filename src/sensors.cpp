#include "sensors.h"
#include <Arduino.h>
#include <Wire.h>
#include <bsec.h> // The official Bosch BSEC Library

// Instantiate the BSEC sensor object
Bsec iaqSensor;

// Helper function to print any BSEC errors to the Serial Monitor
void checkIaqSensorStatus(void) {
    if (iaqSensor.bsecStatus != BSEC_OK) {
        if (iaqSensor.bsecStatus < BSEC_OK) {
            Serial.printf("BSEC error code : %d\n", iaqSensor.bsecStatus);
        } else {
            Serial.printf("BSEC warning code : %d\n", iaqSensor.bsecStatus);
        }
    }

    if (iaqSensor.bme68xStatus != BME68X_OK) {
        if (iaqSensor.bme68xStatus < BME68X_OK) {
            Serial.printf("BME68X error code : %d\n", iaqSensor.bme68xStatus);
        } else {
            Serial.printf("BME68X warning code : %d\n", iaqSensor.bme68xStatus);
        }
    }
}

void init_sensors() {
    // Start the I2C bus on the ESP32's default pins (SDA=21, SCL=22)
    Wire.begin();

    // Start the sensor. 
    // NOTE: BME68X_I2C_ADDR_LOW is 0x76. If specific breakout board 
    // uses 0x77, change this to BME68X_I2C_ADDR_HIGH.
    iaqSensor.begin(BME68X_I2C_ADDR_HIGH, Wire);
    
    String output = "\nBSEC library version " +
                    String(iaqSensor.version.major) + "." +
                    String(iaqSensor.version.minor) + "." +
                    String(iaqSensor.version.major_bugfix) + "." +
                    String(iaqSensor.version.minor_bugfix);
    Serial.println(output);

    checkIaqSensorStatus();

    // Tell the BSEC algorithm which data points we want it to calculate
    bsec_virtual_sensor_t sensorList[6] = {
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,    //Temp
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,       //Humidity
        BSEC_OUTPUT_RAW_PRESSURE,                           //Pressure
        BSEC_OUTPUT_RAW_GAS,                                //Raw VOC (Ohms)
        BSEC_OUTPUT_IAQ,                                    //IAQ Index
        BSEC_OUTPUT_BREATH_VOC_EQUIVALENT                   //VOC in ppm           
    };

    // Subscribe to these outputs with a Low Power (LP) sample rate (every 3 seconds)
    // Ultra-Low Power Mode (ULP) is every 5 minutes
    iaqSensor.updateSubscription(sensorList, 6, BSEC_SAMPLE_RATE_LP);
    checkIaqSensorStatus();
    
    Serial.println("BME680 Initialized Successfully.");
}

void read_sensors(IndoorData* data) {
    // iaqSensor.run() is NON-BLOCKING. It returns true ONLY if new data is ready.
    if (iaqSensor.run()) { 
        // Store the compensated outputs into the custom struct
        data->temperature = iaqSensor.temperature;
        data->humidity = iaqSensor.humidity;
        data->pressure = iaqSensor.pressure / 100.0; // Convert Pascals to hPa (Millibars)
        data->gas_resistance = iaqSensor.gasResistance / 1000.0; // Convert to kOhms
        data->iaq_estimate = iaqSensor.iaq;
        data->breath_voc = iaqSensor.breathVocEquivalent;

        // Print to the serial monitor for debugging
        Serial.printf("Indoor Data -> Temp: %.1fC | Hum: %.1f%% | IAQ Index: %.0f | VOC: %.2f ppm\n | Accuracy: %d\n", 
                      data->temperature, data->humidity, data->iaq_estimate, data->breath_voc, iaqSensor.iaqAccuracy);
    }
}