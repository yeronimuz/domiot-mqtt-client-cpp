#include <Arduino.h>
#include <ESP8266WiFi.h> // Use <WiFi.h> for ESP32
#include <ArduinoJson.h>
#include "constants.h"
#include <OTAService.h>
#include <TemperatureSensor.h>
#include <BatteryLevel.h>
#include <MqttService.h>
#include <DomiotConfig.h>
#include <P1Reader.h>
#include <P1Datagram.h>
#include <P1Debug.h>
#include <SensorValue.h>
#include <vector>
#include <map>
#include "P1DatagramSensorValueMapper.h"
#include <TimeService.h>

#define SENSOR_VALUE_TOPIC "sensor"

constexpr uint8_t DATA_LINE_LED_PIN = LED_BUILTIN;
constexpr uint8_t DATA_LINE_LED_ACTIVE_LEVEL = LOW;
constexpr uint8_t DATA_LINE_LED_IDLE_LEVEL = HIGH;
constexpr unsigned long DATA_LINE_LED_FLASH_DURATION_MS = 35;
constexpr unsigned long DATA_LINE_LED_FLASH_INTERVAL_MS = 100;
constexpr unsigned int P1_SERIAL_RX_BUFFER_SIZE = 1024;
constexpr unsigned int P1_REPEAT_INTERVAL_MS = 60000;

WiFiClient wifiClient;

HardwareSerial &p1Serial = Serial;
OTAService otaService;

MqttService *mqttService = nullptr;
Device device;
long tempSensorId;
long batterySensorId;

unsigned long lastSentMqttPublish = 0;
unsigned long dataLineLedOffMillis = 0;
unsigned long lastDataLineFlashMillis = 0;
TimeService timeService;
WiFiEventHandler wifiGotIpEventHandler;
WiFiEventHandler wifiDisconnectedEventHandler;
bool wifiAddressLogged = false;
bool wifiConnectionEstablished = false;
bool p1SerialInitialized = false;

static void setupP1Serial();

static String sanitizeConfigString(String value)
{
    value.trim();
    if (value.equalsIgnoreCase("null") || value.equalsIgnoreCase("undefined"))
    {
        return "";
    }
    return value;
}

static String resolveWifiHostname(WifiConfig &wifiConfig, MqttConfig &mqttConfig)
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
    Serial.printf("WiFi hostname: %s\r\n", WiFi.hostname().c_str());
    Serial.printf("WiFi MAC: %s\r\n", WiFi.macAddress().c_str());
    Serial.printf("WiFi local IP: %s\r\n", WiFi.localIP().toString().c_str());
    Serial.printf("WiFi subnet mask: %s\r\n", WiFi.subnetMask().toString().c_str());
    Serial.printf("WiFi gateway IP: %s\r\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("WiFi DNS: %s\r\n", WiFi.dnsIP().toString().c_str());
}

static void initWifiDiagnostics()
{
    wifiGotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP &event)
                                                    {
        wifiConnectionEstablished = true;
        wifiAddressLogged = true;
        Serial.printf("DHCP assigned IP: %s\r\n", event.ip.toString().c_str());
        Serial.printf("DHCP subnet mask: %s\r\n", event.mask.toString().c_str());
        Serial.printf("DHCP gateway: %s\r\n", event.gw.toString().c_str()); });

    wifiDisconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &event)
                                                                  {
        if (!wifiConnectionEstablished)
        {
            return;
        }

        wifiAddressLogged = false;
        Serial.printf("WiFi disconnected (reason=%d, ssid=%s)\r\n", event.reason, event.ssid.c_str()); });
}

void initDataLineActivityLed()
{
    pinMode(DATA_LINE_LED_PIN, OUTPUT);
    digitalWrite(DATA_LINE_LED_PIN, DATA_LINE_LED_IDLE_LEVEL);
}

void flashDataLineLedOnActivity(bool dataAvailable)
{
    const unsigned long now = millis();

    if (dataAvailable && (now - lastDataLineFlashMillis >= DATA_LINE_LED_FLASH_INTERVAL_MS))
    {
        lastDataLineFlashMillis = now;
        dataLineLedOffMillis = now + DATA_LINE_LED_FLASH_DURATION_MS;
        digitalWrite(DATA_LINE_LED_PIN, DATA_LINE_LED_ACTIVE_LEVEL);
    }

    if (dataLineLedOffMillis != 0 && static_cast<long>(now - dataLineLedOffMillis) >= 0)
    {
        dataLineLedOffMillis = 0;
        digitalWrite(DATA_LINE_LED_PIN, DATA_LINE_LED_IDLE_LEVEL);
    }
}

