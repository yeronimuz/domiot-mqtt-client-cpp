#pragma once

#include <Arduino.h>

class PubSubClient;

class TemperatureSensor {
    public:
        static float readTemperature();
        static void publishSensorValue(
            PubSubClient& mqttClient,
            long sensorId,
            const char* topic,
            const String& timestamp,
            unsigned long now,
            unsigned long lastPublish);

    private:
        static float _lastSentTemperature;
};
