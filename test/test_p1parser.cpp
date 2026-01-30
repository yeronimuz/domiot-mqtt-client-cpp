#include <unity.h>
#include <Arduino.h>
#include <P1Parser.h>
#include <P1Datagram.h>
#include <FS.h>
#include <LittleFS.h>

// Test data - the p1.datagram content
const char* TEST_P1_DATAGRAM = "/ISK5\\2M550T-1013\r\n"
    "1-3:0.2.8(50)\r\n"
    "0-0:1.0.0(220601161852S)\r\n"
    "0-0:96.1.1(4530303534303037363939333832353230)\r\n"
    "1-0:1.8.1(001779.182*kWh)\r\n"
    "1-0:1.8.2(002180.316*kWh)\r\n"
    "1-0:2.8.1(000967.320*kWh)\r\n"
    "1-0:2.8.2(001994.968*kWh)\r\n"
    "0-0:96.14.0(0002)\r\n"
    "1-0:1.7.0(00.000*kW)\r\n"
    "1-0:2.7.0(00.490*kW)\r\n"
    "0-0:96.7.21(00005)\r\n"
    "0-0:96.7.9(00004)\r\n"
    "1-0:99.97.0(2)(0-0:96.7.19)(211109105421W)(0000004239*s)(220519205327S)(0000011498*s)\r\n"
    "1-0:32.32.0(00002)\r\n"
    "1-0:52.32.0(00003)\r\n"
    "1-0:72.32.0(00001)\r\n"
    "1-0:32.36.0(00001)\r\n"
    "1-0:52.36.0(00001)\r\n"
    "1-0:72.36.0(00001)\r\n"
    "0-0:96.13.0()\r\n"
    "1-0:32.7.0(231.1*V)\r\n"
    "1-0:52.7.0(229.5*V)\r\n"
    "1-0:72.7.0(231.3*V)\r\n"
    "1-0:31.7.0(000*A)\r\n"
    "1-0:51.7.0(001*A)\r\n"
    "1-0:71.7.0(003*A)\r\n"
    "1-0:21.7.0(00.000*kW)\r\n"
    "1-0:41.7.0(00.266*kW)\r\n"
    "1-0:61.7.0(00.000*kW)\r\n"
    "1-0:22.7.0(00.000*kW)\r\n"
    "1-0:42.7.0(00.000*kW)\r\n"
    "1-0:62.7.0(00.757*kW)\r\n"
    "0-1:24.1.0(003)\r\n"
    "0-1:96.1.0(4730303738353635353936353235323230)\r\n"
    "0-1:24.2.1(220601161501S)(01265.379*m3)\r\n"
    "!291A\r\n";

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
    TEST_ASSERT_EQUAL_INT(50, datagram.versionInfo);
}

// Test version info parsing
void test_parse_version_info(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_INT(50, datagram.versionInfo);
}

// Test timestamp parsing
void test_parse_timestamp(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_STRING("220601161852S", datagram.timestamp.c_str());
}

// Test equipment ID parsing
void test_parse_equipment_id(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_STRING("4730303738353635353936353235323230", datagram.equipmentId.c_str());
}

// Test consumed power tariff 1
void test_parse_consumed_power_tariff1(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1779.182, datagram.consumedPowerT1);
}

// Test consumed power tariff 2
void test_parse_consumed_power_tariff2(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 2180.316, datagram.consumedPowerT2);
}

// Test produced power tariff 1
void test_parse_produced_power_tariff1(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 967.320, datagram.producedPowerT1);
}

// Test produced power tariff 2
void test_parse_produced_power_tariff2(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1994.968, datagram.producedPowerT2);
}

// Test current tariff
void test_parse_current_tariff(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_INT(2, datagram.currentTariff);
}

// Test actual power consumed
void test_parse_actual_power_consumed(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.000, datagram.actualPowerConsumed);
}

// Test actual power produced
void test_parse_actual_power_produced(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.490, datagram.actualPowerProduced);
}

// Test power fails any phase
void test_parse_power_fails_any_phase(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_INT(5, datagram.powerFailsAnyPhase);
}

// Test long power fails any phase
void test_parse_long_power_fails_any_phase(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_INT(4, datagram.longPowerFailsAnyPhase);
}

// Test voltage sags in phase L1
void test_parse_voltage_sags_phase_l1(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_INT(2, datagram.voltageSagsInPhaseL1);
}

// Test voltage sags in phase L2
void test_parse_voltage_sags_phase_l2(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_INT(1, datagram.voltageSagsInPhaseL2);
}

// Test device type (gas meter)
void test_parse_device_type(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_EQUAL_INT(3, datagram.deviceType);
}

// Test consumed gas
void test_parse_consumed_gas(void) {
    P1Datagram datagram = P1Parser::parse(String(TEST_P1_DATAGRAM));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1265.379, datagram.consumedGas);
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