static void setupP1Serial()
{
    if (p1SerialInitialized)
    {
        return;
    }

    p1Serial.setRxBufferSize(P1_SERIAL_RX_BUFFER_SIZE);
    p1Serial.begin(115200);
    p1Serial.setTimeout(250); // Keep line reads responsive; P1 lines should end quickly.
    while (p1Serial.available())
    {
        p1Serial.read();
    }

    p1SerialInitialized = true;
    Serial.printf("P1 serial initialized: (buffer=%d, free heap=%lu, max block=%lu)\r\n",
                  P1_SERIAL_RX_BUFFER_SIZE,
                  static_cast<unsigned long>(ESP.getFreeHeap()),
                  static_cast<unsigned long>(ESP.getMaxFreeBlockSize()));
}

void setup()
{
    Serial.begin(115200);
    initDataLineActivityLed();
    delay(1000);
    Serial.println("\n\nStarting Domiot MQTT Client...");

    otaService.attachP1Serial(&p1Serial, &p1SerialInitialized, setupP1Serial);

    WifiConfig wifiConfig;
    MqttConfig mqttConfig;
    String otaUsername;
    String otaPassword;

    {
        DomiotConfig config = DomiotConfig();
        wifiConfig = config.getWifiConfig();
        mqttConfig = config.getMqttConfig();
        otaUsername = sanitizeConfigString(config.getOtaUsername());
        otaPassword = sanitizeConfigString(config.getOtaPassword());
        device = config.getDevice();
    }
    Serial.printf("Free heap after config load: %lu\r\n", static_cast<unsigned long>(ESP.getFreeHeap()));

    // For registration purposes, we need to set the MAC address in the device config, as it's used as a unique identifier for the device.
    device.setMacAddress(WiFi.macAddress());

    if (strlen(wifiConfig.getWifiAccessPoint().c_str()) > 0)
    {
        Serial.printf("Connecting to WiFi (%s)...\r\n", wifiConfig.getWifiAccessPoint().c_str());
        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true);
        WiFi.persistent(false);
        initWifiDiagnostics();

        String wifiHostname = resolveWifiHostname(wifiConfig, mqttConfig);
        WiFi.hostname(wifiHostname);
        Serial.printf("Using WiFi hostname: %s\r\n", wifiHostname.c_str());

        WiFi.begin(wifiConfig.getWifiAccessPoint(), wifiConfig.getWifiPassKey());
        Serial.println("WiFi connection initiated (non-blocking)");

        // Wait for WiFi to connect before attempting MQTT
        int wifiRetries = 0;
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
            wifiRetries++;
        }

        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.printf("\nWiFi connection failed after %d retries. Setting up WiFi-less mode (OTA only).\r\n", wifiRetries);
            otaService.begin(otaUsername, otaPassword);
            Serial.println("Setup complete!");
            return;
        }

        Serial.println("\nWiFi connected!");

        mqttService = new MqttService(
            mqttConfig.getMqttBroker(),
            mqttConfig.getMqttPort(),
            mqttConfig.getMqttUser(),
            mqttConfig.getMqttPassword(),
            mqttConfig.getClientId(),
            &wifiClient);
        otaService.setMqttService(mqttService);

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
            Serial.printf("Free heap before registration: %lu\r\n", static_cast<unsigned long>(ESP.getFreeHeap()));
            mqttService->registerDevice(device);
            Serial.println("Device registration initiated, waiting for sensor ID assignment...");
            int retryCount = 0;
            while (device.hasUnassignedSensors())
            {
                mqttService->getClient().loop();

                const Device incomingDevice = mqttService->getDevice();
                if (!Device::hasUnassignedSensors(incomingDevice))
                {
                    device = incomingDevice;
                    Serial.println("Received assigned sensor IDs from config.");
                    Serial.println("Updated device configuration:");
                    Serial.println(device.toString(true));
                    break;
                }

                delay(100);
                retryCount++;
                if (retryCount > 50)
                { // Timeout after 5 seconds
                    Serial.println("Timeout waiting for sensor ID assignment.");
                    Serial.println("Re-registering device...");
                    Serial.printf("Free heap before re-registration: %lu\r\n", static_cast<unsigned long>(ESP.getFreeHeap()));
                    mqttService->registerDevice(device);
                    retryCount = 0;
                }
            }
            Serial.printf("Free heap after registration: %lu\r\n", static_cast<unsigned long>(ESP.getFreeHeap()));
            Serial.println("Sensor ID assignment completed.");
            // Update sensorIds after registration
            tempSensorId = device.getSensorIdByType(SensorType::TEMP);
            batterySensorId = device.getSensorIdByType(SensorType::VOLTAGE_LEVEL);
        }
        else
        {
            Serial.println("Using configured sensor IDs from device configuration.");
        }

        // Temporarily release MQTT resources to free heap for OTA auth processing
        // OTA needs heap for digest auth computation; we'll reconnect in the loop if needed
        Serial.printf("Free heap before OTA: %lu\r\n", static_cast<unsigned long>(ESP.getFreeHeap()));

        otaService.begin(otaUsername, otaPassword);
    }
    else
    {
        Serial.println("No WiFi credentials configured - OTA available but no MQTT");
        otaService.begin(otaUsername, otaPassword);
    }

    setupP1Serial();

    Serial.println("Setup complete!");
}

