#include <P1Datagram.h>

String* P1Datagram::getAsPayload()
{
    static String payloadParts[MAX_P1_ITEMS];
    int index = 0;

    payloadParts[index++] = "\"versionInfo\": " + String(versionInfo);
    payloadParts[index++] = "\"timestamp\": \"" + timestamp + "\"";
    payloadParts[index++] = "\"equipmentId\": \"" + equipmentId + "\"";
    payloadParts[index++] = "\"consumedPowerT1\": " + String(consumedPowerT1, 2);
    payloadParts[index++] = "\"consumedPowerT2\": " + String(consumedPowerT2, 2);
    payloadParts[index++] = "\"producedPowerT1\": " + String(producedPowerT1, 2);
    payloadParts[index++] = "\"producedPowerT2\": " + String(producedPowerT2, 2);
    payloadParts[index++] = "\"currentTariff\": " + String(currentTariff);
    payloadParts[index++] = "\"actualPowerConsumed\": " + String(actualPowerConsumed, 2);
    payloadParts[index++] = "\"actualPowerProduced\": " + String(actualPowerProduced, 2);
    payloadParts[index++] = "\"powerFailsAnyPhase\": " + String(powerFailsAnyPhase);
    payloadParts[index++] = "\"longPowerFailsAnyPhase\": " + String(longPowerFailsAnyPhase);
    payloadParts[index++] = "\"powerFailEventLog\": \"" + powerFailEventLog + "\"";
    payloadParts[index++] = "\"voltageSagsInPhaseL1\": " + String(voltageSagsInPhaseL1);
    payloadParts[index++] = "\"voltageSagsInPhaseL2\": " + String(voltageSagsInPhaseL2);
    payloadParts[index++] = "\"textMessageCodes\": " + String(textMessageCodes);
    payloadParts[index++] = "\"textMessages\": \"" + textMessages + "\"";
    payloadParts[index++] = "\"instantaneousCurrentL1\": " + String(instantaneousCurrentL1, 2);
    payloadParts[index++] = "\"instantaneousActivePowerL1PlusP\": " + String(instantaneousActivePowerL1PlusP, 2);
    payloadParts[index++] = "\"instantaneousActivePowerL1MinP\": " + String(instantaneousActivePowerL1MinP, 2);
    payloadParts[index++] = "\"deviceType\": " + String(deviceType);
    payloadParts[index++] = "\"key\": \"" + key + "\"";
    payloadParts[index++] = "\"consumedGas\": " + String(consumedGas, 2);
    payloadParts[index++] = "\0";

    return payloadParts;
}