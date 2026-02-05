#pragma once

#include <Arduino.h>
#include "P1Datagram.h"
#include <SensorValue.h>
#include <vector>
#include <Device.h>

class P1DatagramSensorValueMapper
{
public:
    static std::vector<SensorValue> mapToSensorValues(Device device, const P1Datagram& datagram);
private:
    static double getValueFromDatagram(Sensor sensor, P1Datagram datagram);
};