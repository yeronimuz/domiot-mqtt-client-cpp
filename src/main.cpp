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

constexpr uint8_t DATA_LINE_LED_PIN = LED_BUILTIN;
constexpr uint8_t DATA_LINE_LED_ACTIVE_LEVEL = LOW;
constexpr uint8_t DATA_LINE_LED_IDLE_LEVEL = HIGH;
constexpr unsigned long DATA_LINE_LED_FLASH_DURATION_MS = 35;
constexpr unsigned long DATA_LINE_LED_FLASH_INTERVAL_MS = 100;

AsyncWebServer server(80);

WiFiClient wifiClient;

SoftwareSerial mySerial(SERIAL_RX, -1, true); // (RX, TX (one wire protocol), inverted)

MqttService *mqttService = nullptr;
Device device;
long tempSensorId;
long batterySensorId;

unsigned long ota_progress_millis = 0;
float lastSentTemperature = 0.0;
float lastSentBatteryLevel = 0.0;
unsigned long lastSentMqttPublish = 0;
unsigned long dataLineLedOffMillis = 0;
unsigned long lastDataLineFlashMillis = 0;
TimeService timeService;
WiFiEventHandler wifiGotIpEventHandler;
WiFiEventHandler wifiDisconnectedEventHandler;
bool wifiAddressLogged = false;
bool wifiConnectionEstablished = false;
bool otaServiceStarted = false;
bool p1SerialInitialized = false;

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

static void initWifiDiagnostics()
{
    wifiGotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event)
    {
        wifiConnectionEstablished = true;
        wifiAddressLogged = true;
        Serial.printf("DHCP assigned IP: %s\n", event.ip.toString().c_str());
        Serial.printf("DHCP subnet mask: %s\n", event.mask.toString().c_str());
        Serial.printf("DHCP gateway: %s\n", event.gw.toString().c_str());
    });

    wifiDisconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event)
    {
        if (!wifiConnectionEstablished)
        {
            return;
        }

        wifiAddressLogged = false;
        Serial.printf("WiFi disconnected (reason=%d, ssid=%s)\n", event.reason, event.ssid.c_str());
    });
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

static void setupOtaService(const String& otaUsername, const String& otaPassword)
{
    if (otaServiceStarted)
    {
        return;
    }

    // Setup web server routes before starting.
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", "Domiot Sensor update facility on /update."); });

    // Setup ElegantOTA.
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
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);

    server.begin();
    otaServiceStarted = true;
    Serial.printf("HTTP server started on port 80 (free heap=%lu)\n", static_cast<unsigned long>(ESP.getFreeHeap()));
}

static void setupP1Serial()
{
    if (p1SerialInitialized)
    {
        return;
    }

    mySerial.begin(115200, SWSERIAL_8N1, SERIAL_RX, -1, true, 512);
    mySerial.setTimeout(250); // Keep line reads responsive; P1 lines should end quickly.
    while (mySerial.available())
    {
        mySerial.read();
    }

    p1SerialInitialized = true;
    Serial.printf("P1 serial port initialized (free heap=%lu)\n", static_cast<unsigned long>(ESP.getFreeHeap()));
}

