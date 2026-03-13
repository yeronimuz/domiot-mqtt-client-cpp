#include "P1Parser.h"
#include <string>

P1Datagram P1Parser::parse(const String &p1Message)
{
    P1Datagram datagram;
    std::string message(p1Message.c_str());

    // Split the message into lines
    std::regex lineRegex("\r\n|\n");
    std::sregex_token_iterator lineIter(message.begin(), message.end(), lineRegex, -1);
    std::sregex_token_iterator endIter;

    for (; lineIter != endIter; ++lineIter)
    {
        String line = lineIter->str().c_str();
        // Process each line based on its tag
        auto p1StandardOpt = P1Standard::getByTag(line.substring(0, line.indexOf('(')));
        if (p1StandardOpt)
        {
            P1Standard p1Standard = *p1StandardOpt;
            String data;
            
            // For CONSUMED_GAS, extract value from second pair of parentheses
            if (p1Standard.getId() == CONSUMED_GAS) {
                int firstClose = line.indexOf(')');
                int secondOpen = line.indexOf('(', firstClose);
                if (secondOpen != -1) {
                    data = line.substring(secondOpen + 1, line.lastIndexOf(')'));
                }
            } else {
                data = line.substring(line.indexOf('(') + 1, line.lastIndexOf(')'));
            }

            switch (p1Standard.getId())
            {
            case VERSION_INFO:
                datagram.setVersionInfo(static_cast<byte>(data.toInt()));
                break;
            case DATE_TIMESTAMP:
                datagram.setTimestamp(data);
                break;
            case EQUIPMENT_ID_01:
            case EQUIPMENT_ID_00:
                datagram.setEquipmentId(data);
                break;
            case CONS_PWR_TARIFF_1:
                datagram.setConsumedPowerT1(data.toDouble());
                break;
            case CONS_PWR_TARIFF_2:
                datagram.setConsumedPowerT2(data.toDouble());
                break;
            case PROD_PWR_TARIFF_1:
                datagram.setProducedPowerT1(data.toDouble());
                break;
            case PROD_PWR_TARIFF_2:
                datagram.setProducedPowerT2(data.toDouble());
                break;
            case CURRENT_TARIFF:
                datagram.setCurrentTariff(static_cast<byte>(data.toInt()));
                break;
            case ACT_PWR_CONSUMED:
                datagram.setActualPowerConsumed(data.toDouble());
                break;
            case ACT_PWR_PRODUCED:
                datagram.setActualPowerProduced(data.toDouble());
                break;
            case PWR_FAILS_ANY_PHASE:
                datagram.setPowerFailsAnyPhase(static_cast<byte>(data.toInt()));
                break;
            case LONG_PWR_FAILS_ANY_PHASE:
                datagram.setLongPowerFailsAnyPhase(static_cast<byte>(data.toInt()));
                break;
            case PWR_FAIL_EVENT_LOG:
                datagram.setPowerFailEventLog(data);
                break;
            case VOLTAGE_SAGS_IN_PHASE_L1:
                datagram.setVoltageSagsInPhaseL1(static_cast<uint16_t>(data.toInt()));
                break;
            case VOLTAGE_SAGS_IN_PHASE_L2:
                datagram.setVoltageSagsInPhaseL2(static_cast<uint16_t>(data.toInt()));
                break;
            case TXT_MSG_CODES:
                break;
            case TXT_MSG:
                datagram.setTextMessages(data);
                break;
            case INSTANTANEOUS_CURRENT_L1:
                datagram.setInstantaneousCurrentL1(data.toDouble());
                break;
            case INSTANTANEOUS_ACTIVE_POWER_L1_PLUS_P:
                datagram.setInstantaneousActivePowerL1PlusP(data.toDouble());
                break;
            case INSTANTANEOUS_ACTIVE_POWER_L1_MIN_P:
                datagram.setInstantaneousActivePowerL1MinP(data.toDouble());
                break;
            case DEVICE_TYPE:
                datagram.setDeviceType(static_cast<byte>(data.toInt()));
                break;
            case CONSUMED_GAS:
                datagram.setConsumedGas(data.toDouble());
                break;
            default:
                // Unknown tag, ignore
                break;
            }
        }
    }
    return datagram;
}