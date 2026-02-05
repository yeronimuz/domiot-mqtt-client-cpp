#include "Device.h"

enum class ParamType {
    INT,
    FLOAT,
    BOOL,
    STRING
};

static ParamType getParamType(const String& typeStr) {
    if (typeStr == "int" || typeStr == "integer") return ParamType::INT;
    if (typeStr == "float" || typeStr == "double") return ParamType::FLOAT;
    if (typeStr == "bool" || typeStr == "boolean") return ParamType::BOOL;

    return ParamType::STRING;
}

Device Device::fromJson(const JsonObject& json) {
    Device device = Device();
    
    device._deviceId = json["deviceId"] | 0;
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
                case ParamType::INT:
                    param.setValue(paramObj["value"].as<int>());
                    break;
                case ParamType::FLOAT:
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
                        case ParamType::INT:
                            param.setValue(paramObj["value"].as<int>());
                            break;
                        case ParamType::FLOAT:
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

    return device;
}

void Device::writeDeviceJson(JsonDocument& doc) 
{
    File file = LittleFS.open("/device_config.json", "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    serializeJson(doc, file);

    Serial.println("device_config.json written");
}
