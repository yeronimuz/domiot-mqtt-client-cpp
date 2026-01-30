#pragma once
#include <vector>
#include <optional>
#include <Arduino.h>

enum P1Type {
    VERSION_INFO = 0,
    DATE_TIMESTAMP = 1,
    EQUIPMENT_ID_01 = 2,
    EQUIPMENT_ID_00 = 3,
    CONS_PWR_TARIFF_1 = 4,
    CONS_PWR_TARIFF_2 = 5,
    PROD_PWR_TARIFF_1 = 6,
    PROD_PWR_TARIFF_2 = 7,
    CURRENT_TARIFF = 8,
    ACT_PWR_CONSUMED = 9,
    ACT_PWR_PRODUCED = 10,
    PWR_FAILS_ANY_PHASE = 11,
    LONG_PWR_FAILS_ANY_PHASE = 12,
    PWR_FAIL_EVENT_LOG = 13,
    VOLTAGE_SAGS_IN_PHASE_L1 = 14,
    VOLTAGE_SAGS_IN_PHASE_L2 = 15,
    TXT_MSG_CODES = 16,
    TXT_MSG = 17,
    INSTANTANEOUS_CURRENT_L1 = 18,
    INSTANTANEOUS_ACTIVE_POWER_L1_PLUS_P = 19,
    INSTANTANEOUS_ACTIVE_POWER_L1_MIN_P = 20,
    DEVICE_TYPE = 21,
    CONSUMED_GAS = 22
};

class P1Standard {
public:
    const P1Type id;
    const String tag;
    const String description;

    P1Standard(P1Type id, String tag, String desc) : id(id), tag(std::move(tag)), description(std::move(desc)) {}

    const P1Type getId() const {
        return id;
    }

    const String& getTag() const {
        return tag;
    }

    const String& getDescription() const {
        return description;
    }

    static std::optional<P1Standard> getByTag(const String& tag) {
        for (const auto& p1 : values()) {
            if (p1.getTag() == tag) {
                return p1;
            }
        }
        return std::nullopt;
    }

    static const std::vector<P1Standard>& values() {
        static const std::vector<P1Standard> instances = {
            P1Standard(VERSION_INFO, "1-3:0.2.8", "VERSION_INFO"),
            P1Standard(DATE_TIMESTAMP, "0-0:1.0.0", "Date-time stamp of the P1 message"),
            P1Standard(EQUIPMENT_ID_01, "0-1:96.1.0", "Equipment identifier"),
            P1Standard(EQUIPMENT_ID_00, "0-0:96.1.1", "Equipment identifier"),
            P1Standard(CONS_PWR_TARIFF_1, "1-0:1.8.1", "consumed power T1"),
            P1Standard(CONS_PWR_TARIFF_2, "1-0:1.8.2", "consumed power T2"),
            P1Standard(PROD_PWR_TARIFF_1, "1-0:2.8.1", "produced power T1"),
            P1Standard(PROD_PWR_TARIFF_2, "1-0:2.8.2", "produced power T2"),
            P1Standard(CURRENT_TARIFF, "0-0:96.14.0", ""),
            P1Standard(ACT_PWR_CONSUMED, "1-0:1.7.0", ""),
            P1Standard(ACT_PWR_PRODUCED, "1-0:2.7.0", ""),
            P1Standard(PWR_FAILS_ANY_PHASE, "0-0:96.7.21", ""),
            P1Standard(LONG_PWR_FAILS_ANY_PHASE, "0-0:96.7.9", ""),
            P1Standard(PWR_FAIL_EVENT_LOG, "1-0:99.97.0", ""),
            P1Standard(VOLTAGE_SAGS_IN_PHASE_L1, "1-0:32.32.0", ""),
            P1Standard(VOLTAGE_SAGS_IN_PHASE_L2, "1-0:32.36.0", ""),
            P1Standard(TXT_MSG_CODES, "0-0:96.13.1", ""),
            P1Standard(TXT_MSG, "0-0:96.13.0", ""),
            P1Standard(INSTANTANEOUS_CURRENT_L1, "1-0:31.7.0", ""),
            P1Standard(INSTANTANEOUS_ACTIVE_POWER_L1_PLUS_P, "1-0:21.7.0", ""),
            P1Standard(INSTANTANEOUS_ACTIVE_POWER_L1_MIN_P, "1-0:22.7.0", ""),
            P1Standard(DEVICE_TYPE, "0-1:24.1.0", ""),
            P1Standard(CONSUMED_GAS, "0-1:24.2.1", "consumed gas")
        };
        return instances;
    }
};