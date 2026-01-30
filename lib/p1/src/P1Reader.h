#pragma once

#include <SoftwareSerial.h>
#include "P1Datagram.h"

#define SERIAL_RX D5      // pin for SoftwareSerial RX
#define MAXLINELENGTH 128 // longest normal line is 47 char (+3 for \r\n\0)

class P1Reader
{
public:
    static P1Datagram readDatagram(SoftwareSerial &serial);
};
