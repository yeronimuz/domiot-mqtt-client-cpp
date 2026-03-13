#ifndef ESP8266WIFI_H
#define ESP8266WIFI_H

#include <Arduino.h>

class WiFiClient {
};

class ESPClass {
public:
    uint32_t getChipId() const { return 0x123456; }
};

inline ESPClass ESP;

#endif // ESP8266WIFI_H
