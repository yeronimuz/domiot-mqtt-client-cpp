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
private:
    String _manufacturerId;
    String _modelId;
    String _firmwareVersion;
    String _hardwareVersion;
    String _macAddress;
    std::vector<ConfigParameter> _parameters;
    std::vector<Sensor> _sensors;
    std::vector<Actuator> _actuators;

public:
    Device()
        : _manufacturerId(""),
          _modelId(""),
          _firmwareVersion(""),
          _hardwareVersion(""),
          _macAddress("")
    {
    }

    // Getters
    String getManufacturerId() const { return _manufacturerId; }
    String getModelId() const { return _modelId; }
    String getFirmwareVersion() const { return _firmwareVersion; }
    String getHardwareVersion() const { return _hardwareVersion; }
    String getMacAddress() const { return _macAddress; }
    const std::vector<ConfigParameter>& getParameters() const { return _parameters; }
    const std::vector<Sensor>& getSensors() const { return _sensors; }
    const std::vector<Actuator>& getActuators() const { return _actuators; }

    long getSensorIdByType(SensorType type);
    bool hasUnassignedSensors() const;
    String toString(bool pretty = false) const;
    static bool hasUnassignedSensors(const Device& device) { return device.hasUnassignedSensors(); }

    // Setters
    void setManufacturerId(const String& manufacturerId) { _manufacturerId = manufacturerId; }
    void setModelId(const String& modelId) { _modelId = modelId; }
    void setFirmwareVersion(const String& firmwareVersion) { _firmwareVersion = firmwareVersion; }
    void setHardwareVersion(const String& hardwareVersion) { _hardwareVersion = hardwareVersion; }
    void setMacAddress(const String& macAddress) { _macAddress = macAddress; }
    void setParameters(const std::vector<ConfigParameter>& parameters) { _parameters = parameters; }
    void setSensors(const std::vector<Sensor>& sensors) { _sensors = sensors; }
    void setActuators(const std::vector<Actuator>& actuators) { _actuators = actuators; }

    // Mutable access for building/modifying
    std::vector<ConfigParameter>& parameters() { return _parameters; }
    std::vector<Sensor>& sensors() { return _sensors; }
    std::vector<Actuator>& actuators() { return _actuators; }

    static Device fromJson(const JsonObject &json);
    static void toJson(const Device &device, JsonDocument &doc);
    static void writeDeviceJson(JsonDocument &doc);
};
