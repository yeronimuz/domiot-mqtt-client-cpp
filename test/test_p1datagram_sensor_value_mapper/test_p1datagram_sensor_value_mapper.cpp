#include <unity.h>
#include <Arduino.h>
#include <P1Parser.h>
#include <P1Datagram.h>
#include <P1DatagramSensorValueMapper.h>
#include <Device.h>
#include <Sensor.h>
#include <SensorType.h>
#include <SensorValue.h>
#include "TestP1Datagram.h"

void setUp(void) {
    // Set up - runs before each test
}

void tearDown(void) {
    // Tear down - runs after each test
}

// Helper function to create a sensor
Sensor createSensor(int sensorId, SensorType type) {
    return Sensor(sensorId, "", type);
}

// Test mapping with POWER_PT1 sensor type
void test_mapper_power_pt1(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    
    Device device;
    device.sensors().push_back(createSensor(1, SensorType::POWER_PT1));
    
    std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValue(device, datagram);
    
    TEST_ASSERT_EQUAL_INT(1, sensorValues.size());
    TEST_ASSERT_EQUAL_INT(1, sensorValues[0].getSensorId());
    TEST_ASSERT_EQUAL_STRING("220601161852S", sensorValues[0].getTimestamp().c_str());
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1779.182, sensorValues[0].getValue());
}

// Test mapping with POWER_PT2 sensor type
void test_mapper_power_pt2(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    
    Device device;
    device.sensors().push_back(createSensor(2, SensorType::POWER_PT2));
    
    std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValue(device, datagram);
    
    TEST_ASSERT_EQUAL_INT(1, sensorValues.size());
    TEST_ASSERT_EQUAL_INT(2, sensorValues[0].getSensorId());
    TEST_ASSERT_EQUAL_STRING("220601161852S", sensorValues[0].getTimestamp().c_str());
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 2180.316, sensorValues[0].getValue());
}

// Test mapping with GAS_METER sensor type
void test_mapper_gas_meter(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    
    Device device;
    device.sensors().push_back(createSensor(3, SensorType::GAS_METER));
    
    std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValue(device, datagram);
    
    TEST_ASSERT_EQUAL_INT(1, sensorValues.size());
    TEST_ASSERT_EQUAL_INT(3, sensorValues[0].getSensorId());
    TEST_ASSERT_EQUAL_STRING("220601161852S", sensorValues[0].getTimestamp().c_str());
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1265.379, sensorValues[0].getValue());
}

// Test mapping with POWER_AP (Actual Power Produced) sensor type
void test_mapper_power_ap(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    
    Device device;
    device.sensors().push_back(createSensor(4, SensorType::POWER_AP));
    
    std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValue(device, datagram);
    
    TEST_ASSERT_EQUAL_INT(1, sensorValues.size());
    TEST_ASSERT_EQUAL_INT(4, sensorValues[0].getSensorId());
    TEST_ASSERT_EQUAL_STRING("220601161852S", sensorValues[0].getTimestamp().c_str());
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.490, sensorValues[0].getValue());
}

// Test mapping with POWER_AC (Actual Power Consumed) sensor type
void test_mapper_power_ac(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    
    Device device;
    device.sensors().push_back(createSensor(5, SensorType::POWER_AC));
    
    std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValue(device, datagram);
    
    TEST_ASSERT_EQUAL_INT(1, sensorValues.size());
    TEST_ASSERT_EQUAL_INT(5, sensorValues[0].getSensorId());
    TEST_ASSERT_EQUAL_STRING("220601161852S", sensorValues[0].getTimestamp().c_str());
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.000, sensorValues[0].getValue());
}

// Test mapping with POWER_CT1 sensor type
void test_mapper_power_ct1(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    
    Device device;
    device.sensors().push_back(createSensor(6, SensorType::POWER_CT1));
    
    std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValue(device, datagram);
    
    TEST_ASSERT_EQUAL_INT(1, sensorValues.size());
    TEST_ASSERT_EQUAL_INT(6, sensorValues[0].getSensorId());
    TEST_ASSERT_EQUAL_STRING("220601161852S", sensorValues[0].getTimestamp().c_str());
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1779.182, sensorValues[0].getValue());
}

// Test mapping with POWER_CT2 sensor type
void test_mapper_power_ct2(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    
    Device device;
    device.sensors().push_back(createSensor(7, SensorType::POWER_CT2));
    
    std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValue(device, datagram);
    
    TEST_ASSERT_EQUAL_INT(1, sensorValues.size());
    TEST_ASSERT_EQUAL_INT(7, sensorValues[0].getSensorId());
    TEST_ASSERT_EQUAL_STRING("220601161852S", sensorValues[0].getTimestamp().c_str());
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 2180.316, sensorValues[0].getValue());
}

// Test mapping with NOT_USED sensor type (should return 0.0)
void test_mapper_not_used(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    
    Device device;
    device.sensors().push_back(createSensor(8, SensorType::NOT_USED));
    
    std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValue(device, datagram);
    
    TEST_ASSERT_EQUAL_INT(1, sensorValues.size());
    TEST_ASSERT_EQUAL_INT(8, sensorValues[0].getSensorId());
    TEST_ASSERT_EQUAL_STRING("220601161852S", sensorValues[0].getTimestamp().c_str());
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.0, sensorValues[0].getValue());
}

