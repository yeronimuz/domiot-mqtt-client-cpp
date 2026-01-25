#pragma once

#include <Arduino.h>

#define DEFAULT_PORT 1883

class MqttConfig
{
private:
    String _mqttBroker;
    u_short _mqttPort;
    String _mqttUser;
    String _mqttPassword;
    String _clientId;

public:
    MqttConfig() : _mqttBroker(""), _mqttPort(DEFAULT_PORT), _mqttUser(""), _mqttPassword(""), _clientId("") {}
    MqttConfig(String mqttBroker, u_short mqttPort, String mqttUser, String mqttPassword, String clientId)
    {
        this->_mqttBroker = mqttBroker;
        this->_mqttUser = mqttUser;
        this->_mqttPassword = mqttPassword;
        this->_mqttPort = mqttPort;
        this->_clientId = clientId;
    }

    String getMqttBroker() { return _mqttBroker; }
    String getMqttUser() { return _mqttUser; }
    String getMqttPassword() { return _mqttPassword; }
};