void publishP1SensorValues()
{
    if (!p1SerialInitialized || otaService.isPrepared())
    {
        return;
    }

    if (!device.hasUnassignedSensors())
    {
        // Read P1 data continuously to avoid dropping bytes from the serial buffer.
        P1Datagram p1Datagram = P1Reader::readDatagram(p1Serial);

        // The reader returns an empty placeholder datagram while a telegram is still
        // being accumulated. Only publish completed, parsed telegrams.
        if (p1Datagram.getTimestamp().length() == 0 || p1Datagram.getVersionInfo() == 0)
        {
            return;
        }

        // Map P1Datagram to SensorValues and publish
        std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValues(device, p1Datagram);

        // Track last published value and time per sensor to:
        //  - drop repeated values within the repeat interval
        //  - force republish after the repeat interval even when unchanged
        static std::map<long, std::pair<float, unsigned long>> lastPublished;
        const unsigned long nowMs = millis();

        for (SensorValue sv : sensorValues)
        {
            long id = sv.getSensorId();
            float val = sv.getValue();
            auto it = lastPublished.find(id);
            bool valueChanged = (it == lastPublished.end()) || (it->second.first != val);
            bool intervalElapsed = (it == lastPublished.end()) || (nowMs - it->second.second >= P1_REPEAT_INTERVAL_MS);

            if (!valueChanged && !intervalElapsed)
            {
                P1_DEBUG_PRINTF("P1 skip (unchanged, <1 min): sensorId=%ld value=%.3f\r\n", id, val);
                continue;
            }

            String payload = sv.toJson();
            P1_DEBUG_PRINTF("P1 publish: %s\r\n", payload.c_str());
            mqttService->getClient().publish(
                SENSOR_VALUE_TOPIC,
                reinterpret_cast<const uint8_t *>(payload.c_str()),
                payload.length());
            lastPublished[id] = {val, nowMs};
        }
    }
}

void publishBatteryLevelValue(unsigned long now)
{
    if (batterySensorId > 0 && timeService.ensureUtcTimeSynced())
    {
        BatteryLevel::publishSensorValue(
            mqttService->getClient(),
            batterySensorId,
            SENSOR_VALUE_TOPIC,
            timeService.getUtcTimestamp(),
            now,
            lastSentMqttPublish);
    }
}

void publishTemperatureValue(unsigned long now)
{
    if (tempSensorId > 0 && timeService.ensureUtcTimeSynced())
    {
        TemperatureSensor::publishSensorValue(
            mqttService->getClient(),
            tempSensorId,
            SENSOR_VALUE_TOPIC,
            timeService.getUtcTimestamp(),
            now,
            lastSentMqttPublish);
    }
}


void loop()
{

    otaService.loop();
    yield();

    flashDataLineLedOnActivity(p1SerialInitialized && p1Serial.available() > 0);

    // Maintain WiFi connection
    if (WiFi.status() == WL_CONNECTED)
    {
        if (!wifiAddressLogged)
        {
            wifiConnectionEstablished = true;
            wifiAddressLogged = true;
            Serial.printf("WiFi connected, DHCP IP: %s\r\n", WiFi.localIP().toString().c_str());
            logNetworkInfo();
        }

        if (mqttService == nullptr)
        {
            return;
        }

        if (otaService.isPrepared())
        {
            return;
        }

        // MQTT client loop - non-blocking
        if (!mqttService->isConnected())
        {
            mqttService->connect();
        }
        mqttService->getClient().loop();

        unsigned long now = millis();

        publishP1SensorValues();
        publishTemperatureValue(now);
        publishBatteryLevelValue(now);
        lastSentMqttPublish = now;
    }
    else
    {
        // WiFi not connected - just yield to allow async operations
        yield();
    }
}
