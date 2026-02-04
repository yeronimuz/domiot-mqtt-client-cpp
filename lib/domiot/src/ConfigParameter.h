#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <any>

class ConfigParameter
{
public:
    String _name;
    String _parameterType;
    std::any _value;
    bool _readonly;

    ConfigParameter() = default;
};