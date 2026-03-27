#pragma once

#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

class MqttService;

class OTAService
{
public:
    using RestoreP1SerialCallback = void (*)();

    explicit OTAService(uint16_t port = 80);

    void setMqttService(MqttService *mqttService);
    void attachP1Serial(
        HardwareSerial *p1Serial,
        bool *p1SerialInitialized,
        RestoreP1SerialCallback restoreP1SerialCallback);
    void begin(const String &otaUsername, const String &otaPassword);
    void loop();

    bool isPrepared() const;
    bool isUploadActive() const;

private:
    static constexpr unsigned long PREPARE_TIMEOUT_MS = 30000;
    static constexpr uint16_t MQTT_BUFFER_SIZE_DURING_OTA = 256;

    AsyncWebServer server;
    MqttService *mqttService;
    HardwareSerial *p1Serial;
    bool *p1SerialInitialized;
    RestoreP1SerialCallback restoreP1SerialCallback;
    bool started;
    bool prepareActive;
    bool uploadActive;
    unsigned long prepareStartMillis;
    unsigned long progressMillis;
    uint16_t mqttBufferSizeBeforePrepare;

    void prepareForUpdate();
    void restoreAfterPrepare();
    void handleStart();
    void handleProgress(size_t current, size_t final);
    void handleEnd(bool success);
};