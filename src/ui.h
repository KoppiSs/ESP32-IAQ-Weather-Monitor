#ifndef UI_H
#define UI_H

#include "sensors.h"
#include "weather.h"

// Initialize the visual layout
void init_ui();

// Pass the memory addresses of our data structs to update the text
void update_ui(IndoorData* indoor, WeatherData* outdoor);

#endif