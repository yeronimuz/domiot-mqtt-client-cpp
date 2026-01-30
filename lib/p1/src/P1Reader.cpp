#include "P1Reader.h"
#include "P1Parser.h"

#define MAXLINELENGTH 128 // longest normal line is 47 char (+3 for \r\n\0)

P1Datagram P1Reader::readDatagram(SoftwareSerial &serial)
{
    char telegram[MAXLINELENGTH];
    String p1Message;

    // Read until '!' character
    while (serial.available())
    {
        int len = serial.readBytesUntil('\n', telegram, MAXLINELENGTH);
        telegram[len] = '\n';
        telegram[len+1] = 0;
        yield();

        p1Message += telegram;
        if (telegram[0] == '!')
        {
            P1Datagram datagram = P1Parser::parse(p1Message);
            return datagram;
        }
    }
    return P1Datagram();
}