#include "BatteryLevel.h"
#include "constants.h"
#include <Arduino.h>
#include <PubSubClient.h>

float BatteryLevel::_lastSentBatteryLevel = 0.0f;

float BatteryLevel::readBatteryLevel() {
    int adcValue = analogRead(BATTERY_PIN);
    float voltage = (adcValue / 1023.0) * 5.0;  // Assuming a 5V reference
    float batteryPercentage = (voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE) * 100.0;

    return constrain(batteryPercentage, 0, 100);
}

void BatteryLevel::publishSensorValue(
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

    float batteryLevel = readBatteryLevel();
    if (batteryLevel == _lastSentBatteryLevel)
    {
        return;
    }

    _lastSentBatteryLevel = batteryLevel;
    String payload = "{";
    payload += "\"sensorId\": " + String(sensorId) + ", ";
    payload += "\"timestamp\": \"" + timestamp + "\", ";
    payload += "\"value\": " + String(batteryLevel, 2);
    payload += "}";

    mqttClient.publish(topic, reinterpret_cast<const uint8_t*>(payload.c_str()), payload.length());
}