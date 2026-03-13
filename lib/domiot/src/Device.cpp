#include "Device.h"

#include <cstdarg>
#include <cstdio>

enum class ParamType {
    NUMBER,
    BOOL,
    STRING
};

static ParamType getParamType(const String& typeStr) {
    String normalized = typeStr;
    normalized.toLowerCase();

    if (normalized == "number" || normalized == "int" || normalized == "integer" || normalized == "float" || normalized == "double") {
        return ParamType::NUMBER;
    }

    if (normalized == "bool" || normalized == "boolean") {
        return ParamType::BOOL;
    }

    return ParamType::STRING;
}

static void serialLogf(const char* format, ...) {
    char buffer[256];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Serial.print(buffer);
}

static void printDeviceConfigurationDetails(const Device& device) {
    serialLogf("Manufacturer: %s, Model: %s\n",
        device.getManufacturerId().c_str(),
        device.getModelId().c_str());
    serialLogf("Number of parameters: %d, Number of sensors: %d\n",
        device.getParameters().size(),
        device.getSensors().size());
    Serial.println("Device configuration details:");

    for (const ConfigParameter& param : device.getParameters()) {
        serialLogf("  Parameter: %s, Type: %s, Readonly: %s\n",
            param.getName().c_str(),
            param.getParameterType().c_str(),
            param.isReadonly() ? "true" : "false");
    }

    for (const Sensor& sensor : device.getSensors()) {
        const Topic& topic = sensor.getTopic();
        serialLogf("  Sensor ID: %ld, Type: %s, Topic Type: %s, Topic Path: %s\n",
            sensor.getSensorId(),
            sensor.getType().getDescription().c_str(),
            topic.getType().c_str(),
            topic.getPath().c_str());

        for (const ConfigParameter& param : sensor.getParameters()) {
            serialLogf("    Sensor Parameter: %s, Type: %s, Readonly: %s\n",
                param.getName().c_str(),
                param.getParameterType().c_str(),
                param.isReadonly() ? "true" : "false");
        }
    }
}

static bool anyToNumber(const std::any& value, float& outNumber)
{
    if (!value.has_value()) {
        return false;
    }

    if (const int* number = std::any_cast<int>(&value)) {
        outNumber = static_cast<float>(*number);
        return true;
    }

    if (const long* number = std::any_cast<long>(&value)) {
        outNumber = static_cast<float>(*number);
        return true;
    }

    if (const unsigned int* number = std::any_cast<unsigned int>(&value)) {
        outNumber = static_cast<float>(*number);
        return true;
    }

    if (const unsigned long* number = std::any_cast<unsigned long>(&value)) {
        outNumber = static_cast<float>(*number);
        return true;
    }

    if (const float* number = std::any_cast<float>(&value)) {
        outNumber = *number;
        return true;
    }

    if (const double* number = std::any_cast<double>(&value)) {
        outNumber = static_cast<float>(*number);
        return true;
    }

    if (const String* text = std::any_cast<String>(&value)) {
        outNumber = text->toFloat();
        return true;
    }

    if (const bool* boolValue = std::any_cast<bool>(&value)) {
        outNumber = *boolValue ? 1.0f : 0.0f;
        return true;
    }

    return false;
}

static bool anyToBool(const std::any& value, bool& outBool)
{
    if (!value.has_value()) {
        return false;
    }

    if (const bool* boolValue = std::any_cast<bool>(&value)) {
        outBool = *boolValue;
        return true;
    }

    if (const int* number = std::any_cast<int>(&value)) {
        outBool = (*number != 0);
        return true;
    }

    if (const long* number = std::any_cast<long>(&value)) {
        outBool = (*number != 0);
        return true;
    }

    if (const float* number = std::any_cast<float>(&value)) {
        outBool = (*number != 0.0f);
        return true;
    }

    if (const double* number = std::any_cast<double>(&value)) {
        outBool = (*number != 0.0);
        return true;
    }

    if (const String* text = std::any_cast<String>(&value)) {
        String normalized = *text;
        normalized.trim();
        normalized.toLowerCase();
        outBool = (normalized == "true" || normalized == "1" || normalized == "yes");
        return true;
    }

    return false;
}

