#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>         // Use <WiFi.h> for ESP32
#include <PubSubClient.h>
#include <Device.h>
#include <LittleFS.h>

class MqttService {
    private:    
        String _mqttServer;
        String _mqttUser;
        String _mqttPassword;
        WiFiClient* _wifiClient;
        PubSubClient _mqttClient;

        Device _device;
    public:
        MqttService(String server = "", String user = "", String password = "", WiFiClient* wifiClient = nullptr) :
            _mqttServer(server),
            _mqttUser(user),
            _mqttPassword(password),
            _wifiClient(wifiClient),
            _mqttClient(*wifiClient)
        {
            _mqttClient.setServer(_mqttServer.c_str(), 1883);
            _mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
                this->callback(topic, payload, length);
            });
        }

        void connect();
        boolean isConnected();
        void registerDevice(Device device);
        void callback(char *topic, byte *payload, unsigned int length);
        Device& getDevice() { return _device; }
        PubSubClient &getClient() { return _mqttClient; }
};