// Test mapping with unsupported sensor type (should return 0.0)
void test_mapper_unsupported_type(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    
    Device device;
    device.sensors().push_back(createSensor(9, SensorType::TEMP));
    
    std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValue(device, datagram);
    
    TEST_ASSERT_EQUAL_INT(1, sensorValues.size());
    TEST_ASSERT_EQUAL_INT(9, sensorValues[0].getSensorId());
    TEST_ASSERT_EQUAL_STRING("220601161852S", sensorValues[0].getTimestamp().c_str());
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.0, sensorValues[0].getValue());
}

// Test mapping with multiple sensors of different types
void test_mapper_multiple_sensors(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    
    Device device;
    device.sensors().push_back(createSensor(1, SensorType::POWER_PT1));
    device.sensors().push_back(createSensor(2, SensorType::POWER_PT2));
    device.sensors().push_back(createSensor(3, SensorType::GAS_METER));
    device.sensors().push_back(createSensor(4, SensorType::POWER_AP));
    device.sensors().push_back(createSensor(5, SensorType::POWER_AC));
    
    std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValue(device, datagram);
    
    TEST_ASSERT_EQUAL_INT(5, sensorValues.size());
    
    // Verify POWER_PT1
    TEST_ASSERT_EQUAL_INT(1, sensorValues[0].getSensorId());
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1779.182, sensorValues[0].getValue());
    TEST_ASSERT_EQUAL_STRING("220601161852S", sensorValues[0].getTimestamp().c_str());
    
    // Verify POWER_PT2
    TEST_ASSERT_EQUAL_INT(2, sensorValues[1].getSensorId());
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 2180.316, sensorValues[1].getValue());
    TEST_ASSERT_EQUAL_STRING("220601161852S", sensorValues[1].getTimestamp().c_str());
    
    // Verify GAS_METER
    TEST_ASSERT_EQUAL_INT(3, sensorValues[2].getSensorId());
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1265.379, sensorValues[2].getValue());
    TEST_ASSERT_EQUAL_STRING("220601161852S", sensorValues[2].getTimestamp().c_str());
    
    // Verify POWER_AP
    TEST_ASSERT_EQUAL_INT(4, sensorValues[3].getSensorId());
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.490, sensorValues[3].getValue());
    TEST_ASSERT_EQUAL_STRING("220601161852S", sensorValues[3].getTimestamp().c_str());
    
    // Verify POWER_AC
    TEST_ASSERT_EQUAL_INT(5, sensorValues[4].getSensorId());
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.000, sensorValues[4].getValue());
    TEST_ASSERT_EQUAL_STRING("220601161852S", sensorValues[4].getTimestamp().c_str());
}

// Test mapping with empty device (no sensors)
void test_mapper_empty_device(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    
    Device device;
    
    std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValue(device, datagram);
    
    TEST_ASSERT_EQUAL_INT(0, sensorValues.size());
}

// Test mapping with all supported sensor types
void test_mapper_all_supported_types(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    
    Device device;
    device.sensors().push_back(createSensor(1, SensorType::POWER_PT1));
    device.sensors().push_back(createSensor(2, SensorType::POWER_PT2));
    device.sensors().push_back(createSensor(3, SensorType::POWER_CT1));
    device.sensors().push_back(createSensor(4, SensorType::POWER_CT2));
    device.sensors().push_back(createSensor(5, SensorType::POWER_AP));
    device.sensors().push_back(createSensor(6, SensorType::POWER_AC));
    device.sensors().push_back(createSensor(7, SensorType::GAS_METER));
    
    std::vector<SensorValue> sensorValues = P1DatagramSensorValueMapper::mapToSensorValue(device, datagram);
    
    TEST_ASSERT_EQUAL_INT(7, sensorValues.size());
    
    // Verify all values are mapped correctly
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1779.182, sensorValues[0].getValue()); // POWER_PT1
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 2180.316, sensorValues[1].getValue()); // POWER_PT2
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1779.182, sensorValues[2].getValue()); // POWER_CT1
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 2180.316, sensorValues[3].getValue()); // POWER_CT2
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.490, sensorValues[4].getValue());    // POWER_AP
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.000, sensorValues[5].getValue());    // POWER_AC
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1265.379, sensorValues[6].getValue()); // GAS_METER
    
    // Verify all have the same timestamp
    for (const auto& sv : sensorValues) {
        TEST_ASSERT_EQUAL_STRING("220601161852S", sv.getTimestamp().c_str());
    }
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_mapper_power_pt1);
    RUN_TEST(test_mapper_power_pt2);
    RUN_TEST(test_mapper_gas_meter);
    RUN_TEST(test_mapper_power_ap);
    RUN_TEST(test_mapper_power_ac);
    RUN_TEST(test_mapper_power_ct1);
    RUN_TEST(test_mapper_power_ct2);
    RUN_TEST(test_mapper_not_used);
    RUN_TEST(test_mapper_unsupported_type);
    RUN_TEST(test_mapper_multiple_sensors);
    RUN_TEST(test_mapper_empty_device);
    RUN_TEST(test_mapper_all_supported_types);
    
    return UNITY_END();
}
