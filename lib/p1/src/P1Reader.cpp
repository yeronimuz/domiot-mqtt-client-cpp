#include "P1Reader.h"
#include "P1Parser.h"

#define MAXLINELENGTH 128 // longest normal line is 47 char (+3 for \r\n\0)

P1Datagram P1Reader::readDatagram(SoftwareSerial &serial)
{
    static String p1Message;
    static bool readingDatagram = false;
    static unsigned long stateChangeTime = 0;
    char telegram[MAXLINELENGTH + 1];
    unsigned long now = millis();

    // Reset state if stuck for > 30 seconds (allows meter restart/reconnect).
    if (readingDatagram && (now - stateChangeTime > 30000))
    {
        Serial.println("P1Reader: State timeout, resetting...");
        readingDatagram = false;
        p1Message = "";
    }

    // Keep reading as long as bytes are available and emit exactly one completed telegram.
    while (serial.available() > 0)
    {
        int len = serial.readBytesUntil('\n', telegram, MAXLINELENGTH - 1);
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
            if (!readingDatagram)
            {
                Serial.println("P1Reader: Telegram start detected.");
                stateChangeTime = now;
            }
            p1Message = "";
            readingDatagram = true;
        }

        if (!readingDatagram)
        {
            yield();
            continue;
        }

        p1Message += telegram;
        p1Message += '\n';

        if (telegram[0] == '!')
        {
            Serial.println("P1Reader: Telegram complete.");
            readingDatagram = false;
            P1Datagram datagram = P1Parser::parse(p1Message);
            return datagram;
        }

        yield();
    }

    return P1Datagram();
}