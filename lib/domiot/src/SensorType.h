#pragma once

#include <Arduino.h>
#include <optional>

class SensorType {
public:
    static const SensorType NOT_USED;
    static const SensorType POWER_PT1;
    static const SensorType POWER_PT2;
    static const SensorType POWER_CT1;
    static const SensorType POWER_CT2;
    static const SensorType POWER_AP;
    static const SensorType POWER_AC;
    static const SensorType GAS_METER;
    static const SensorType TEMP;
    static const SensorType HUMID;
    static const SensorType WATER;
    static const SensorType GAS_SENSOR;
    static const SensorType HYDRO;
    static const SensorType STATUS;
    static const SensorType VOLTAGE_LEVEL;
    static const SensorType CURRENT_LEVEL;

    int getId() const { return _id; }
    String getDescription() const { return _description; }

    /**
     * Get type by Id.
     *
     * @param type The type id to lookup.
     * @return Optional containing the SensorType that matches the id, or empty if not found
     */
    static std::optional<SensorType> getType(String typeStr);

    // Comparison operators
    bool operator==(const SensorType& other) const { return _id == other._id; }
    bool operator!=(const SensorType& other) const { return _id != other._id; }

private:
    int _id;
    String _description;

    SensorType(int id, const String& desc) : _id(id), _description(desc) {}
};
