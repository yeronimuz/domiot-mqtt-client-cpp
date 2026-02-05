#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Topic.h"
#include "ConfigParameter.h"

class Actuator
{
private:
    int _actuatorId;
    String _deviceMac;
    Topic _topic;
    String _type;
    std::vector<ConfigParameter> _parameters;

public:
    Actuator() = default;

    // Getters
    int getActuatorId() const { return _actuatorId; }
    String getDeviceMac() const { return _deviceMac; }
    const Topic& getTopic() const { return _topic; }
    String getType() const { return _type; }
    const std::vector<ConfigParameter>& getParameters() const { return _parameters; }

    // Setters
    void setActuatorId(int actuatorId) { _actuatorId = actuatorId; }
    void setDeviceMac(const String& deviceMac) { _deviceMac = deviceMac; }
    void setType(const String& type) { _type = type; }

    // Mutable access
    Topic& topic() { return _topic; }
    std::vector<ConfigParameter>& parameters() { return _parameters; }
};