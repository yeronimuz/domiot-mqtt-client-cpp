#include "P1DatagramSensorValueMapper.h"
#include <P1Standard.h>
#include <SensorValue.h>
#include <Device.h>

namespace
{
bool isP1SensorType(const SensorType &sensorType)
{
    int typeId = sensorType.getId();
    return typeId == SensorType::POWER_PT1.getId() ||
           typeId == SensorType::POWER_PT2.getId() ||
           typeId == SensorType::GAS_METER.getId() ||
           typeId == SensorType::POWER_AP.getId() ||
           typeId == SensorType::POWER_AC.getId() ||
           typeId == SensorType::POWER_CT1.getId() ||
           typeId == SensorType::POWER_CT2.getId();
}

bool isLeapYear(int year)
{
    if (year % 400 == 0)
    {
        return true;
    }
    if (year % 100 == 0)
    {
        return false;
    }
    return (year % 4 == 0);
}

int daysInMonth(int year, int month)
{
    static const int daysPerMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year))
    {
        return 29;
    }
    return daysPerMonth[month - 1];
}

String toUtcTimestamp(const String &p1Timestamp)
{
    if (p1Timestamp.length() < 13)
    {
        return p1Timestamp;
    }

    char dstFlag = p1Timestamp.charAt(p1Timestamp.length() - 1);
    int offsetHours = 0;
    if (dstFlag == 'W')
    {
        offsetHours = 1;
    }
    else if (dstFlag == 'S')
    {
        offsetHours = 2;
    }
    else
    {
        return p1Timestamp;
    }

    String digits = p1Timestamp.substring(0, p1Timestamp.length() - 1);
    if (digits.length() < 12)
    {
        return p1Timestamp;
    }

    int year = 2000 + digits.substring(0, 2).toInt();
    int month = digits.substring(2, 4).toInt();
    int day = digits.substring(4, 6).toInt();
    int hour = digits.substring(6, 8).toInt();
    int minute = digits.substring(8, 10).toInt();
    int second = digits.substring(10, 12).toInt();

    // Basic range validation to guard against malformed input and
    // prevent out-of-bounds access in daysInMonth().
    if (month < 1 || month > 12 ||
        day < 1 || day > 31 ||
        hour < 0 || hour > 23 ||
        minute < 0 || minute > 59 ||
        second < 0 || second > 59)
    {
        return p1Timestamp;
    }
    hour -= offsetHours;
    while (hour < 0)
    {
        hour += 24;
        day -= 1;
        if (day < 1)
        {
            month -= 1;
            if (month < 1)
            {
                month = 12;
                year -= 1;
            }
            day = daysInMonth(year, month);
        }
    }

    char buffer[25];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minute, second);
    return String(buffer);
}

} // namespace

std::vector<SensorValue> P1DatagramSensorValueMapper::mapToSensorValues(Device device, const P1Datagram &datagram)
{
    std::vector<SensorValue> sensorValues;

    for (const Sensor &sensor : device.getSensors())
    {
        if (!isP1SensorType(sensor.getType()) || sensor.getSensorId() <= 0)
        {
            continue;
        }

        SensorValue sv;
        sv.setSensorId(sensor.getSensorId());
        sv.setTimestamp(toUtcTimestamp(datagram.getTimestamp()));

        double value = getValueFromDatagram(sensor, datagram);
        sv.setValue(value);
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
            return 0.0; // Unsupported type, filtered by mapToSensorValues
        }
    }
    return 0.0; // Type not used
}