#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Topic.h"
#include "ConfigParameter.h"

class Actuator
{
public:
    int _actuatorId;
    String _deviceMac;
    Topic _topic;
    String _type;
    std::vector<ConfigParameter> _parameters;

    Actuator() = default;
};