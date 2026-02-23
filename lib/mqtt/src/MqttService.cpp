#include "MqttService.h"
#include <ArduinoJson.h>

void MqttService::connect()
{
    while (!isConnected())
    {
        Serial.print("Connecting to MQTT... ");
        String effectiveClientId = _clientId;
        if (effectiveClientId.length() == 0)
        {
            effectiveClientId = "domiot-" + String(ESP.getChipId(), HEX);
        }

        if (_mqttUser.length() > 0 && _mqttPassword.length() > 0 && _mqttServer.length() > 0)
        {
            Serial.printf("Using MQTT config %s, %s, %s\n", _mqttServer.c_str(), _mqttUser.c_str(), effectiveClientId.c_str());
            if (_mqttClient.connect(effectiveClientId.c_str(), _mqttUser.c_str(), _mqttPassword.c_str()))
            {
                Serial.println("Connected!");
            }
            else
            {
                Serial.println("Failed, retrying in 5s...");
            }
        }
        else
        {
            Serial.println("No MQTT credentials provided, trying anonymous connection...");
            if (_mqttClient.connect(effectiveClientId.c_str()))
            {
                Serial.println("Connected!");
            }
            else
            {
                Serial.println("Failed, retrying in 5s...");
            }
        }
        delay(5000);
    }
}

boolean MqttService::isConnected()
{
    return _mqttClient.connected();
}

void MqttService::registerDevice(Device device)
{
    JsonDocument doc;
    boolean isRegistered = false;

    if (!isConnected())
    {
        connect();
    }
    _mqttClient.subscribe("config");

    String payload;
    serializeJson(doc, payload);
    isRegistered = _mqttClient.publish("register", (byte *)payload.c_str(), payload.length());
    Serial.print("Device registration ");
    Serial.println(isRegistered ? "succeeded" : "failed");
}

/* MqTT callback
 * will be called when a message arrives on the topic that this device is subscribed to
 * A device may receive a deviceConfig including a deviceId or operational parameters (in config message)
 */
void MqttService::callback(char *topic, byte *payload, unsigned int length)
{
    if (strcmp(topic, "config") == 0)
    {
        // Handle config message
        Serial.println("Config message received");
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload, length);
        if (error)
        {
            Serial.print("JSON parse failed: ");
            Serial.println(error.c_str());
            return;
        }
        
        // Write to file FIRST, before creating device object
        Device::writeDeviceJson(doc);
        
        // Now create device from JSON
        _device = Device::fromJson(doc.as<JsonObject>());
        Serial.printf("Device ID:  %ld\n", _device.getDeviceId());
    }

}