#pragma once
#include <Arduino.h>

class SensorValue
{
public:
    long sensorId;
    String timestamp;
    float value;

    SensorValue() = default;
};
