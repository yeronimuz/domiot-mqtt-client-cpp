#pragma once

#include <Arduino.h>
#include "P1Datagram.h"
#include <SensorValue.h>
#include <vector>
#include <Device.h>

class P1DatagramSensorValueMapper
{
public:
    static std::vector<SensorValue> mapToSensorValues(const Device& device, const P1Datagram& datagram);
private:
    static double getValueFromDatagram(const Sensor& sensor, const P1Datagram& datagram);
};