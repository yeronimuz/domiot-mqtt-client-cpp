#pragma once

#include <Arduino.h>
#include "Topic.h"
#include "SensorType.h"
#include "ConfigParameter.h"
#include <vector>

class Sensor
{
public:
    int _sensorId;
    String _deviceMac;
    Topic _topic;
    SensorType _type;
    std::vector<ConfigParameter> _parameters;

    Sensor() = default;
    Sensor(int sensorId, String deviceMac, SensorType type)
        : _sensorId(sensorId), _deviceMac(deviceMac), _type(type) {}
};