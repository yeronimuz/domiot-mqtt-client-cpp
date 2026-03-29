#include "MqttService.h"
#include <ArduinoJson.h>
#include <cstring>

namespace {
constexpr const char* REGISTER_TOPIC = "register";
constexpr const char* CONFIG_TOPIC = "config";
constexpr const char* CONFIG_TOPIC_FILTER = "config/#";
#if defined(ESP8266)
constexpr uint16_t CONFIG_PACKET_BUFFER_SIZE = 2048;
constexpr uint16_t CONFIG_PACKET_BUFFER_FALLBACK_SIZE = 1536;
constexpr uint16_t CONFIG_PACKET_BUFFER_MIN_SIZE = 1024;
constexpr uint32_t MQTT_RUNTIME_HEAP_RESERVE_BYTES = 7000;
#else
constexpr uint16_t CONFIG_PACKET_BUFFER_SIZE = 2048;
#endif

bool isConfigTopic(const char* topic)
{
    if (topic == nullptr)
    {
        return false;
    }

    const size_t configTopicLength = strlen(CONFIG_TOPIC);
    if (strcmp(topic, CONFIG_TOPIC) == 0)
    {
        return true;
    }

    return strncmp(topic, CONFIG_TOPIC, configTopicLength) == 0 && topic[configTopicLength] == '/';
}

void ensureConfigPacketBuffer(PubSubClient& mqttClient)
{
    if (mqttClient.getBufferSize() >= CONFIG_PACKET_BUFFER_SIZE)
    {
        return;
    }

#if defined(ESP8266)
    const uint16_t targets[] = {
        CONFIG_PACKET_BUFFER_SIZE,
        CONFIG_PACKET_BUFFER_FALLBACK_SIZE,
        CONFIG_PACKET_BUFFER_MIN_SIZE,
    };

    for (uint16_t target : targets)
    {
        if (mqttClient.getBufferSize() >= target)
        {
            return;
        }

        const uint32_t freeHeapBefore = ESP.getFreeHeap();
        if (freeHeapBefore <= (target + MQTT_RUNTIME_HEAP_RESERVE_BYTES))
        {
            Serial.printf("Skipping MQTT buffer resize to %u (free heap=%lu, reserve=%lu).\r\n",
                          target,
                          static_cast<unsigned long>(freeHeapBefore),
                          static_cast<unsigned long>(MQTT_RUNTIME_HEAP_RESERVE_BYTES));
            continue;
        }

        if (mqttClient.setBufferSize(target))
        {
            Serial.printf("MQTT buffer size set to %u (free heap=%lu).\r\n",
                          target,
                          static_cast<unsigned long>(freeHeapBefore));
            return;
        }

        Serial.printf("Failed to increase MQTT packet buffer size to %u (free heap=%lu).\r\n",
                      target,
                      static_cast<unsigned long>(freeHeapBefore));
    }

    Serial.printf("Continuing with MQTT buffer size %u.\r\n", mqttClient.getBufferSize());
#else
    if (!mqttClient.setBufferSize(CONFIG_PACKET_BUFFER_SIZE))
    {
        Serial.println("Failed to increase MQTT packet buffer size.");
    }
#endif
}
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
    if (!isConnected())
    {
        connect();
    }

    // Resize before building JSON payload to maximize available heap during realloc.
    ensureConfigPacketBuffer(_mqttClient);

    bool subscribed = _mqttClient.subscribe(CONFIG_TOPIC_FILTER);
    if (!subscribed)
    {
        // Fallback for brokers/ACLs that only permit exact topic subscriptions.
        subscribed = _mqttClient.subscribe(CONFIG_TOPIC);
    }
    if (!subscribed)
    {
        Serial.println("Subscription to config topic failed before registration publish.");
    }

    JsonDocument doc;
    Device::toJson(device, doc);

    const size_t payloadLength = measureJson(doc);
    const size_t estimatedConfigPayloadLength = payloadLength;
    if (_mqttClient.getBufferSize() < estimatedConfigPayloadLength)
    {
        Serial.print("Warning: MQTT buffer likely too small for config response. Buffer=");
        Serial.print(_mqttClient.getBufferSize());
        Serial.print(", estimated config payload=");
        Serial.println(static_cast<unsigned long>(estimatedConfigPayloadLength));
    }

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
    if (!isConfigTopic(topic))
    {
        return;
    }

    Serial.print("Config message received on topic: ");
    Serial.println(topic);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error)
    {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Write to file FIRST, before creating device object.
    Device::writeDeviceJson(doc);

    // Now create device from JSON.
    _device = Device::fromJson(doc.as<JsonObject>());

}