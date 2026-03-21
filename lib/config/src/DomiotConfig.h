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
        String _otaUsername;
        String _otaPassword;

        void getDomiotConfig();

    public:
        DomiotConfig();
        WifiConfig getWifiConfig() { return _wifiConfig; }
        MqttConfig getMqttConfig() { return _mqttConfig; }
        String getOtaUsername() { return _otaUsername; }
        String getOtaPassword() { return _otaPassword; }
        Device getDevice() { return _device; };

};