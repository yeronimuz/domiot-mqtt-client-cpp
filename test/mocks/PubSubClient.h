#ifndef PUBSUBCLIENT_H
#define PUBSUBCLIENT_H

#include <Arduino.h>
#include <functional>

class WiFiClient;

using MqttCallback = std::function<void(char*, uint8_t*, unsigned int)>;

class PubSubClient {
public:
    static inline uint16_t bufferSize = 256;
    static inline bool connectedState = true;
    static inline bool connectReturnValue = true;
    static inline bool subscribeReturnValue = true;
    static inline bool publishReturnValue = true;
    static inline bool beginPublishReturnValue = true;
    static inline int endPublishReturnValue = 1;

    static inline bool connectCalled = false;
    static inline bool subscribeCalled = false;
    static inline bool publishCalled = false;
    static inline bool beginPublishCalled = false;
    static inline bool endPublishCalled = false;

    static inline String lastServer = "";
    static inline uint16_t lastServerPort = 0;
    static inline String lastConnectedClientId = "";
    static inline String lastConnectedUser = "";
    static inline String lastConnectedPassword = "";
    static inline String lastSubscribeTopic = "";
    static inline String lastPublishTopic = "";
    static inline String lastPublishPayload = "";
    static inline unsigned int lastPublishLength = 0;

    PubSubClient() = default;

    explicit PubSubClient(WiFiClient& client)
        : _client(&client)
    {
    }

    void setClient(WiFiClient& client)
    {
        _client = &client;
    }

    void setServer(const char* server, uint16_t port)
    {
        lastServer = server == nullptr ? "" : String(server);
        lastServerPort = port;
    }

    void setCallback(MqttCallback callback)
    {
        _callback = callback;
    }

    bool setBufferSize(uint16_t size)
    {
        bufferSize = size;
        return true;
    }

    uint16_t getBufferSize()
    {
        return bufferSize;
    }

    int state()
    {
        return connectedState ? 0 : -1;
    }

    bool connect(const char* clientId)
    {
        connectCalled = true;
        lastConnectedClientId = clientId == nullptr ? "" : String(clientId);
        lastConnectedUser = "";
        lastConnectedPassword = "";

        if (connectReturnValue)
        {
            connectedState = true;
        }

        return connectReturnValue;
    }

    bool connect(const char* clientId, const char* user, const char* password)
    {
        connectCalled = true;
        lastConnectedClientId = clientId == nullptr ? "" : String(clientId);
        lastConnectedUser = user == nullptr ? "" : String(user);
        lastConnectedPassword = password == nullptr ? "" : String(password);

        if (connectReturnValue)
        {
            connectedState = true;
        }

        return connectReturnValue;
    }

    bool connected()
    {
        return connectedState;
    }

    bool subscribe(const char* topic)
    {
        subscribeCalled = true;
        lastSubscribeTopic = topic == nullptr ? "" : String(topic);
        return subscribeReturnValue;
    }

    bool publish(const char* topic, const uint8_t* payload, unsigned int length)
    {
        publishCalled = true;
        lastPublishTopic = topic == nullptr ? "" : String(topic);
        lastPublishLength = length;

        lastPublishPayload = "";
        for (unsigned int i = 0; i < length; i++)
        {
            lastPublishPayload += static_cast<char>(payload[i]);
        }

        return publishReturnValue;
    }

    bool beginPublish(const char* topic, unsigned int plength, bool retained)
    {
        (void)retained;
        beginPublishCalled = true;
        publishCalled = true;
        lastPublishTopic = topic == nullptr ? "" : String(topic);
        lastPublishLength = plength;
        lastPublishPayload = "";
        return beginPublishReturnValue;
    }

    int endPublish()
    {
        endPublishCalled = true;
        return endPublishReturnValue;
    }

    size_t write(uint8_t value)
    {
        lastPublishPayload += static_cast<char>(value);
        return 1;
    }

    size_t write(const uint8_t* buffer, size_t size)
    {
        for (size_t i = 0; i < size; i++)
        {
            lastPublishPayload += static_cast<char>(buffer[i]);
        }
        return size;
    }

    bool loop()
    {
        return true;
    }

    static void resetMockState()
    {
        bufferSize = 256;
        connectedState = true;
        connectReturnValue = true;
        subscribeReturnValue = true;
        publishReturnValue = true;
        beginPublishReturnValue = true;
        endPublishReturnValue = 1;

        connectCalled = false;
        subscribeCalled = false;
        publishCalled = false;
        beginPublishCalled = false;
        endPublishCalled = false;

        lastServer = "";
        lastServerPort = 0;
        lastConnectedClientId = "";
        lastConnectedUser = "";
        lastConnectedPassword = "";
        lastSubscribeTopic = "";
        lastPublishTopic = "";
        lastPublishPayload = "";
        lastPublishLength = 0;
    }

private:
    WiFiClient* _client = nullptr;
    MqttCallback _callback;
};

#endif // PUBSUBCLIENT_H
