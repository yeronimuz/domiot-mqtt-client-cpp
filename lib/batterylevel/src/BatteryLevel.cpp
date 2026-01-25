#include "BatteryLevel.h"
#include "constants.h"
#include <Arduino.h>

float BatteryLevel::readBatteryLevel() {
    int adcValue = analogRead(BATTERY_PIN);
    float voltage = (adcValue / 1023.0) * 5.0;  // Assuming a 5V reference
    float batteryPercentage = (voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE) * 100.0;

    return constrain(batteryPercentage, 0, 100);
}