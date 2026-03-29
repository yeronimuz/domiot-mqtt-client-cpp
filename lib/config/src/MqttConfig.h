#pragma once

#include <Arduino.h>

#define DEFAULT_PORT 1883

class MqttConfig
{
private:
    String _mqttBroker;
    uint16_t _mqttPort;
    String _mqttUser;
    String _mqttPassword;
    String _clientId;

public:
    MqttConfig() : _mqttBroker(""), _mqttPort(DEFAULT_PORT), _mqttUser(""), _mqttPassword(""), _clientId("") {}
    MqttConfig(String mqttBroker, uint16_t mqttPort, String mqttUser, String mqttPassword, String clientId)
    {
        this->_mqttBroker = mqttBroker;
        this->_mqttUser = mqttUser;
        this->_mqttPassword = mqttPassword;
        this->_mqttPort = mqttPort;
        this->_clientId = clientId;
    }

    String getMqttBroker() { return _mqttBroker; }
    uint16_t getMqttPort() { return _mqttPort; }
    String getMqttUser() { return _mqttUser; }
    String getMqttPassword() { return _mqttPassword; }
    String getClientId() { return _clientId; }
};