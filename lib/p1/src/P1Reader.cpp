#include "P1Reader.h"
#include "P1Parser.h"

#define MAXLINELENGTH 128 // longest normal line is 47 char (+3 for \r\n\0)

namespace
{
constexpr unsigned long P1_READER_IDLE_TIMEOUT_MS = 30000;
}

P1Datagram P1Reader::readDatagram(SoftwareSerial &serial)
{
    static String p1Message;
    static bool readingDatagram = false;
    static unsigned long lastByteTime = 0;
    static unsigned int telegramLineCount = 0;
    char telegram[MAXLINELENGTH + 1];
    const unsigned long now = millis();

    // Reset state if stuck for > 30 seconds (allows meter restart/reconnect).
    if (readingDatagram && (now - lastByteTime > P1_READER_IDLE_TIMEOUT_MS))
    {
        Serial.println("P1: idle timeout, resetting");
        readingDatagram = false;
        p1Message = "";
        telegramLineCount = 0;
    }

    // Keep reading as long as bytes are available and emit exactly one completed telegram.
    while (serial.available() > 0)
    {
        int len = serial.readBytesUntil('\n', telegram, MAXLINELENGTH);
        if (len <= 0)
        {
            yield();
            continue;
        }

        telegram[len] = 0;
        if (telegram[len - 1] == '\r')
        {
            telegram[len - 1] = 0;
        }

        // Synchronize to telegram start marker.
        if (telegram[0] == '/')
        {
            if (readingDatagram && p1Message.length() > 0)
            {
                Serial.println("P1: restarting incomplete telegram");
            }

            p1Message = "";
            readingDatagram = true;
            lastByteTime = millis();
            telegramLineCount = 0;
            Serial.println("P1: telegram start");
        }

        if (!readingDatagram)
        {
            yield();
            continue;
        }

        if (len == MAXLINELENGTH)
        {
            Serial.println("P1: line buffer overflow");
        }

        p1Message += telegram;
        p1Message += '\n';
        telegramLineCount++;
        lastByteTime = millis();

        if (telegram[0] == '!')
        {
            Serial.print("P1: telegram complete, lines=");
            Serial.println(telegramLineCount);
            readingDatagram = false;
            telegramLineCount = 0;
            P1Datagram datagram = P1Parser::parse(p1Message);
            return datagram;
        }
    }

    yield();
    return P1Datagram();
}