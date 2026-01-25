#pragma once

// Battery level constants
#define BATTERY_PIN A0

class BatteryLevel {
    public:
        static float readBatteryLevel();
};
