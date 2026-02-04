#include <Arduino.h>
#include <ESP8266WiFi.h> // Use <WiFi.h> for ESP32
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include "constants.h"
#include <TemperatureSensor.h>
#include <BatteryLevel.h>
#include <MqttService.h>
#include <DomiotConfig.h>
#include <P1Reader.h>
#include <P1Datagram.h>

#define MAX_WIFI_RETRIES 20
#define INCLUDE_TEMPERATURE_SENSOR false
#define INCLUDE_BATTERY_LEVEL_SENSOR false
#define SENSOR_VALUE_TOPIC "sensor"

AsyncWebServer server(80);

WiFiClient wifiClient;

SoftwareSerial mySerial(SERIAL_RX, -1, true); // (RX, TX, inverted)

String deviceName = "ESP_Sensor";
String deviceType = "TemperatureSensor";

MqttService* mqttService = nullptr;
Device device;
long tempSensorId;
long batterySensorId;

unsigned long ota_progress_millis = 0;
float lastSentTemperature = 0.0;
float lastSentBatteryLevel = 0.0;
unsigned long lastSentMqttPublish = 0;

String getTimestamp()
{
    // Format timestamp as ISO 8601: "2025-03-17T21:55:59.524"
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    char timestamp_buffer[30];
    strftime(timestamp_buffer, sizeof(timestamp_buffer), "%Y-%m-%dT%H:%M:%S", timeinfo);
    unsigned long milliseconds = millis() % 1000;
    char ms_buffer[4];
    sprintf(ms_buffer, "%03lu", milliseconds);
    return String(timestamp_buffer) + "." + String(ms_buffer);
}

void onOTAStart()
{
    Serial.println("OTA update started!");
}

void onOTAProgress(size_t current, size_t final)
{
    // Log every 1 second
    if (millis() - ota_progress_millis > 1000)
    {
        ota_progress_millis = millis();
        Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
    }
}

void onOTAEnd(bool success)
{
    if (success)
    {
        Serial.println("OTA update finished successfully!");
    }
    else
    {
        Serial.println("There was an error during OTA update!");
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\nStarting Domiot MQTT Client...");

    DomiotConfig config = DomiotConfig();
    WifiConfig wifiConfig = config.getWifiConfig();
    MqttConfig mqttConfig = config.getMqttConfig();
    device = config.getDevice();

    // Setup web server routes before starting
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", "Domiot Sensor update facility on /update."); });

    // Setup ElegantOTA
    ElegantOTA.begin(&server);
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);

    // Start the server immediately (before WiFi connects)
    server.begin();
    Serial.println("HTTP server started on port 80");

    if (strlen(wifiConfig.getWifiAccessPoint().c_str()) > 0)
    {
        Serial.printf("Connecting to WiFi (%s)...\n", wifiConfig.getWifiAccessPoint().c_str());
        WiFi.mode(WIFI_STA);
        WiFi.begin(wifiConfig.getWifiAccessPoint(), wifiConfig.getWifiPassKey());
        Serial.println("WiFi connection initiated (non-blocking)");

        mqttService = new MqttService(
            mqttConfig.getMqttBroker(),
            mqttConfig.getMqttUser(),
            mqttConfig.getMqttPassword(),
            &wifiClient);
        Serial.print("Connecting to MQTT ");
        while (!mqttService->isConnected())
        {
            mqttService->connect();
            delay(500);
        }
        Serial.println("\nMQTT connected!");
        
        if (device._deviceId == 0)
        {
            Serial.println("No device ID configured, registering device...");
            mqttService->registerDevice(device);
        }
        else
        {
            Serial.printf("Using configured device ID: %ld\n", device._deviceId);
        }

    }
    else
    {
        Serial.println("No WiFi credentials configured - OTA available but no MQTT");
    }

    Serial.println("Setup complete!");
}

void loop()
{
    // OTA update handling - call this first
    ElegantOTA.loop();
    yield();

    // Maintain WiFi connection
    if (WiFi.status() == WL_CONNECTED)
    {
        // MQTT client loop - non-blocking
        if (!mqttService->isConnected())
        {
            mqttService->connect();
        }
        mqttService->getClient().loop();

        unsigned long now = millis();

        String timestamp = getTimestamp();
        String payload;
        payload += "{";
        payload += "\"sensorId\": 0, ";
        payload += "\"timestamp\": \"" + timestamp + "\", ";

        if (INCLUDE_TEMPERATURE_SENSOR && now - lastSentMqttPublish >= 1000)
        {
            float temperature = TemperatureSensor::readTemperature();
            if (temperature != lastSentTemperature)
            {
                lastSentTemperature = temperature;
                payload += "\"value\": " + String(temperature, 2);
                payload += "}";
                mqttService->getClient().publish(SENSOR_VALUE_TOPIC, payload.c_str());
            }
        }
        else if (INCLUDE_BATTERY_LEVEL_SENSOR && now - lastSentMqttPublish >= 1000)
        {
            float batteryLevel = BatteryLevel::readBatteryLevel();
            if (batteryLevel != lastSentBatteryLevel)
            {
                lastSentBatteryLevel = batteryLevel;
                payload += "\"value\": " + String(batteryLevel, 2);
                payload += "}";
                mqttService->getClient().publish(SENSOR_VALUE_TOPIC, payload.c_str());
            }
        }
        else
        {
            // Read P1 data here
            P1Datagram p1Datagram = P1Reader::readDatagram(mySerial);
            // Map P1Datagram to SensorValues and publish
            String* payloadParts = p1Datagram.getAsPayload();
            for (int i = 0; i < MAX_P1_ITEMS && payloadParts[i][0] != 0; i++)
            {
                payload += payloadParts[i];
                payload += "}";
                mqttService->getClient().publish(SENSOR_VALUE_TOPIC, payload.c_str());
            }
            delete[] payloadParts;
        }
        lastSentMqttPublish = now;
    }
    else
    {
        // WiFi not connected - just yield to allow async operations
        yield();
    }
}
