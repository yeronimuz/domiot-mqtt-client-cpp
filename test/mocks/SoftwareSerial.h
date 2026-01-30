#ifndef SOFTWARESERIAL_H
#define SOFTWARESERIAL_H

#include <Arduino.h>

// Mock SoftwareSerial for native platform testing
class SoftwareSerial {
public:
    SoftwareSerial(int rx, int tx) {}
    void begin(unsigned long baud) {}
    int available() { return 0; }
    int read() { return -1; }
    size_t readBytesUntil(char terminator, char* buffer, size_t length) { return 0; }
};

#endif // SOFTWARESERIAL_H
