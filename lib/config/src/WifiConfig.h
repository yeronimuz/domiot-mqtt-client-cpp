#pragma once

#include <Arduino.h>

class WifiConfig
{
    private:
        String wifiAccessPoint;
        String wifiPassKey;

    public:
        WifiConfig() : wifiAccessPoint(""), wifiPassKey("") {}
        WifiConfig(String wifiAccessPoint, String wifiPassKey);

        String getWifiAccessPoint() { return wifiAccessPoint;}
        String getWifiPassKey() { return wifiPassKey;}
};