static bool anyToString(const std::any& value, String& outString)
{
    if (!value.has_value()) {
        return false;
    }

    if (const String* text = std::any_cast<String>(&value)) {
        outString = *text;
        return true;
    }

    if (const bool* boolValue = std::any_cast<bool>(&value)) {
        outString = *boolValue ? "true" : "false";
        return true;
    }

    if (const int* number = std::any_cast<int>(&value)) {
        outString = String(*number);
        return true;
    }

    if (const long* number = std::any_cast<long>(&value)) {
        outString = String(*number);
        return true;
    }

    if (const unsigned int* number = std::any_cast<unsigned int>(&value)) {
        outString = String(*number);
        return true;
    }

    if (const unsigned long* number = std::any_cast<unsigned long>(&value)) {
        outString = String(*number);
        return true;
    }

    if (const float* number = std::any_cast<float>(&value)) {
        outString = String(*number);
        return true;
    }

    if (const double* number = std::any_cast<double>(&value)) {
        outString = String(*number);
        return true;
    }

    if (const char* const* text = std::any_cast<const char*>(&value)) {
        outString = (*text == nullptr) ? "" : String(*text);
        return true;
    }

    if (char* const* text = std::any_cast<char*>(&value)) {
        outString = (*text == nullptr) ? "" : String(*text);
        return true;
    }

    return false;
}

static void writeConfigParameterToJson(const ConfigParameter& param, JsonObject& paramObj)
{
    paramObj["name"] = param.getName();
    paramObj["parameterType"] = param.getParameterType();

    switch (getParamType(param.getParameterType())) {
        case ParamType::NUMBER: {
            float numberValue = 0.0f;
            if (anyToNumber(param.getValue(), numberValue)) {
                paramObj["value"] = numberValue;
            } else {
                paramObj["value"] = 0;
            }
            break;
        }
        case ParamType::BOOL: {
            bool boolValue = false;
            if (anyToBool(param.getValue(), boolValue)) {
                paramObj["value"] = boolValue;
            } else {
                paramObj["value"] = false;
            }
            break;
        }
        case ParamType::STRING:
        default: {
            String textValue;
            if (anyToString(param.getValue(), textValue)) {
                paramObj["value"] = textValue;
            } else {
                paramObj["value"] = "";
            }
            break;
        }
    }

    paramObj["readonly"] = param.isReadonly();
}

long Device::getSensorIdByType(SensorType type) {
    for (const Sensor& sensor : _sensors) {
        if (sensor.getType() == type) {
            return sensor.getSensorId();
        }
    }
    return 0; // Return 0 if not found, indicating no sensor of that type
}

bool Device::hasUnassignedSensors() const
{
    for (const Sensor& sensor : _sensors) {
        if (sensor.getSensorId() <= 0) {
            return true;
        }
    }

    return false;
}

