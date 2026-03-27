#pragma once

#include <Arduino.h>
#include "P1Datagram.h"

#define MAXLINELENGTH 256 // keep enough room for long optional DSMR lines

class P1Reader
{
public:
    static P1Datagram readDatagram(Stream &serial);
};
