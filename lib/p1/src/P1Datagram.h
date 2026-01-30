#pragma once
#include <Arduino.h>

#define MAX_P1_ITEMS 24

class P1Datagram {
public:
    byte versionInfo;
    String timestamp;
    String equipmentId;
    double consumedPowerT1;
    double consumedPowerT2;
    double producedPowerT1;
    double producedPowerT2;
    byte currentTariff;
    double actualPowerConsumed;
    double actualPowerProduced;
    byte powerFailsAnyPhase;
    byte longPowerFailsAnyPhase;
    String powerFailEventLog;
    uint16_t voltageSagsInPhaseL1;
    uint16_t voltageSagsInPhaseL2;
    uint16_t textMessageCodes;
    String textMessages;
    double instantaneousCurrentL1;
    double instantaneousActivePowerL1PlusP;
    double instantaneousActivePowerL1MinP;
    byte deviceType;
    String key;
    double consumedGas;

    P1Datagram() = default;
    String* getAsPayload();
};