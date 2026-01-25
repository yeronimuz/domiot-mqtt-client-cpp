#include "TemperatureSensor.h"
#include "constants.h"
#include <Arduino.h>

float TemperatureSensor::readTemperature() {
    int adcValue = analogRead(THERMISTOR_PIN);
    float resistance = SERIES_RESISTOR / ((1023.0 / adcValue) - 1);

    float steinhart;
    steinhart = resistance / NOMINAL_RESISTANCE;  // (R/Ro)
    steinhart = log(steinhart);                  // ln(R/Ro)
    steinhart /= BETA_COEFFICIENT;               // 1/B * ln(R/Ro)
    steinhart += 1.0 / (NOMINAL_TEMPERATURE + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart;                 // Invert
    steinhart -= 273.15;                         // Convert to °C

    return steinhart;
}