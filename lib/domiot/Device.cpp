#include "Device.h"

Device Device::fromJson(const JsonObject& json) {
    Device device;
    
    device.deviceId = json["deviceId"] | 0;
    device.manufacturerId = json["manufacturerId"].isNull() ? "" : json["manufacturerId"].as<String>();
    device.modelId = json["modelId"].isNull() ? "" : json["modelId"].as<String>();
    device.firmwareVersion = json["firmwareVersion"].isNull() ? "" : json["firmwareVersion"].as<String>();
    device.hardwareVersion = json["hardwareVersion"] | "";
    device.macAddress = json["macAddress"] | "";

    // Parse parameters
    JsonArray paramsArray = json["parameters"];
    if (!paramsArray.isNull()) {
        for (JsonObject paramObj : paramsArray) {
            DeviceParameter param;
            param.name = paramObj["name"] | "";
            param.parameterType = paramObj["parameterType"] | "";
            param.value = paramObj["value"];
            param.readonly = paramObj["readonly"] | false;
            device.parameters.push_back(param);
        }
    }

    // Parse sensors
    JsonArray sensorsArray = json["sensors"];
    if (!sensorsArray.isNull()) {
        for (JsonObject sensorObj : sensorsArray) {
            Sensor sensor;
            sensor.sensorId = sensorObj["sensorId"] | 0;
            sensor.deviceMac = sensorObj["deviceMac"].isNull() ? "" : sensorObj["deviceMac"].as<String>();
            sensor.type = sensorObj["type"] | "";

            // Parse topic
            JsonObject topicObj = sensorObj["topic"];
            if (!topicObj.isNull()) {
                sensor.topic.type = topicObj["type"] | "";
                sensor.topic.path = topicObj["path"] | "";
            }

            // Parse sensor parameters
            JsonArray sensorParamsArray = sensorObj["parameters"];
            if (!sensorParamsArray.isNull()) {
                for (JsonObject paramObj : sensorParamsArray) {
                    SensorParameter param;
                    param.name = paramObj["name"] | "";
                    param.parameterType = paramObj["parameterType"] | "";
                    param.value = paramObj["value"];
                    param.readonly = paramObj["readonly"] | false;
                    sensor.parameters.push_back(param);
                }
            }

            device.sensors.push_back(sensor);
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

    file.close();
    Serial.println("device_config.json written");
}
