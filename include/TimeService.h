#pragma once

#include <Arduino.h>

class TimeService
{
public:
    explicit TimeService(unsigned long retryIntervalMs = 60000);

    void syncUtcTime();
    bool ensureUtcTimeSynced();
    bool isTimeSynced() const;
    String getUtcTimestamp() const;

private:
    bool utcTimeSynced;
    unsigned long lastTimeSyncAttempt;
    unsigned long retryIntervalMs;
};
