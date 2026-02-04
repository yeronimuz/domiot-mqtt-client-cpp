#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <LittleFS.h>
#include "SensorType.h"
#include "ConfigParameter.h"
#include "Sensor.h"
#include "Actuator.h"


class Device
{
public:
    long _deviceId;
    String _manufacturerId;
    String _modelId;
    String _firmwareVersion;
    String _hardwareVersion;
    String _macAddress;
    std::vector<ConfigParameter> _parameters;
    std::vector<Sensor> _sensors;
    std::vector<Actuator> _actuators;

    Device() = default;

    static Device fromJson(const JsonObject &json);
    static void toJson(const Device &device);
    static void writeDeviceJson(JsonDocument &doc);
};
