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

static String buildConfigPayloadWithAssignedTempSensor(long sensorId)
{
    String payload =
        "{"
        "\"manufacturerId\":\"Domiot\"," 
        "\"modelId\":\"P1\"," 
        "\"firmwareVersion\":\"1.2.3\"," 
        "\"hardwareVersion\":\"rev-a\"," 
        "\"macAddress\":\"AA:BB:CC:DD:EE:FF\"," 
        "\"parameters\":null," 
        "\"sensors\":[{" 
        "\"sensorId\":";
    payload += String(sensorId);
    payload +=
        ","
        "\"deviceMac\":\"AA:BB:CC:DD:EE:FF\"," 
        "\"type\":\"TEMP\"," 
        "\"topic\":{\"type\":\"temperature\",\"path\":\"meterbox/sensor/temp\"}," 
        "\"parameters\":null"
        "}],"
        "\"actuators\":null"
        "}";
    return payload;
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
    TEST_ASSERT_EQUAL_STRING("config/#", PubSubClient::lastSubscribeTopic.c_str());
    TEST_ASSERT_EQUAL_UINT(2048, PubSubClient::bufferSize);

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
    TEST_ASSERT_TRUE(actuators.isNull());
}

void test_callback_processes_config_subtopic_payload(void)
{
    WiFiClient wifiClient;
    MqttService mqttService("broker.local", 1883, "user", "pass", "client-1", &wifiClient);

    String payload = buildConfigPayloadWithAssignedTempSensor(42);
    byte* payloadBytes = reinterpret_cast<byte*>(const_cast<char*>(payload.c_str()));
    char topic[] = "config/client-1";

    mqttService.callback(topic, payloadBytes, payload.length());

    Device& assignedDevice = mqttService.getDevice();
    TEST_ASSERT_EQUAL_INT(42, assignedDevice.getSensorIdByType(SensorType::TEMP));
    TEST_ASSERT_FALSE(assignedDevice.hasUnassignedSensors());
}

void test_device_to_json_writes_null_for_empty_strings(void)
{
    Device device;

    ConfigParameter deviceParameter;
    deviceParameter.setName("");
    deviceParameter.setParameterType("");
    deviceParameter.setValue(String(""));
    deviceParameter.setReadonly(false);
    device.parameters().push_back(deviceParameter);

    Sensor sensor(7, "", SensorType::TEMP);
    sensor.topic().setType("");
    sensor.topic().setPath("");

    ConfigParameter sensorParameter;
    sensorParameter.setName("");
    sensorParameter.setParameterType("");
    sensorParameter.setValue(String(""));
    sensorParameter.setReadonly(true);
    sensor.parameters().push_back(sensorParameter);
    device.sensors().push_back(sensor);

    Actuator actuator;
    actuator.setActuatorId(9);
    actuator.setDeviceMac("");
    actuator.setType("");
    actuator.topic().setType("");
    actuator.topic().setPath("");
    device.actuators().push_back(actuator);

    JsonDocument doc;
    Device::toJson(device, doc);

    JsonObject capturedObject = doc.as<JsonObject>();
    TEST_ASSERT_FALSE(capturedObject.isNull());
    TEST_ASSERT_TRUE(capturedObject["manufacturerId"].isNull());
    TEST_ASSERT_TRUE(capturedObject["modelId"].isNull());
    TEST_ASSERT_TRUE(capturedObject["firmwareVersion"].isNull());
    TEST_ASSERT_TRUE(capturedObject["hardwareVersion"].isNull());
    TEST_ASSERT_TRUE(capturedObject["macAddress"].isNull());

    JsonArray parameters = capturedObject["parameters"].as<JsonArray>();
    TEST_ASSERT_FALSE(parameters.isNull());
    TEST_ASSERT_EQUAL_UINT(1, parameters.size());

    JsonObject firstParameter = parameters[0].as<JsonObject>();
    TEST_ASSERT_TRUE(firstParameter["name"].isNull());
    TEST_ASSERT_TRUE(firstParameter["parameterType"].isNull());
    TEST_ASSERT_TRUE(firstParameter["value"].isNull());

    JsonArray sensors = capturedObject["sensors"].as<JsonArray>();
    TEST_ASSERT_FALSE(sensors.isNull());
    TEST_ASSERT_EQUAL_UINT(1, sensors.size());

    JsonObject firstSensor = sensors[0].as<JsonObject>();
    TEST_ASSERT_TRUE(firstSensor["deviceMac"].isNull());

    JsonObject sensorTopic = firstSensor["topic"].as<JsonObject>();
    TEST_ASSERT_FALSE(sensorTopic.isNull());
    TEST_ASSERT_TRUE(sensorTopic["type"].isNull());
    TEST_ASSERT_TRUE(sensorTopic["path"].isNull());

    JsonArray sensorParameters = firstSensor["parameters"].as<JsonArray>();
    TEST_ASSERT_FALSE(sensorParameters.isNull());
    TEST_ASSERT_EQUAL_UINT(1, sensorParameters.size());

    JsonObject firstSensorParameter = sensorParameters[0].as<JsonObject>();
    TEST_ASSERT_TRUE(firstSensorParameter["name"].isNull());
    TEST_ASSERT_TRUE(firstSensorParameter["parameterType"].isNull());
    TEST_ASSERT_TRUE(firstSensorParameter["value"].isNull());

    JsonArray actuators = capturedObject["actuators"].as<JsonArray>();
    TEST_ASSERT_FALSE(actuators.isNull());
    TEST_ASSERT_EQUAL_UINT(1, actuators.size());

    JsonObject firstActuator = actuators[0].as<JsonObject>();
    TEST_ASSERT_TRUE(firstActuator["deviceMac"].isNull());
    TEST_ASSERT_TRUE(firstActuator["type"].isNull());

    JsonObject actuatorTopic = firstActuator["topic"].as<JsonObject>();
    TEST_ASSERT_FALSE(actuatorTopic.isNull());
    TEST_ASSERT_TRUE(actuatorTopic["type"].isNull());
    TEST_ASSERT_TRUE(actuatorTopic["path"].isNull());
    TEST_ASSERT_TRUE(firstActuator["parameters"].isNull());
}

void test_device_to_json_writes_null_for_empty_lists(void)
{
    Device device;

    JsonDocument doc;
    Device::toJson(device, doc);

    JsonObject capturedObject = doc.as<JsonObject>();
    TEST_ASSERT_FALSE(capturedObject.isNull());
    TEST_ASSERT_TRUE(capturedObject["parameters"].isNull());
    TEST_ASSERT_TRUE(capturedObject["sensors"].isNull());
    TEST_ASSERT_TRUE(capturedObject["actuators"].isNull());
}

int main(int argc, char** argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_register_device_subscribes_and_publishes_expected_payload);
    RUN_TEST(test_callback_processes_config_subtopic_payload);
    RUN_TEST(test_device_to_json_writes_null_for_empty_strings);
    RUN_TEST(test_device_to_json_writes_null_for_empty_lists);
    return UNITY_END();
}
