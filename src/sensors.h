#ifndef SENSORS_H
#define SENSORS_H

// Central container for local environment data
typedef struct {
    float temperature;
    float humidity;
    float pressure;
    float gas_resistance;  // Raw VOC reading
    float breath_voc;       //Actual VOC 
    float iaq_estimate;    // Bosch's calculated Indoor Air Quality index
} IndoorData;

// Function prototypes
void init_sensors();
void read_sensors(IndoorData* data);

#endif