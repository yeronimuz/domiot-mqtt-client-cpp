#pragma once

#include <Arduino.h>

class WifiConfig
{
    private:
        String wifiAccessPoint;
        String wifiPassKey;
        String wifiHostname;

    public:
        WifiConfig() : wifiAccessPoint(""), wifiPassKey(""), wifiHostname("") {}
        WifiConfig(String wifiAccessPoint, String wifiPassKey, String wifiHostname = "");

        String getWifiAccessPoint() { return wifiAccessPoint;}
        String getWifiPassKey() { return wifiPassKey;}
        String getWifiHostname() { return wifiHostname; }
};