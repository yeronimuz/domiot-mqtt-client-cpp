#pragma once

// Keys for Preferences
#pragma once
#define PREF_WIFI_KEY   "WIFI"
#define PREF_MQTT_KEY   "MQTT"
#define PREF_SENSOR_ID_KEY "PREF_SENSOR_ID"

// Thresholds battery level for 3.7V batteries
#define MAX_VOLTAGE     4.2  // Full battery voltage
#define MIN_VOLTAGE     3.3  // Empty battery voltage

#define THERMISTOR_PIN A0
#define DHT11_PIN D7 // The ESP8266 pin D7 connected to DHT11 sensor 

// Temperature constants
#define SERIES_RESISTOR 10000.0  // 10kΩ series resistor
#define NOMINAL_RESISTANCE 10000.0 // 10kΩ at 25°C
#define NOMINAL_TEMPERATURE 25.0
#define BETA_COEFFICIENT 3950.0  // Beta value
