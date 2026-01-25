#pragma once

#include <LittleFS.h>
#include "WifiConfig.h"
#include "MqttConfig.h"
#include <Device.h>
#include <ArduinoJson.h>


class DomiotConfig
{
    private:
        WifiConfig _wifiConfig;
        MqttConfig _mqttConfig;
        Device _device;
        // Intermediate json object with wifi and mqtt data
        JsonDocument _domiotJson;

        void getDomiotConfig();

    public:
        DomiotConfig();
        WifiConfig getWifiConfig() { return _wifiConfig; }
        MqttConfig getMqttConfig() { return _mqttConfig; }
        Device getDevice() { return _device; };

};