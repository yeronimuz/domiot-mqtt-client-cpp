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

#define MAX_WIFI_RETRIES 20
#define INCLUDE_TEMPERATURE_SENSOR false
#define INCLUDE_BATTERY_LEVEL_SENSOR false

/* Device's AP when not configured */
const char *ssid = "TBD-SSID"; // AP SSID
const char *password = "";     // No password

AsyncWebServer server(80);

WiFiClient wifiClient;

String deviceName = "ESP_Sensor";
String deviceType = "TemperatureSensor";

MqttService mqttService;
Device device;

unsigned long ota_progress_millis = 0;

String getTimestamp()
{
    // Format timestamp as ISO 8601: "2025-03-17T21:55:59.524164121"
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    char timestamp_buffer[30];
    strftime(timestamp_buffer, sizeof(timestamp_buffer), "%Y-%m-%dT%H:%M:%S", timeinfo);
    unsigned long milliseconds = millis() % 1000;
    return String(timestamp_buffer) + "." + String(milliseconds * 1000000, DEC);
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
    // <Add your own code here>
}

void setup()
{
    Serial.begin(115200);

    DomiotConfig config = DomiotConfig();
    WifiConfig wifiConfig = config.getWifiConfig();
    MqttConfig mqttConfig = config.getMqttConfig();
    device = config.getDevice();

    if (strlen(wifiConfig.getWifiAccessPoint().c_str()) > 0)
    {
        Serial.printf("Connecting to WiFi (%s)...\n", wifiConfig.getWifiAccessPoint().c_str());
        WiFi.begin(wifiConfig.getWifiAccessPoint(), wifiConfig.getWifiPassKey());

        mqttService = MqttService(
            mqttConfig.getMqttBroker(),
            mqttConfig.getMqttUser(),
            mqttConfig.getMqttPassword(),
            &wifiClient);

        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < MAX_WIFI_RETRIES)
        {
            delay(500);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\nWiFi Connected!");
            Serial.println("IP Address: " + WiFi.localIP().toString());

            server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                      { request->send(200, "text/plain", "Hi! This is ElegantOTA AsyncDemo."); });

            ElegantOTA.begin(&server); // Start ElegantOTA
            // ElegantOTA callbacks
            ElegantOTA.onStart(onOTAStart);
            ElegantOTA.onProgress(onOTAProgress);
            ElegantOTA.onEnd(onOTAEnd);

            server.begin();
            Serial.println("HTTP server started");
            mqttService.connect();
            mqttService.registerDevice(device);
        }
        else
        {
            Serial.printf("\nFailed to connect to WiFi after %d attempts\n", MAX_WIFI_RETRIES);
            Serial.println("Shutting down...");
            while (true)
            {
                delay(1000);
            }
        }
    }
}

void loop()
{
    bool isNewTemperaturePresent = false;
    bool isNewBatteryLevelPresent = false;
    JsonDocument jsonDoc;
    long deviceId = device.deviceId;
    // TODO: Set common data
    /*
     * {"sensorId":6,"timestamp":"2025-03-17T21:55:59.524164121","value":0.359}
     */
    float lastSentTemperature = 0.0;
    float lastSentBatteryLevel = 0.0;

    if (!mqttService.isConnected())
    {
        mqttService.connect();
    }
    mqttService.getClient().loop();

    String timestamp = getTimestamp();
    String payload = "{";
    payload += "\"sensorId\":2,";
    payload += "\"timestamp\":\"" + timestamp + "\",";
    if (INCLUDE_TEMPERATURE_SENSOR)
    {
        float temperature = TemperatureSensor::readTemperature();
        if (temperature != lastSentTemperature)
        {
            // TODO: set sensorId properly
            lastSentTemperature = temperature;
            payload += "\"value\":" + String(temperature, 2);
            payload += "}";
            mqttService.getClient().publish("sensor", payload.c_str());
        }
    }
    if (INCLUDE_BATTERY_LEVEL_SENSOR)
    {
        float batteryLevel = BatteryLevel::readBatteryLevel();
        if (batteryLevel != lastSentBatteryLevel)
        {
            lastSentBatteryLevel = batteryLevel;
            payload += "\"value\":" + String(batteryLevel, 2);
            payload += "}";
            mqttService.getClient().publish("sensor", payload.c_str());
        }
    }
    else
    {
        payload += "\"value\": 0.0";
        payload += "}";
        mqttService.getClient().publish("sensor", payload.c_str());
    }
}
