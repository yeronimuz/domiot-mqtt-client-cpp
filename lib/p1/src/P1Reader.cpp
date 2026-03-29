#include "P1Reader.h"
#include "P1Parser.h"

namespace
{
constexpr unsigned long P1_READER_IDLE_TIMEOUT_MS = 30000;
constexpr size_t P1_READER_MAX_DATAGRAM_LENGTH = 2048;
}

P1Datagram P1Reader::readDatagram(Stream &serial)
{
    static String p1Message;
    static bool readingDatagram = false;
    static unsigned long lastByteTime = 0;
    static char telegram[MAXLINELENGTH];
    static size_t telegramLen = 0;
    static bool droppingOverlongLine = false;

    const unsigned long now = millis();

    // Reset state if stuck for > 30 seconds (allows meter restart/reconnect).
    if (readingDatagram && (now - lastByteTime > P1_READER_IDLE_TIMEOUT_MS))
    {
        Serial.println("P1Reader: State timeout, resetting...");
        readingDatagram = false;
        p1Message = "";
        telegramLen = 0;
        droppingOverlongLine = false;
    }

    // Keep reading as long as bytes are available and emit exactly one completed telegram.
    while (serial.available() > 0)
    {
        int raw = serial.read();
        if (raw < 0)
        {
            yield();
            continue;
        }

        char ch = static_cast<char>(raw);
        lastByteTime = millis();

        // Process line when receiving CR or LF (handles CRLF and LF-only meters).
        if (ch == '\r' || ch == '\n')
        {
            if (droppingOverlongLine)
            {
                // Discard bytes until end-of-line, then resume normal parsing.
                droppingOverlongLine = false;
                telegramLen = 0;
                continue;
            }

            if (telegramLen == 0)
            {
                continue;
            }

            telegram[telegramLen] = 0;
            telegramLen = 0;

            // Synchronize to telegram start marker.
            if (telegram[0] == '/')
            {
                if (!readingDatagram)
                {
                    Serial.println("P1Reader: Telegram start detected.");
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

            if (p1Message.length() > P1_READER_MAX_DATAGRAM_LENGTH)
            {
                Serial.println("P1Reader: Datagram too long, dropping...");
                readingDatagram = false;
                p1Message = "";
                yield();
                continue;
            }

            if (telegram[0] == '!')
            {
                Serial.println("P1Reader: Telegram complete.");
                readingDatagram = false;
                P1Datagram datagram = P1Parser::parse(p1Message);
                p1Message = "";
                return datagram;
            }

            yield();
            continue;
        }

        if (telegramLen < (MAXLINELENGTH - 1))
        {
            telegram[telegramLen++] = ch;
        }
        else
        {
            // Keep current datagram alive and skip only this oversized line.
            if (!droppingOverlongLine)
            {
                Serial.println("P1Reader: Line overflow, skipping long line...");
            }
            droppingOverlongLine = true;
            yield();
        }
    }

    return P1Datagram();
}