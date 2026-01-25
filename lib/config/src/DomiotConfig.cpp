#include "DomiotConfig.h"
#include <ArduinoJson.h>
#include "Device.h"

DomiotConfig::DomiotConfig()
{
    // Read WifiConfig and MqttConfig from LittleFS
    // Read DeviceConfig from LittleFS when exists
    if (!LittleFS.begin())
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    getDomiotConfig();
}

void DomiotConfig::getDomiotConfig()
{
    File file = LittleFS.open("/config.json", "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, static_cast<Stream &>(file));
    file.close();

    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
    }
    else
    {
        _domiotJson = doc;

        _wifiConfig = WifiConfig(doc["ssid"].as<String>(), doc["password"].as<String>());

        _mqttConfig = MqttConfig(
            doc["host"].as<String>(),
            doc["port"].as<u_short>(),
            doc["user"].as<String>(),
            doc["password"].as<String>(),
            doc["client_id"].as<String>());
    }

    // Read device config from LittleFS when exists
    if (LittleFS.exists("/device_config.json"))
    {
        File deviceFile = LittleFS.open("/device_config.json", "r");
        if (!deviceFile)
        {
            Serial.println("Failed to open device_config.json for reading");
            return;
        }

        JsonDocument doc;
        deserializeJson(doc, static_cast<Stream &>(deviceFile));
        Device device = Device::fromJson(doc.as<JsonObject>());
        deviceFile.close();
    }
}

