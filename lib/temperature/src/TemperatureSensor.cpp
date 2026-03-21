#include "TemperatureSensor.h"
#include "constants.h"
#include <Arduino.h>
#include <PubSubClient.h>

float TemperatureSensor::_lastSentTemperature = 0.0f;

float TemperatureSensor::readTemperature() {
    int adcValue = analogRead(THERMISTOR_PIN);
    float resistance = SERIES_RESISTOR / ((1023.0 / adcValue) - 1);

    float steinhart;
    steinhart = resistance / NOMINAL_RESISTANCE;  // (R/Ro)
    steinhart = log(steinhart);                  // ln(R/Ro)
    steinhart /= BETA_COEFFICIENT;               // 1/B * ln(R/Ro)
    steinhart += 1.0 / (NOMINAL_TEMPERATURE + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart;                 // Invert
    steinhart -= 273.15;                         // Convert to °C

    return steinhart;
}

void TemperatureSensor::publishSensorValue(
    PubSubClient& mqttClient,
    long sensorId,
    const char* topic,
    const String& timestamp,
    unsigned long now,
    unsigned long lastPublish)
{
    if (sensorId == 0 || now - lastPublish < 1000)
    {
        return;
    }

    float temperature = readTemperature();
    if (temperature == _lastSentTemperature)
    {
        return;
    }

    _lastSentTemperature = temperature;
    String payload = "{";
    payload += "\"sensorId\": " + String(sensorId) + ", ";
    payload += "\"timestamp\": \"" + timestamp + "\", ";
    payload += "\"value\": " + String(temperature, 2);
    payload += "}";

    mqttClient.publish(topic, reinterpret_cast<const uint8_t*>(payload.c_str()), payload.length());
}