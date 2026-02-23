#include "WifiConfig.h"

WifiConfig::WifiConfig(String wifiAccessPoint, String wifiPassKey, String wifiHostname)
{
    this->wifiAccessPoint = wifiAccessPoint;
    this->wifiPassKey = wifiPassKey;
    this->wifiHostname = wifiHostname;
}