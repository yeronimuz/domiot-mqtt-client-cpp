#pragma once
#include <Arduino.h>

class SensorValue
{
private:
    long sensorId;
    String timestamp;
    float value;

public:
    SensorValue() = default;

    // Getters
    long getSensorId() const { return sensorId; }
    String getTimestamp() const { return timestamp; }
    float getValue() const { return value; }

    // Setters
    void setSensorId(long id) { sensorId = id; }
    void setTimestamp(const String& ts) { timestamp = ts; }
    void setValue(float val) { value = val; }
    String toString() const
    {
        return "SensorValue { sensorId: " + String(sensorId) + ", timestamp: " + timestamp + ", value: " + String(value, 2) + " }";
    }   
};