void setup()
{
    Serial.begin(115200);
    initDataLineActivityLed();
    delay(1000);
    Serial.println("\n\nStarting Domiot MQTT Client...");

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
    Serial.printf("Free heap after config load: %lu\n", static_cast<unsigned long>(ESP.getFreeHeap()));

    // For registration purposes, we need to set the MAC address in the device config, as it's used as a unique identifier for the device. 
    device.setMacAddress(WiFi.macAddress());

    if (strlen(wifiConfig.getWifiAccessPoint().c_str()) > 0)
    {
        Serial.printf("Connecting to WiFi (%s)...\n", wifiConfig.getWifiAccessPoint().c_str());
        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true);
        WiFi.persistent(false);
        initWifiDiagnostics();

        String wifiHostname = resolveWifiHostname(wifiConfig, mqttConfig);
        WiFi.hostname(wifiHostname);
        Serial.printf("Using WiFi hostname: %s\n", wifiHostname.c_str());

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
            Serial.printf("\nWiFi connection failed after %d retries. Setting up WiFi-less mode (OTA only).\n", wifiRetries);
            setupOtaService(otaUsername, otaPassword);
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
            Serial.printf("Free heap before registration: %lu\n", static_cast<unsigned long>(ESP.getFreeHeap()));
            mqttService->registerDevice(device);
            Serial.println("Device registration initiated, waiting for sensor ID assignment...");
            int retryCount = 0;
            while (device.hasUnassignedSensors())
            {
                mqttService->getClient().loop();

                if (!device.hasUnassignedSensors(mqttService->getDevice()))
                {
                    device = mqttService->getDevice();
                    Serial.println("Received assigned sensor IDs from config.");
                    Serial.println("Updated device configuration:");
                    Serial.println(device.toString(true));
                    break;
                }

                delay(100);
                retryCount++;
                if (retryCount > 50) { // Timeout after 5 seconds
                    Serial.println("Timeout waiting for sensor ID assignment.");
                    Serial.println("Re-registering device...");
                    Serial.printf("Free heap before re-registration: %lu\n", static_cast<unsigned long>(ESP.getFreeHeap()));
                    mqttService->registerDevice(device);
                    retryCount = 0;
                }
            }
            Serial.printf("Free heap after registration: %lu\n", static_cast<unsigned long>(ESP.getFreeHeap()));
            Serial.println("Sensor ID assignment completed.");
            // Update sensorIds after registration
            tempSensorId = device.getSensorIdByType(SensorType::TEMP);
            batterySensorId = device.getSensorIdByType(SensorType::VOLTAGE_LEVEL);
        }
        else
        {
            Serial.println("Using configured sensor IDs from device configuration.");
        }

        setupOtaService(otaUsername, otaPassword);
    }
    else
    {
        Serial.println("No WiFi credentials configured - OTA available but no MQTT");
        setupOtaService(otaUsername, otaPassword);
    }

    setupP1Serial();

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
        static unsigned long lastValidDatagramMillis = 0;
        static unsigned long lastP1StatusLogMillis = 0;

        // Read P1 data continuously to avoid dropping bytes from the serial buffer.
        P1Datagram p1Datagram = P1Reader::readDatagram(mySerial);

        // Consider a datagram valid when at least one core header field is parsed.
        bool hasCoreP1Fields =
            p1Datagram.getTimestamp().length() > 0 ||
            p1Datagram.getVersionInfo() != 0 ||
            p1Datagram.getEquipmentId().length() > 0;

        if (!hasCoreP1Fields)
        {
            unsigned long now = millis();
            if (now - lastP1StatusLogMillis >= 5000)
            {
                lastP1StatusLogMillis = now;
                unsigned long msSinceLastDatagram =
                    (lastValidDatagramMillis == 0) ? 0 : (now - lastValidDatagramMillis);
                Serial.printf("P1 waiting for valid frame (avail=%d, ms_since_last=%lu)\n",
                              mySerial.available(),
                              msSinceLastDatagram);
            }
            return;
        }

        lastValidDatagramMillis = millis();
        Serial.printf("P1 datagram received (ts=%s, version=%u)\n",
                      p1Datagram.getTimestamp().c_str(),
                      p1Datagram.getVersionInfo());

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

    flashDataLineLedOnActivity(mySerial.available() > 0);

    // Maintain WiFi connection
    if (WiFi.status() == WL_CONNECTED)
    {
        if (!wifiAddressLogged)
        {
            wifiConnectionEstablished = true;
            wifiAddressLogged = true;
            Serial.printf("WiFi connected, DHCP IP: %s\n", WiFi.localIP().toString().c_str());
            logNetworkInfo();
        }

        if (mqttService == nullptr)
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
