#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <any>

class ConfigParameter
{
private:
    String _name;
    String _parameterType;
    std::any _value;
    bool _readonly;

public:
    ConfigParameter() = default;

    // Getters
    String getName() const { return _name; }
    String getParameterType() const { return _parameterType; }
    const std::any& getValue() const { return _value; }
    bool isReadonly() const { return _readonly; }

    // Setters
    void setName(const String& name) { _name = name; }
    void setParameterType(const String& parameterType) { _parameterType = parameterType; }
    void setValue(const std::any& value) { _value = value; }
    void setReadonly(bool readonly) { _readonly = readonly; }
};