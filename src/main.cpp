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
#include <SensorValue.h>
#include <vector>
#include "P1DatagramSensorValueMapper.h"

#define MAX_WIFI_RETRIES 20
#define SENSOR_VALUE_TOPIC "sensor"

AsyncWebServer server(80);

WiFiClient wifiClient;

SoftwareSerial mySerial(SERIAL_RX, -1, true); // (RX, TX, inverted)

String deviceName = "ESP_Sensor";
String deviceType = "TemperatureSensor";

MqttService *mqttService = nullptr;
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
        // WiFi.setOutputPower(0); // Reduce WiFi power to minimal and hopefully reduce interference with smart meter
        // WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
        // WiFi.setPhyMode(WIFI_PHY_MODE_11B);
        WiFi.setAutoReconnect(true);
        WiFi.persistent(false);

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

        if (device.getDeviceId() == 0)
        {
            Serial.println("No device ID configured, registering device...");
            mqttService->registerDevice(device);
            Serial.println("Device registration initiated, waiting for device ID assignment...");
            // Wait for device to be registered and assigned an ID. The ID will be set in the callback.
            int retryCount = 0;
            while (device.getDeviceId() == 0)
            {
                mqttService->getClient().loop();
                delay(100);
                retryCount++;
                if (retryCount > 50) { // Timeout after 5 seconds
                    Serial.println("Timeout waiting for device ID assignment.");
                    Serial.println("Re-registering device...");
                    mqttService->registerDevice(device);
                    retryCount = 0;
                }
            }
            Serial.printf("Assigned device ID: %ld\n", device.getDeviceId());
            // Update sensorIds after registration
            tempSensorId = device.getSensorIdByType(SensorType::TEMP);
            batterySensorId = device.getSensorIdByType(SensorType::VOLTAGE_LEVEL);
        }
        else
        {
            Serial.printf("Using configured device ID: %ld\n", device.getDeviceId());
        }
    }
    else
    {
        Serial.println("No WiFi credentials configured - OTA available but no MQTT");
    }

    Serial.println("Setup complete!");
}

void publishTemperatureSensorValue(unsigned long now, unsigned long lastPublish, const String& timestamp)
{
    if (tempSensorId != 0 && now - lastPublish >= 1000)
    {
        float temperature = TemperatureSensor::readTemperature();
        if (temperature != lastSentTemperature)
        {
            String payload = "{";
            payload += "\"sensorId\": " + String(tempSensorId) + ", ";
            payload += "\"timestamp\": \"" + timestamp + "\", ";
            lastSentTemperature = temperature;
            payload += "\"value\": " + String(temperature, 2);
            payload += "}";
            mqttService->getClient().publish(SENSOR_VALUE_TOPIC, payload.c_str());
        }
    }
}

void publishBatterySensorValue(unsigned long now, unsigned long lastPublish, const String& timestamp)
{
    if (batterySensorId != 0 && now - lastPublish >= 1000)
    {
        float batteryLevel = BatteryLevel::readBatteryLevel();
        if (batteryLevel != lastSentBatteryLevel)
        {
            String payload = "{";
            lastSentBatteryLevel = batteryLevel;
            payload += "\"sensorId\": " + String(batterySensorId) + ", ";
            payload += "\"timestamp\": \"" + timestamp + "\", ";
            payload += "\"value\": " + String(batteryLevel, 2);
            payload += "}";
            mqttService->getClient().publish(SENSOR_VALUE_TOPIC, payload.c_str());
        }
    }
}

void publishP1SensorValues()
{
    if (device.getSensorIdByType(SensorType::POWER_CT1) != 0)
    {
        // Read P1 data here
        P1Datagram p1Datagram = P1Reader::readDatagram(mySerial);

        // Map P1Datagram to SensorValues and publish
        std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValues(device, p1Datagram);

        for (SensorValue sv : sensorValues)
        {
            String payload = sv.toString();
            mqttService->getClient().publish(SENSOR_VALUE_TOPIC, payload.c_str());
        }
    }
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
        
        publishTemperatureSensorValue(now, lastSentMqttPublish, timestamp);
        publishBatterySensorValue(now, lastSentMqttPublish, timestamp);
        publishP1SensorValues();
        
        lastSentMqttPublish = now;
    }
    else
    {
        // WiFi not connected - just yield to allow async operations
        yield();
    }
}
