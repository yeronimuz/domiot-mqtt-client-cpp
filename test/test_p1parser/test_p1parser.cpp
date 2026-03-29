#include <unity.h>
#include <Arduino.h>
#include <P1Parser.h>
#include <P1Datagram.h>
#include <FS.h>
#include <LittleFS.h>
#include "TestP1Datagram.h"

void setUp(void) {
    // Set up - runs before each test
}

void tearDown(void) {
    // Tear down - runs after each test
}

// Test basic parsing
void test_parse_returns_valid_datagram(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    
    // Check that basic fields are parsed
    TEST_ASSERT_EQUAL_INT(50, datagram.getVersionInfo());
}

// Test version info parsing
void test_parse_version_info(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_INT(50, datagram.getVersionInfo());
}

// Test timestamp parsing
void test_parse_timestamp(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_STRING("220601161852S", datagram.getTimestamp().c_str());
}

// Test equipment ID parsing
void test_parse_equipment_id(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_STRING("4730303738353635353936353235323230", datagram.getEquipmentId().c_str());
}

// Test consumed power tariff 1
void test_parse_consumed_power_tariff1(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1779.182, datagram.getConsumedPowerT1());
}

// Test consumed power tariff 2
void test_parse_consumed_power_tariff2(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 2180.316, datagram.getConsumedPowerT2());
}

// Test produced power tariff 1
void test_parse_produced_power_tariff1(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 967.320, datagram.getProducedPowerT1());
}

// Test produced power tariff 2
void test_parse_produced_power_tariff2(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1994.968, datagram.getProducedPowerT2());
}

// Test current tariff
void test_parse_current_tariff(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_INT(2, datagram.getCurrentTariff());
}

// Test actual power consumed
void test_parse_actual_power_consumed(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.000, datagram.getActualPowerConsumed());
}

// Test actual power produced
void test_parse_actual_power_produced(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.490, datagram.getActualPowerProduced());
}

// Test power fails any phase
void test_parse_power_fails_any_phase(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_INT(5, datagram.getPowerFailsAnyPhase());
}

// Test long power fails any phase
void test_parse_long_power_fails_any_phase(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_INT(4, datagram.getLongPowerFailsAnyPhase());
}

// Test voltage sags in phase L1
void test_parse_voltage_sags_phase_l1(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_INT(2, datagram.getVoltageSagsInPhaseL1());
}

// Test voltage sags in phase L2
void test_parse_voltage_sags_phase_l2(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_INT(1, datagram.getVoltageSagsInPhaseL2());
}

// Test device type (gas meter)
void test_parse_device_type(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_INT(3, datagram.getDeviceType());
}

// Test consumed gas
void test_parse_consumed_gas(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1265.379, datagram.getConsumedGas());
}

// Test empty datagram handling
void test_parse_empty_string(void) {
    P1Datagram datagram = P1Parser::parse(String(""));
    // Should not crash, just return empty datagram
    TEST_PASS();
}

// Test datagram with minimal content
void test_parse_minimal_datagram(void) {
    const char* minimalDatagram = "/ISK5\\2M550T-1013\r\n!291A\r\n";
    P1Datagram datagram = P1Parser::parse(String(minimalDatagram));
    TEST_PASS();
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_parse_returns_valid_datagram);
    RUN_TEST(test_parse_version_info);
    RUN_TEST(test_parse_timestamp);
    RUN_TEST(test_parse_equipment_id);
    RUN_TEST(test_parse_consumed_power_tariff1);
    RUN_TEST(test_parse_consumed_power_tariff2);
    RUN_TEST(test_parse_produced_power_tariff1);
    RUN_TEST(test_parse_produced_power_tariff2);
    RUN_TEST(test_parse_current_tariff);
    RUN_TEST(test_parse_actual_power_consumed);
    RUN_TEST(test_parse_actual_power_produced);
    RUN_TEST(test_parse_power_fails_any_phase);
    RUN_TEST(test_parse_long_power_fails_any_phase);
    RUN_TEST(test_parse_voltage_sags_phase_l1);
    RUN_TEST(test_parse_voltage_sags_phase_l2);
    RUN_TEST(test_parse_device_type);
    RUN_TEST(test_parse_consumed_gas);
    RUN_TEST(test_parse_empty_string);
    RUN_TEST(test_parse_minimal_datagram);
    
    return UNITY_END();
}
