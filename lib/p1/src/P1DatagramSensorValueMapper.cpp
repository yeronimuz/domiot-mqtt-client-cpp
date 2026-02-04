#include "P1DatagramSensorValueMapper.h"
#include <P1Standard.h>
#include <SensorValue.h>
#include <Device.h>

std::vector<SensorValue> P1DatagramSensorValueMapper::mapToSensorValue(Device device, const P1Datagram &datagram)
{
    std::vector<SensorValue> sensorValues;

    for (const Sensor &sensor : device._sensors)
    {
        SensorValue sv;
        sv.sensorId = sensor._sensorId;
        sv.timestamp = datagram.getTimestamp();

        sv.value = getValueFromDatagram(sensor, datagram);
        sensorValues.push_back(sv);
    }
    // Add more mappings as needed...

    return sensorValues;
}

double P1DatagramSensorValueMapper::getValueFromDatagram(Sensor sensor, P1Datagram datagram)
{
    if (sensor._type != SensorType::NOT_USED)
    {
        int typeId = sensor._type.getId();
        if (typeId == SensorType::POWER_PT1.getId())
        {
            return datagram.getConsumedPowerT1();
        }
        else if (typeId == SensorType::POWER_PT2.getId())
        {
            return datagram.getConsumedPowerT2();
        }
        else if (typeId == SensorType::GAS_METER.getId())
        {
            return datagram.getConsumedGas();
        }
        else if (typeId == SensorType::POWER_AP.getId())
        {
            return datagram.getActualPowerProduced();
        }
        else if (typeId == SensorType::POWER_AC.getId())
        {
            return datagram.getActualPowerConsumed();
        }
        else if (typeId == SensorType::POWER_CT1.getId())
        {
            return datagram.getConsumedPowerT1();
        }
        else if (typeId == SensorType::POWER_CT2.getId())
        {
            return datagram.getConsumedPowerT2();
        }
        // Add more cases as needed...
        else
        {
            return 0.0; // Unsupported type
        }
    }
    return 0.0; // Type not found
}