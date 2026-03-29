#pragma once

#include <Arduino.h>
#include "Topic.h"
#include "SensorType.h"
#include "ConfigParameter.h"
#include <vector>

class Sensor
{
private:
    int _sensorId;
    String _deviceMac;
    Topic _topic;
    SensorType _type;
    std::vector<ConfigParameter> _parameters;

public:
    Sensor() = default;
    Sensor(int sensorId, String deviceMac, SensorType type)
        : _sensorId(sensorId), _deviceMac(deviceMac), _type(type) {}

    // Getters
    int getSensorId() const { return _sensorId; }
    String getDeviceMac() const { return _deviceMac; }
    const Topic& getTopic() const { return _topic; }
    SensorType getType() const { return _type; }
    const std::vector<ConfigParameter>& getParameters() const { return _parameters; }

    // Setters
    void setSensorId(int sensorId) { _sensorId = sensorId; }
    void setDeviceMac(const String& deviceMac) { _deviceMac = deviceMac; }
    void setType(SensorType type) { _type = type; }

    // Mutable access
    Topic& topic() { return _topic; }
    std::vector<ConfigParameter>& parameters() { return _parameters; }
};