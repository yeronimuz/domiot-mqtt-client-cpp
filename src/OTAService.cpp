#include "OTAService.h"

#include <ESP8266WiFi.h>
#include <MqttService.h>

OTAService::OTAService(uint16_t port)
    : server(port),
      mqttService(nullptr),
      p1Serial(nullptr),
      p1SerialInitialized(nullptr),
      restoreP1SerialCallback(nullptr),
      started(false),
      prepareActive(false),
      uploadActive(false),
      prepareStartMillis(0),
      progressMillis(0),
      mqttBufferSizeBeforePrepare(0)
{
}

void OTAService::setMqttService(MqttService *service)
{
    mqttService = service;
}

void OTAService::attachP1Serial(
    HardwareSerial *serial,
    bool *serialInitialized,
    RestoreP1SerialCallback restoreSerialCallback)
{
    p1Serial = serial;
    p1SerialInitialized = serialInitialized;
    restoreP1SerialCallback = restoreSerialCallback;
}

void OTAService::begin(const String &otaUsername, const String &otaPassword)
{
    if (started)
    {
        return;
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", "Domiot Sensor update facility on /update."); });

    server.on("/ota/prepare", HTTP_GET, [this](AsyncWebServerRequest *request)
    {
        prepareForUpdate();
        Serial.printf("OTA prepare: MQTT freed (free heap=%lu, max block=%lu)\r\n",
                      static_cast<unsigned long>(ESP.getFreeHeap()),
                      static_cast<unsigned long>(ESP.getMaxFreeBlockSize()));
        request->send(200, "text/plain", "OK");
    });

    if (otaUsername.length() > 0 && otaPassword.length() > 0)
    {
        ElegantOTA.begin(&server, otaUsername.c_str(), otaPassword.c_str());
        Serial.println("ElegantOTA authentication enabled.");
    }
    else
    {
        ElegantOTA.begin(&server);
        Serial.println("ElegantOTA authentication disabled.");
    }

    ElegantOTA.onStart([this]() { handleStart(); });
    ElegantOTA.onProgress([this](size_t current, size_t final) { handleProgress(current, final); });
    ElegantOTA.onEnd([this](bool success) { handleEnd(success); });

    server.begin();
    started = true;

    Serial.printf("HTTP server started on port 80 (free heap=%lu, max block=%lu)\r\n",
                  static_cast<unsigned long>(ESP.getFreeHeap()),
                  static_cast<unsigned long>(ESP.getMaxFreeBlockSize()));
}

void OTAService::loop()
{
    if (!started)
    {
        return;
    }

    ElegantOTA.loop();

    if (prepareActive && !uploadActive && prepareStartMillis != 0 &&
        millis() - prepareStartMillis > PREPARE_TIMEOUT_MS)
    {
        Serial.println("OTA prepare timed out, restoring runtime services.");
        restoreAfterPrepare();
    }
}

bool OTAService::isPrepared() const
{
    return prepareActive;
}

bool OTAService::isUploadActive() const
{
    return uploadActive;
}

void OTAService::prepareForUpdate()
{
    prepareActive = true;
    uploadActive = false;
    prepareStartMillis = millis();

    if (mqttService != nullptr)
    {
        PubSubClient &mqttClient = mqttService->getClient();
        mqttBufferSizeBeforePrepare = mqttClient.getBufferSize();
        mqttClient.disconnect();
        mqttClient.setBufferSize(MQTT_BUFFER_SIZE_DURING_OTA);
    }

    if (p1Serial != nullptr && p1SerialInitialized != nullptr && *p1SerialInitialized)
    {
        p1Serial->end();
        *p1SerialInitialized = false;
    }
}

void OTAService::restoreAfterPrepare()
{
    prepareActive = false;
    uploadActive = false;
    prepareStartMillis = 0;

    if (mqttService != nullptr)
    {
        PubSubClient &mqttClient = mqttService->getClient();
        if (mqttBufferSizeBeforePrepare != 0 && mqttClient.getBufferSize() != mqttBufferSizeBeforePrepare)
        {
            mqttClient.setBufferSize(mqttBufferSizeBeforePrepare);
        }
    }
    mqttBufferSizeBeforePrepare = 0;

    if (p1SerialInitialized != nullptr && !*p1SerialInitialized && restoreP1SerialCallback != nullptr)
    {
        restoreP1SerialCallback();
    }
}

void OTAService::handleStart()
{
    prepareActive = true;
    uploadActive = true;
    prepareStartMillis = millis();
    progressMillis = 0;

    Serial.printf("OTA update started! (free heap=%lu, max block=%lu)\r\n",
                  static_cast<unsigned long>(ESP.getFreeHeap()),
                  static_cast<unsigned long>(ESP.getMaxFreeBlockSize()));
}

void OTAService::handleProgress(size_t current, size_t final)
{
    if (millis() - progressMillis > 1000)
    {
        progressMillis = millis();
        Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\r\n", current, final);
    }
}

void OTAService::handleEnd(bool success)
{
    uploadActive = false;

    if (success)
    {
        prepareActive = false;
        prepareStartMillis = 0;
        mqttBufferSizeBeforePrepare = 0;
        Serial.println("OTA update finished successfully!");
    }
    else
    {
        Serial.println("OTA failed, restoring runtime services.");
        restoreAfterPrepare();
        Serial.println("There was an error during OTA update!");
    }
}