#pragma once
#include <Arduino.h>
#include <regex>
#include "P1Datagram.h"
#include "P1Standard.h"

class P1Parser {
private:    
    P1Parser() = default;
    
public:
    static P1Datagram parse(const String& p1Message);
};