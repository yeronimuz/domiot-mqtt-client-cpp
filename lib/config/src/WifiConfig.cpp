#include "WifiConfig.h"

WifiConfig::WifiConfig(String wifiAccessPoint, String wifiPassKey)
{
    this->wifiAccessPoint = wifiAccessPoint;
    this->wifiPassKey = wifiPassKey;
}