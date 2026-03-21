#include "P1Parser.h"
#include <string>

namespace
{
String extractTag(const String &line)
{
    int openParenIndex = line.indexOf('(');
    if (openParenIndex < 0)
    {
        return line;
    }

    return line.substring(0, openParenIndex);
}

String extractData(const P1Standard &p1Standard, const String &line)
{
    if (p1Standard.getId() == CONSUMED_GAS)
    {
        int firstClose = line.indexOf(')');
        int secondOpen = line.indexOf('(', firstClose);
        if (secondOpen != -1)
        {
            return line.substring(secondOpen + 1, line.lastIndexOf(')'));
        }
        return "";
    }

    int openParenIndex = line.indexOf('(');
    int closeParenIndex = line.lastIndexOf(')');
    if (openParenIndex < 0 || closeParenIndex <= openParenIndex)
    {
        return "";
    }

    return line.substring(openParenIndex + 1, closeParenIndex);
}
}

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
        line.trim();
        if (line.length() == 0 || line[0] == '/' || line[0] == '!')
        {
            continue;
        }

        // Process each line based on its tag
        String tag = extractTag(line);
        auto p1StandardOpt = P1Standard::getByTag(tag);
        if (p1StandardOpt)
        {
            P1Standard p1Standard = *p1StandardOpt;
            String data = extractData(p1Standard, line);

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