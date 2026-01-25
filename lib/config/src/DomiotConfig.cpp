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

        _wifiConfig = WifiConfig(doc["wifi"]["ssid"].as<String>(), doc["wifi"]["password"].as<String>());

        _mqttConfig = MqttConfig(
            doc["mqtt"]["host"].as<String>(),
            doc["mqtt"]["port"].as<u_short>(),
            doc["mqtt"]["user"].as<String>(),
            doc["mqtt"]["password"].as<String>(),
            doc["mqtt"]["client_id"].as<String>());
    }

    // Read device config from LittleFS when exists
    if (LittleFS.exists("/device_config.json"))
    {
        File deviceFile = LittleFS.open("/device.json", "r");
        if (!deviceFile)
        {
            Serial.println("Failed to open device.json for reading");
            return;
        }

        JsonDocument doc;
        deserializeJson(doc, static_cast<Stream &>(deviceFile));
        Device device = Device::fromJson(doc.as<JsonObject>());
        deviceFile.close();
    }
}

