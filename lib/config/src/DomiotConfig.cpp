#include "DomiotConfig.h"
#include <ArduinoJson.h>
#include "Device.h"

DomiotConfig::DomiotConfig()
{
    Serial.println("DomiotConfig::DomiotConfig() starting");

    // Read WifiConfig and MqttConfig from LittleFS
    // Read DeviceConfig from LittleFS when exists
    if (!LittleFS.begin())
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    Serial.println("LittleFS mounted successfully");

    getDomiotConfig();
}

void DomiotConfig::getDomiotConfig()
{
    Serial.println("DomiotConfig::getDomiotConfig() called");

    bool hasConfig = LittleFS.exists("/config.json");
    Serial.printf("LittleFS /config.json exists: %s\n", hasConfig ? "yes" : "no");

    if (hasConfig)
    {
        File file = LittleFS.open("/config.json", "r");
        if (!file)
        {
            Serial.println("Failed to open /config.json for reading");
        }
        else
        {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, static_cast<Stream &>(file));
            file.close();

            if (error)
            {
                Serial.print("deserializeJson() failed for /config.json: ");
                Serial.println(error.c_str());
            }
            else
            {
                _domiotJson = doc;

                _wifiConfig = WifiConfig(
                    doc["wifi"]["ssid"] | "",
                    doc["wifi"]["password"] | "",
                    doc["wifi"]["hostname"] | "");

                _mqttConfig = MqttConfig(
                    doc["mqtt"]["host"] | "",
                    static_cast<u_short>(doc["mqtt"]["port"] | DEFAULT_PORT),
                    doc["mqtt"]["user"] | "",
                    doc["mqtt"]["password"] | "",
                    doc["mqtt"]["client_id"] | "");
            }
        }
    }
    else
    {
        Serial.println("No /config.json found in LittleFS.");
    }

    // Read device config from LittleFS when exists
    bool hasDeviceConfig = LittleFS.exists("/device.json");
    Serial.printf("LittleFS /device.json exists: %s\n", hasDeviceConfig ? "yes" : "no");

    if (hasDeviceConfig)
    {
        File deviceFile = LittleFS.open("/device.json", "r");
        Serial.println("Reading device configuration from LittleFS...");
        if (!deviceFile)
        {
            Serial.println("Failed to open /device.json for reading");
            return;
        }

        // Print the raw file payload to aid serial debugging.
        String deviceConfigContents = deviceFile.readString();
        Serial.println("Contents of /device.json:");
        Serial.println(deviceConfigContents);

        JsonDocument deviceDoc;
        DeserializationError deviceError = deserializeJson(deviceDoc, deviceConfigContents);
        if (deviceError)
        {
            Serial.print("deserializeJson() failed for /device.json: ");
            Serial.println(deviceError.c_str());
        }
        else
        {
            _device = Device::fromJson(deviceDoc.as<JsonObject>());
        }

        deviceFile.close();
    }
    else
    {
        Serial.println("No existing device configuration found in LittleFS.");
    }
}

