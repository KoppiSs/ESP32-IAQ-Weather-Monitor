#include "weather.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

// Wi-Fi credentials (template). Actual credentials in non-public "secrets.h" file.
//const char* WIFI_SSID = "WiFiName";
//const char* WIFI_PASS = "WifiPassword";

// Open-Meteo API URL configured for Espoo, Finland
const char* API_URL = "https://api.open-meteo.com/v1/forecast?latitude=60.21&longitude=24.66&current=temperature_2m,relative_humidity_2m,wind_speed_10m,weather_code&hourly=temperature_2m,weather_code&forecast_hours=12&timezone=Europe%2FHelsinki";

void init_weather_api() {
    Serial.print("Connecting to Wi-Fi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    // Attempt connection for ~10 seconds without permanently blocking the system
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWi-Fi Connected Successfully!");
    } else {
        Serial.println("\nWi-Fi Connection Failed.");
    }
}

void fetch_weather_data(WeatherData* data) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi disconnected. Cannot fetch weather.");
        return;
    }

    HTTPClient http;
    http.begin(API_URL);
    
    int httpResponseCode = http.GET();
    
    if (httpResponseCode == 200) {
        // The API responded successfully, grab the raw JSON text
        String payload = http.getString();
        
        // ArduinoJson v7 automatically allocates memory for the document
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Serial.print("JSON Parsing failed: ");
            Serial.println(error.c_str());
            http.end();
            return;
        }

        // Drill down into the JSON tree to extract target values for current weather
        data->outdoor_temp = doc["current"]["temperature_2m"];
        data->outdoor_humidity = doc["current"]["relative_humidity_2m"];
        data->wind_speed = doc["current"]["wind_speed_10m"];
        data->weather_code = doc["current"]["weather_code"];

        // And for hourly forecast data (next 12 hours)
        for (int i = 0; i < 12; i++) {
            String rawTime = doc["hourly"]["time"][i].as<String>(); // Grab the raw time string (e.g., "2026-07-06T14:00")
            data->hourly_times[i] = rawTime.substring(11, 16);      // Extract just the "14:00" part (Index 11 to 15)
            data->hourly_temps[i] = doc["hourly"]["temperature_2m"][i];
            data->hourly_codes[i] = doc["hourly"]["weather_code"][i];
        }
        
        Serial.println("Current and hourly weather fetched successfully.");

        // Debugging prints
        Serial.printf("Weather Fetched: %.1fC, %d%% Humidity\n", data->outdoor_temp, data->outdoor_humidity);
        Serial.printf("Forecast at %s: %.1fC\n", data->hourly_times[3].c_str(), data->hourly_temps[3]);

    } else {
        Serial.printf("API Request Error. HTTP Code: %d\n", httpResponseCode);
    }
    
    // Free the HTTP resources
    http.end();
}

// Translate the raw integer of weather_code into a human-readable string
String decode_weather(int code) {
    switch (code) {
        case 0: return "Clear Sky";
        case 1: case 2: case 3: return "Partly Cloudy";
        case 45: case 48: return "Fog";
        case 51: case 53: case 55: return "Drizzle";
        case 56: case 57: return "Freezing Drizzle";
        case 61: case 63: case 65: return "Rain";
        case 66: case 67: return "Freezing Rain";
        case 71: case 73: case 75: return "Snow";
        case 77: return "Snow Grains";
        case 80: case 81: case 82: return "Rain Showers";
        case 85: case 86: return "Snow Showers";
        case 95: case 96: case 99: return "Thunderstorm";
        default: return "Unknown";
    }
}