Device Device::fromJson(const JsonObject& json) {
    Device device = Device();
    
    device._manufacturerId = json["manufacturerId"] | "";
    device._modelId = json["modelId"] | "";
    device._firmwareVersion = json["firmwareVersion"] | "";
    device._hardwareVersion = json["hardwareVersion"] | "";
    device._macAddress = json["macAddress"] | "";

    // Parse parameters
    JsonArray paramsArray = json["parameters"];
    if (!paramsArray.isNull()) {
        for (JsonObject paramObj : paramsArray) {
            ConfigParameter param;
            param.setName(paramObj["name"] | "");
            param.setParameterType(paramObj["parameterType"] | "");
            
            // Parse value based on type
            switch (getParamType(param.getParameterType())) {
                case ParamType::NUMBER:
                    param.setValue(paramObj["value"].as<float>());
                    break;
                case ParamType::BOOL:
                    param.setValue(paramObj["value"].as<bool>());
                    break;
                case ParamType::STRING:
                default:
                    param.setValue(String(paramObj["value"].as<const char*>()));
                    break;
            }
            
            param.setReadonly(paramObj["readonly"] | false);
            device._parameters.push_back(param);
        }
    }

    // Parse sensors
    JsonArray sensorsArray = json["sensors"];
    if (!sensorsArray.isNull()) {
        for (JsonObject sensorObj : sensorsArray) {
            Sensor sensor(sensorObj["sensorId"] | 0,
                sensorObj["deviceMac"] | "",
                SensorType::getType(sensorObj["type"] | "").value_or(SensorType::NOT_USED)
            );

            // Parse topic
            JsonObject topicObj = sensorObj["topic"];
            if (!topicObj.isNull()) {
                sensor.topic().setType(topicObj["type"] | "");
                sensor.topic().setPath(topicObj["path"] | "");
            }

            // Parse sensor parameters
            JsonArray sensorParamsArray = sensorObj["parameters"];
            if (!sensorParamsArray.isNull()) {
                for (JsonObject paramObj : sensorParamsArray) {
                    ConfigParameter param = ConfigParameter();
                    param.setName(paramObj["name"] | "");
                    param.setParameterType(paramObj["parameterType"] | "");
                    
                    // Parse value based on type
                    switch (getParamType(param.getParameterType())) {
                        case ParamType::NUMBER:
                            param.setValue(paramObj["value"].as<float>());
                            break;
                        case ParamType::BOOL:
                            param.setValue(paramObj["value"].as<bool>());
                            break;
                        case ParamType::STRING:
                        default:
                            param.setValue(String(paramObj["value"].as<const char*>()));
                            break;
                    }
                    
                    param.setReadonly(paramObj["readonly"] | false);
                    sensor.parameters().push_back(param);
                }
            }

            device._sensors.push_back(sensor);
        }
    }
    Serial.println("Device configuration parsed from JSON successfully.");
    printDeviceConfigurationDetails(device);
    return device;
}

void Device::toJson(const Device& device, JsonDocument& doc)
{
    JsonObject root = doc.to<JsonObject>();

    root["manufacturerId"] = device.getManufacturerId();
    root["modelId"] = device.getModelId();
    root["firmwareVersion"] = device.getFirmwareVersion();
    root["hardwareVersion"] = device.getHardwareVersion();
    root["macAddress"] = device.getMacAddress();

    JsonArray parametersArray = root["parameters"].to<JsonArray>();
    for (const ConfigParameter& param : device.getParameters()) {
        JsonObject paramObj = parametersArray.add<JsonObject>();
        writeConfigParameterToJson(param, paramObj);
    }

    JsonArray sensorsArray = root["sensors"].to<JsonArray>();
    for (const Sensor& sensor : device.getSensors()) {
        JsonObject sensorObj = sensorsArray.add<JsonObject>();
        sensorObj["sensorId"] = sensor.getSensorId();
        sensorObj["deviceMac"] = sensor.getDeviceMac();
        sensorObj["type"] = sensor.getType().getDescription();

        JsonObject topicObj = sensorObj["topic"].to<JsonObject>();
        topicObj["type"] = sensor.getTopic().getType();
        topicObj["path"] = sensor.getTopic().getPath();

        JsonArray sensorParamsArray = sensorObj["parameters"].to<JsonArray>();
        for (const ConfigParameter& param : sensor.getParameters()) {
            JsonObject paramObj = sensorParamsArray.add<JsonObject>();
            writeConfigParameterToJson(param, paramObj);
        }
    }

    JsonArray actuatorsArray = root["actuators"].to<JsonArray>();
    for (const Actuator& actuator : device.getActuators()) {
        JsonObject actuatorObj = actuatorsArray.add<JsonObject>();
        actuatorObj["actuatorId"] = actuator.getActuatorId();
        actuatorObj["deviceMac"] = actuator.getDeviceMac();
        actuatorObj["type"] = actuator.getType();

        JsonObject topicObj = actuatorObj["topic"].to<JsonObject>();
        topicObj["type"] = actuator.getTopic().getType();
        topicObj["path"] = actuator.getTopic().getPath();

        JsonArray actuatorParamsArray = actuatorObj["parameters"].to<JsonArray>();
        for (const ConfigParameter& param : actuator.getParameters()) {
            JsonObject paramObj = actuatorParamsArray.add<JsonObject>();
            writeConfigParameterToJson(param, paramObj);
        }
    }
}

void Device::writeDeviceJson(JsonDocument& doc) 
{
    File file = LittleFS.open("/device.json", "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    serializeJson(doc, file);

    Serial.println("device.json written");
}
