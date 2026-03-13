#include <unity.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoFake.h>

#include <MqttService.h>
#include <PubSubClient.h>

using namespace fakeit;

void setUp(void)
{
    ArduinoFakeReset();
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char[]))).AlwaysReturn(1);
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char[]))).AlwaysReturn(1);

    PubSubClient::resetMockState();
}

void tearDown(void)
{
}

void test_register_device_subscribes_and_publishes_expected_payload(void)
{
    WiFiClient wifiClient;
    MqttService mqttService("broker.local", 1883, "user", "pass", "client-1", &wifiClient);

    Device device;
    device.setManufacturerId("Domiot");
    device.setModelId("P1");
    device.setFirmwareVersion("1.2.3");
    device.setHardwareVersion("rev-a");
    device.setMacAddress("AA:BB:CC:DD:EE:FF");

    ConfigParameter deviceParameter;
    deviceParameter.setName("intervalMs");
    deviceParameter.setParameterType("int");
    deviceParameter.setValue(1000);
    deviceParameter.setReadonly(false);
    device.parameters().push_back(deviceParameter);

    Sensor sensor(7, "AA:BB:CC:DD:EE:FF", SensorType::TEMP);
    sensor.topic().setType("temperature");
    sensor.topic().setPath("meterbox/sensor/temp");

    ConfigParameter sensorParameter;
    sensorParameter.setName("precision");
    sensorParameter.setParameterType("float");
    sensorParameter.setValue(0.1f);
    sensorParameter.setReadonly(true);
    sensor.parameters().push_back(sensorParameter);
    device.sensors().push_back(sensor);

    mqttService.registerDevice(device);

    TEST_ASSERT_TRUE(PubSubClient::subscribeCalled);
    TEST_ASSERT_EQUAL_STRING("config", PubSubClient::lastSubscribeTopic.c_str());

    TEST_ASSERT_TRUE(PubSubClient::beginPublishCalled);
    TEST_ASSERT_TRUE(PubSubClient::endPublishCalled);
    TEST_ASSERT_TRUE(PubSubClient::publishCalled);
    TEST_ASSERT_EQUAL_STRING("register", PubSubClient::lastPublishTopic.c_str());

    JsonDocument captured;
    DeserializationError error = deserializeJson(captured, PubSubClient::lastPublishPayload.c_str());
    TEST_ASSERT_TRUE_MESSAGE(!error, "Captured register payload should be valid JSON");

    JsonObject capturedObject = captured.as<JsonObject>();
    TEST_ASSERT_FALSE(capturedObject.isNull());

    TEST_ASSERT_EQUAL_STRING("Domiot", capturedObject["manufacturerId"] | "");
    TEST_ASSERT_EQUAL_STRING("P1", capturedObject["modelId"] | "");
    TEST_ASSERT_EQUAL_STRING("1.2.3", capturedObject["firmwareVersion"] | "");
    TEST_ASSERT_EQUAL_STRING("rev-a", capturedObject["hardwareVersion"] | "");
    TEST_ASSERT_EQUAL_STRING("AA:BB:CC:DD:EE:FF", capturedObject["macAddress"] | "");

    JsonArray parameters = capturedObject["parameters"].as<JsonArray>();
    TEST_ASSERT_FALSE(parameters.isNull());
    TEST_ASSERT_EQUAL_UINT(1, parameters.size());

    JsonObject firstParameter = parameters[0].as<JsonObject>();
    TEST_ASSERT_EQUAL_STRING("intervalMs", firstParameter["name"] | "");
    TEST_ASSERT_EQUAL_STRING("int", firstParameter["parameterType"] | "");
    TEST_ASSERT_EQUAL_INT(1000, firstParameter["value"].as<int>());
    TEST_ASSERT_FALSE(firstParameter["readonly"].as<bool>());

    JsonArray sensors = capturedObject["sensors"].as<JsonArray>();
    TEST_ASSERT_FALSE(sensors.isNull());
    TEST_ASSERT_EQUAL_UINT(1, sensors.size());

    JsonObject firstSensor = sensors[0].as<JsonObject>();
    TEST_ASSERT_EQUAL_INT(7, firstSensor["sensorId"].as<int>());
    TEST_ASSERT_EQUAL_STRING("AA:BB:CC:DD:EE:FF", firstSensor["deviceMac"] | "");
    TEST_ASSERT_EQUAL_STRING("TEMP", firstSensor["type"] | "");

    JsonObject topic = firstSensor["topic"].as<JsonObject>();
    TEST_ASSERT_FALSE(topic.isNull());
    TEST_ASSERT_EQUAL_STRING("temperature", topic["type"] | "");
    TEST_ASSERT_EQUAL_STRING("meterbox/sensor/temp", topic["path"] | "");

    JsonArray sensorParameters = firstSensor["parameters"].as<JsonArray>();
    TEST_ASSERT_FALSE(sensorParameters.isNull());
    TEST_ASSERT_EQUAL_UINT(1, sensorParameters.size());

    JsonObject firstSensorParameter = sensorParameters[0].as<JsonObject>();
    TEST_ASSERT_EQUAL_STRING("precision", firstSensorParameter["name"] | "");
    TEST_ASSERT_EQUAL_STRING("float", firstSensorParameter["parameterType"] | "");
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.1f, firstSensorParameter["value"].as<float>());
    TEST_ASSERT_TRUE(firstSensorParameter["readonly"].as<bool>());

    JsonArray actuators = capturedObject["actuators"].as<JsonArray>();
    TEST_ASSERT_FALSE(actuators.isNull());
    TEST_ASSERT_EQUAL_UINT(0, actuators.size());
}

int main(int argc, char** argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_register_device_subscribes_and_publishes_expected_payload);
    return UNITY_END();
}
