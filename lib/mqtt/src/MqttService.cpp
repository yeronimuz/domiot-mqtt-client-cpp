#include "MqttService.h"
#include <ArduinoJson.h>

namespace {
constexpr const char* REGISTER_TOPIC = "register";
constexpr const char* CONFIG_TOPIC = "config";
}

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
            Serial.print("Using MQTT config ");
            Serial.print(_mqttServer);
            Serial.print(", ");
            Serial.print(_mqttUser);
            Serial.print(", ");
            Serial.println(effectiveClientId);
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

void MqttService::registerDevice(const Device& device)
{
    JsonDocument doc;
    Device::toJson(device, doc);

    if (!isConnected())
    {
        connect();
    }
    if (!_mqttClient.subscribe(CONFIG_TOPIC))
    {
        Serial.println("Subscription to config topic failed before registration publish.");
    }

    const size_t payloadLength = measureJson(doc);
    const bool beginOk = _mqttClient.beginPublish(REGISTER_TOPIC, payloadLength, false);
    if (!beginOk)
    {
        Serial.println("Failed to begin MQTT registration publish.");
        Serial.print("MQTT state: ");
        Serial.println(_mqttClient.state());
        Serial.print("MQTT buffer size: ");
        Serial.println(_mqttClient.getBufferSize());
        Serial.print("Payload length: ");
        Serial.println(static_cast<unsigned long>(payloadLength));
        return;
    }

    const size_t writtenLength = serializeJson(doc, _mqttClient);
    const bool endOk = (_mqttClient.endPublish() == 1);
    const bool isRegistered = (writtenLength == payloadLength) && endOk;

    Serial.print("Device registration ");
    Serial.println(isRegistered ? "succeeded" : "failed");

    if (!isRegistered)
    {
        Serial.print("Written length: ");
        Serial.println(static_cast<unsigned long>(writtenLength));
        Serial.print("MQTT state: ");
        Serial.println(_mqttClient.state());
        Serial.print("MQTT buffer size: ");
        Serial.println(_mqttClient.getBufferSize());
        Serial.print("Payload length: ");
        Serial.println(static_cast<unsigned long>(payloadLength));
    }
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
    }

}