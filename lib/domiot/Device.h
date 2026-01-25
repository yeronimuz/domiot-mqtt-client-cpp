#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <LittleFS.h>

class Topic
{
public:
    String type;
    String path;

    Topic() = default;
    Topic(const String &type, const String &path) : type(type), path(path) {}
};

class SensorParameter
{
public:
    String name;
    String parameterType;
    JsonVariant value;
    bool readonly;

    SensorParameter() = default;
};

class Sensor
{
public:
    int sensorId;
    String deviceMac;
    Topic topic;
    String type;
    std::vector<SensorParameter> parameters;

    Sensor() = default;
};

class DeviceParameter
{
public:
    String name;
    String parameterType;
    JsonVariant value;
    bool readonly;

    DeviceParameter() = default;
};

class Device
{
public:
    long deviceId;
    String manufacturerId;
    String modelId;
    String firmwareVersion;
    String hardwareVersion;
    String macAddress;
    std::vector<DeviceParameter> parameters;
    std::vector<Sensor> sensors;

    Device() = default;

    static Device fromJson(const JsonObject &json);
    static void toJson(const Device &device);
    static void writeDeviceJson(JsonDocument &doc);
};
