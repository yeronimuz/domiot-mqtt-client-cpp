#include "P1DatagramSensorValueMapper.h"
#include <P1Standard.h>
#include <SensorValue.h>
#include <Device.h>

std::vector<SensorValue> P1DatagramSensorValueMapper::mapToSensorValues(Device device, const P1Datagram &datagram)
{
    std::vector<SensorValue> sensorValues;

    for (const Sensor &sensor : device.getSensors())
    {
        SensorValue sv;
        sv.setSensorId(sensor.getSensorId());
        sv.setTimestamp(datagram.getTimestamp());

        sv.setValue(getValueFromDatagram(sensor, datagram));
        sensorValues.push_back(sv);
    }

    return sensorValues;
}

double P1DatagramSensorValueMapper::getValueFromDatagram(Sensor sensor, P1Datagram datagram)
{
    if (sensor.getType() != SensorType::NOT_USED)
    {
        int typeId = sensor.getType().getId();
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
    return 0.0; // Type not used
}