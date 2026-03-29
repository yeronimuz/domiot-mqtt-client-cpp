#pragma once

#include <Arduino.h>

class PubSubClient;

// Battery level constants
#define BATTERY_PIN A0

class BatteryLevel {
    public:
        static float readBatteryLevel();
        static void publishSensorValue(
            PubSubClient& mqttClient,
            long sensorId,
            const char* topic,
            const String& timestamp,
            unsigned long now,
            unsigned long lastPublish);

    private:
        static float _lastSentBatteryLevel;
};
