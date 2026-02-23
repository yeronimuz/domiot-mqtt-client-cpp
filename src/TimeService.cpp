#include "TimeService.h"
#include <time.h>

TimeService::TimeService(unsigned long retryIntervalMs)
    : utcTimeSynced(false),
      lastTimeSyncAttempt(0),
      retryIntervalMs(retryIntervalMs)
{
}

bool TimeService::isTimeSynced() const
{
    time_t now = time(nullptr);
    return now >= 1700000000;
}

void TimeService::syncUtcTime()
{
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    unsigned long start = millis();
    while (!isTimeSynced() && (millis() - start) < 10000)
    {
        delay(200);
    }
}

bool TimeService::ensureUtcTimeSynced()
{
    if (utcTimeSynced)
    {
        return true;
    }

    unsigned long now = millis();
    if (now - lastTimeSyncAttempt < retryIntervalMs)
    {
        return false;
    }

    lastTimeSyncAttempt = now;
    syncUtcTime();
    utcTimeSynced = isTimeSynced();
    if (utcTimeSynced)
    {
        Serial.println("UTC time synchronized.");
    }
    else
    {
        Serial.println("UTC time not synchronized yet.");
    }
    return utcTimeSynced;
}

String TimeService::getUtcTimestamp() const
{
    // Format timestamp as ISO 8601 UTC: "2025-03-17T21:55:59.524Z"
    time_t now = time(nullptr);
    struct tm *timeinfo = gmtime(&now);
    char timestamp_buffer[32];
    strftime(timestamp_buffer, sizeof(timestamp_buffer), "%Y-%m-%dT%H:%M:%S", timeinfo);
    unsigned long milliseconds = millis() % 1000;
    char ms_buffer[4];
    sprintf(ms_buffer, "%03lu", milliseconds);
    return String(timestamp_buffer) + "." + String(ms_buffer) + "Z";
}
