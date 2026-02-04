#pragma once
#include <Arduino.h>

#define MAX_P1_ITEMS 24

class P1Datagram {
private:
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

public:
    P1Datagram() = default;
    
    // Getters
    byte getVersionInfo() const { return versionInfo; }
    String getTimestamp() const { return timestamp; }
    String getEquipmentId() const { return equipmentId; }
    double getConsumedPowerT1() const { return consumedPowerT1; }
    double getConsumedPowerT2() const { return consumedPowerT2; }
    double getProducedPowerT1() const { return producedPowerT1; }
    double getProducedPowerT2() const { return producedPowerT2; }
    byte getCurrentTariff() const { return currentTariff; }
    double getActualPowerConsumed() const { return actualPowerConsumed; }
    double getActualPowerProduced() const { return actualPowerProduced; }
    byte getPowerFailsAnyPhase() const { return powerFailsAnyPhase; }
    byte getLongPowerFailsAnyPhase() const { return longPowerFailsAnyPhase; }
    String getPowerFailEventLog() const { return powerFailEventLog; }
    uint16_t getVoltageSagsInPhaseL1() const { return voltageSagsInPhaseL1; }
    uint16_t getVoltageSagsInPhaseL2() const { return voltageSagsInPhaseL2; }
    uint16_t getTextMessageCodes() const { return textMessageCodes; }
    String getTextMessages() const { return textMessages; }
    double getInstantaneousCurrentL1() const { return instantaneousCurrentL1; }
    double getInstantaneousActivePowerL1PlusP() const { return instantaneousActivePowerL1PlusP; }
    double getInstantaneousActivePowerL1MinP() const { return instantaneousActivePowerL1MinP; }
    byte getDeviceType() const { return deviceType; }
    String getKey() const { return key; }
    double getConsumedGas() const { return consumedGas; }
    
    // Setters
    void setVersionInfo(byte value) { versionInfo = value; }
    void setTimestamp(const String& value) { timestamp = value; }
    void setEquipmentId(const String& value) { equipmentId = value; }
    void setConsumedPowerT1(double value) { consumedPowerT1 = value; }
    void setConsumedPowerT2(double value) { consumedPowerT2 = value; }
    void setProducedPowerT1(double value) { producedPowerT1 = value; }
    void setProducedPowerT2(double value) { producedPowerT2 = value; }
    void setCurrentTariff(byte value) { currentTariff = value; }
    void setActualPowerConsumed(double value) { actualPowerConsumed = value; }
    void setActualPowerProduced(double value) { actualPowerProduced = value; }
    void setPowerFailsAnyPhase(byte value) { powerFailsAnyPhase = value; }
    void setLongPowerFailsAnyPhase(byte value) { longPowerFailsAnyPhase = value; }
    void setPowerFailEventLog(const String& value) { powerFailEventLog = value; }
    void setVoltageSagsInPhaseL1(uint16_t value) { voltageSagsInPhaseL1 = value; }
    void setVoltageSagsInPhaseL2(uint16_t value) { voltageSagsInPhaseL2 = value; }
    void setTextMessageCodes(uint16_t value) { textMessageCodes = value; }
    void setTextMessages(const String& value) { textMessages = value; }
    void setInstantaneousCurrentL1(double value) { instantaneousCurrentL1 = value; }
    void setInstantaneousActivePowerL1PlusP(double value) { instantaneousActivePowerL1PlusP = value; }
    void setInstantaneousActivePowerL1MinP(double value) { instantaneousActivePowerL1MinP = value; }
    void setDeviceType(byte value) { deviceType = value; }
    void setKey(const String& value) { key = value; }
    void setConsumedGas(double value) { consumedGas = value; }
    
    String* getAsPayload();
};