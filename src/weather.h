#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>

// Central container for outdoor weather data
typedef struct {
    // Current weather
    float outdoor_temp;
    int outdoor_humidity;
    float wind_speed;
    int weather_code;   // Raw WMO integer for decoding general weather state
    // Hourly forecast arrays
    String hourly_times[12];
    float hourly_temps[12];
    int hourly_codes[12];
} WeatherData;

// Function prototypes
void init_weather_api();
void fetch_weather_data(WeatherData* data);
String decode_weather(int code);    // Helper to translate weather_code to a word

#endif