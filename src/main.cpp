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
#include <TimeService.h>

#define MAX_WIFI_RETRIES 20
#define SENSOR_VALUE_TOPIC "sensor"

AsyncWebServer server(80);

WiFiClient wifiClient;

SoftwareSerial mySerial(SERIAL_RX, -1, true); // (RX, TX (one wire protocol), inverted)

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
TimeService timeService;

static String sanitizeConfigString(String value)
{
    value.trim();
    if (value.equalsIgnoreCase("null") || value.equalsIgnoreCase("undefined"))
    {
        return "";
    }
    return value;
}

static String resolveWifiHostname(WifiConfig& wifiConfig, MqttConfig& mqttConfig)
{
    String wifiHostname = sanitizeConfigString(wifiConfig.getWifiHostname());
    if (wifiHostname.length() == 0)
    {
        wifiHostname = sanitizeConfigString(mqttConfig.getClientId());
    }
    if (wifiHostname.length() == 0)
    {
        wifiHostname = "domiot-" + String(ESP.getChipId(), HEX);
    }
    return wifiHostname;
}

static void logNetworkInfo()
{
    Serial.printf("WiFi hostname: %s\n", WiFi.hostname().c_str());
    Serial.printf("WiFi MAC: %s\n", WiFi.macAddress().c_str());
    Serial.printf("WiFi local IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("WiFi subnet mask: %s\n", WiFi.subnetMask().toString().c_str());
    Serial.printf("WiFi gateway IP: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("WiFi DNS: %s\n", WiFi.dnsIP().toString().c_str());
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
    // For registration purposes, we need to set the MAC address in the device config, as it's used as a unique identifier for the device. 
    device.setMacAddress(WiFi.macAddress());

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
        WiFi.setAutoReconnect(true);
        WiFi.persistent(false);

        String wifiHostname = resolveWifiHostname(wifiConfig, mqttConfig);
        WiFi.hostname(wifiHostname);
        Serial.printf("Using WiFi hostname: %s\n", wifiHostname.c_str());

        WiFi.begin(wifiConfig.getWifiAccessPoint(), wifiConfig.getWifiPassKey());
        Serial.println("WiFi connection initiated (non-blocking)");

        mqttService = new MqttService(
            mqttConfig.getMqttBroker(),
            mqttConfig.getMqttPort(),
            mqttConfig.getMqttUser(),
            mqttConfig.getMqttPassword(),
            mqttConfig.getClientId(),
            &wifiClient);

        Serial.print("Connecting to MQTT ");
        while (!mqttService->isConnected())
        {
            mqttService->connect();
            delay(500);
        }
        Serial.println("\nMQTT connected!");
        logNetworkInfo();

        timeService.syncUtcTime();
        if (timeService.isTimeSynced())
        {
            Serial.println("UTC time synchronized.");
        }
        else
        {
            Serial.println("UTC time not synchronized yet.");
        }

        if (device.hasUnassignedSensors())
        {
            Serial.println("One or more sensorIds are not assigned, registering device...");
            mqttService->registerDevice(device);
            Serial.println("Device registration initiated, waiting for sensor ID assignment...");
            // Wait for config response with assigned sensor IDs.
            int retryCount = 0;
            while (device.hasUnassignedSensors())
            {
                mqttService->getClient().loop();

                if (!device.hasUnassignedSensors(mqttService->getDevice()))
                {
                    device = mqttService->getDevice();
                    Serial.println("Received assigned sensor IDs from config.");
                    break;
                }

                delay(100);
                retryCount++;
                if (retryCount > 50) { // Timeout after 5 seconds
                    Serial.println("Timeout waiting for sensor ID assignment.");
                    Serial.println("Re-registering device...");
                    mqttService->registerDevice(device);
                    retryCount = 0;
                }
            }
            Serial.println("Sensor ID assignment completed.");
            // Update sensorIds after registration
            tempSensorId = device.getSensorIdByType(SensorType::TEMP);
            batterySensorId = device.getSensorIdByType(SensorType::VOLTAGE_LEVEL);
        }
        else
        {
            Serial.println("Using configured sensor IDs from device configuration.");
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
        // Sensor ID for POWER_CT1 (and presumably other P1 sensors) is configured, 
        // so we can continue to read and publish P1 sensor values
        
        // Read P1 data here
        P1Datagram p1Datagram = P1Reader::readDatagram(mySerial);

        // Map P1Datagram to SensorValues and publish
        std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValues(device, p1Datagram);

        for (SensorValue sv : sensorValues)
        {
            String payload = sv.toString();
            Serial.printf("Publishing P1 sensor value: %s\n", payload.c_str());
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

        publishP1SensorValues();

        if (timeService.ensureUtcTimeSynced())
        {
            String timestamp = timeService.getUtcTimestamp();
            publishTemperatureSensorValue(now, lastSentMqttPublish, timestamp);
            publishBatterySensorValue(now, lastSentMqttPublish, timestamp);
            lastSentMqttPublish = now;
        }
    }
    else
    {
        // WiFi not connected - just yield to allow async operations
        yield();
    }
}
