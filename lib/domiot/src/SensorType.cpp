#include "SensorType.h"

// Define static members
const SensorType SensorType::NOT_USED(0, "NOT_USED");
const SensorType SensorType::POWER_PT1(1, "POWER_PT1");
const SensorType SensorType::POWER_PT2(2, "POWER_PT2");
const SensorType SensorType::POWER_CT1(3, "POWER_CT1");
const SensorType SensorType::POWER_CT2(4, "POWER_CT2");
const SensorType SensorType::POWER_AP(5, "POWER_AP");
const SensorType SensorType::POWER_AC(6, "POWER_AC");
const SensorType SensorType::GAS_METER(7, "GAS_METER");
const SensorType SensorType::TEMP(8, "TEMP");
const SensorType SensorType::HUMID(9, "HUMID");
const SensorType SensorType::WATER(10, "WATER_CONSUMED");
const SensorType SensorType::GAS_SENSOR(11, "GAS_SENSOR");
const SensorType SensorType::HYDRO(12, "HYDRATION");
const SensorType SensorType::STATUS(13, "STATUS");
const SensorType SensorType::VOLTAGE_LEVEL(14, "VOLTAGE_LEVEL");
const SensorType SensorType::CURRENT_LEVEL(15, "CURRENT_LEVEL");

std::optional<SensorType> SensorType::getType(String typeStr) {
    static const SensorType* allTypes[] = {
        &NOT_USED, &POWER_PT1, &POWER_PT2, &POWER_CT1, &POWER_CT2,
        &POWER_AP, &POWER_AC, &GAS_METER, &TEMP, &HUMID,
        &WATER, &GAS_SENSOR, &HYDRO, &STATUS, &VOLTAGE_LEVEL, &CURRENT_LEVEL
    };

    for (const SensorType* st : allTypes) {
        if (st->_description.compareTo(typeStr) == 0) {
            return *st;
        }
    }
    return std::nullopt;
}
