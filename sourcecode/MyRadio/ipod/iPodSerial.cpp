/*******************************************************************************
 * Copyright (c) 2009 David Findlay
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    - Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
#include "iPodSerial.h"
#include "util/printf.h"
#include <ctype.h>

//#define DEBUG

#ifdef DEBUG
static const char *STATE_NAME[] =
{
    "Waiting for Header 1",
    "Waiting for Header 2",
    "Waiting for length",
    "Waiting for data",
    "Waiting for checksum"
};
#endif

iPodSerial::iPodSerial()
    : receiveState(WAITING_FOR_HEADER1),
      dataSize(0),
      pData(0),
      checksum(0),
      pSerial(&Serial) // default to regular serial port as that's all most Arduinos have
{
}

void iPodSerial::setSerial(SerialPort &newiPodSerial)
{
    pSerial = &newiPodSerial;
}

void iPodSerial::setup()
{
	pSerial->end();
    pSerial->begin(IPOD_SERIAL_RATE);
}

bool iPodSerial::validChecksum(byte actual)
{
    int expected = dataSize;
    for (int i = 0; i < dataSize; ++i)
    {
        expected += dataBuffer[i];
    }

    expected = (0x100 - expected) & 0xFF;

    if (expected == actual)
    {
        return true;
    }
    else
    {
#ifdef DEBUG
    	if (pDebugPrint)
        {
            pDebugPrint->print("checksum mismatch: expected ");
            pDebugPrint->print(expected, HEX);
            pDebugPrint->print(" but got ");
            pDebugPrint->println(actual, HEX);
        }
#endif

        return false;
    }
}

void iPodSerial::dumpReceive()
{
    dprintf("data size = %d\n\r,dataSize");
    ddump("dataBuffer",dataBuffer,dataSize);
}

void iPodSerial::processResponse()
{
    const int avail = pSerial->available();
    if (avail <= 0)
    {
        return;
    }

    // read a single byte from the iPod
    const int b = pSerial->read();

#ifdef DEBUG
        dprintf("Receive Status: %s %02x %c\n\r",
        		STATE_NAME[receiveState],b,isprint(b) ? b : '.');
#endif

    switch (receiveState)
    {
    case WAITING_FOR_HEADER1:
        if (b == HEADER1)
        {
            receiveState = WAITING_FOR_HEADER2;
        }
        break;

    case WAITING_FOR_HEADER2:
        if (b == HEADER2)
        {
            receiveState = WAITING_FOR_LENGTH;
        } else
        {
        	receiveState = WAITING_FOR_HEADER1;
        }
        break;

    case WAITING_FOR_LENGTH:
        dataSize = b;
        pData = dataBuffer;
        receiveState = WAITING_FOR_DATA;
#ifdef DEBUG
        if (pDebugPrint)
        {
            pDebugPrint->print("Data length is ");
            pDebugPrint->println(dataSize, DEC);
        }

        if(dataSize>127 || dataSize<0) {
        	dmsg("ipod cmd length too big, ignoring");
            receiveState = WAITING_FOR_HEADER1;
        }
#endif
        break;

    case WAITING_FOR_DATA:
        *pData++ = b;

        if ((pData - dataBuffer) == dataSize)
        {
            receiveState = WAITING_FOR_CHECKSUM;
        }
        else
        {
#ifdef DEBUG
            dprintf("Waiting for %d\n\r", dataSize - (pData - dataBuffer));
#endif
        }
        break;

    case WAITING_FOR_CHECKSUM:
        if (validChecksum(b))
        {
            processData();
        } else {
        	dmsg("invalid checksum in received ipod command");
        }
        receiveState = WAITING_FOR_HEADER1;
        memset(dataBuffer, 0, sizeof(dataBuffer));
        break;
    }
}

void iPodSerial::sendCommandWithLength(
    size_t length,
    const byte *pData)
{

//#ifdef DEBUG
//        dprintf("Sending to ipod mode %d cmd1 0x%x cmd2 %x of length %d\n\r",pData[0],pData[1],pData[2],length);
//#endif

    sendHeader();
    sendLength(length);
    sendBytes(length, pData);
    sendChecksum();
}

void iPodSerial::sendCommand(
    byte mode,
    byte cmdByte1)
{
    sendHeader();
    sendLength(1 + 1);
    sendByte(mode);
    sendByte(cmdByte1);
    sendChecksum();
}

void iPodSerial::sendCommand(
    byte mode,
    byte cmdByte1,
    byte cmdByte2)
{
    sendHeader();
    sendLength(1 + 1 + 1);
    sendByte(mode);
    sendByte(cmdByte1);
    sendByte(cmdByte2);
    sendChecksum();
}

void iPodSerial::sendCommandWithOneByteParam(
    byte mode,
    byte cmdByte1,
    byte cmdByte2,
    byte byteParam)
{
    sendHeader();
    sendLength(1 + 1 + 1 + 1);
    sendByte(mode);
    sendByte(cmdByte1);
    sendByte(cmdByte2);
    sendByte(byteParam);
    sendChecksum();
}

void iPodSerial::sendCommandWithOneNumberParam(
    byte mode,
    byte cmdByte1,
    byte cmdByte2,
    unsigned long numberParam)
{
    sendHeader();
    sendLength(1 + 1 + 1 + 4);
    sendByte(mode);
    sendByte(cmdByte1);
    sendByte(cmdByte2);
    sendParam(numberParam);
    sendChecksum();
}

void iPodSerial::sendCommandWithOneByteAndOneNumberParam(
    byte mode,
    byte cmdByte1,
    byte cmdByte2,
    byte byteParam1,
    unsigned long numberParam2)
{
    sendHeader();
    sendLength(1 + 1 + 1 + 1 + (1 * 4));
    sendByte(mode);
    sendByte(cmdByte1);
    sendByte(cmdByte2);
    sendByte(byteParam1);
    sendParam(numberParam2);
    sendChecksum();
}

void iPodSerial::sendCommandWithOneByteAndTwoNumberParams(
    byte mode,
    byte cmdByte1,
    byte cmdByte2,
    byte byteParam1,
    unsigned long numberParam2,
    unsigned long numberParam3)
{
    sendHeader();
    sendLength(1 + 1 + 1 + 1 + (2 * 4));
    sendByte(mode);
    sendByte(cmdByte1);
    sendByte(cmdByte2);
    sendByte(byteParam1);
    sendParam(numberParam2);
    sendParam(numberParam3);
    sendChecksum();
}

void iPodSerial::sendHeader()
{
    sendByte(HEADER1);
    sendByte(HEADER2);
}

void iPodSerial::sendLength(size_t length) // length is mode+command+parameters in bytes
{
    sendByte(length);
    checksum = length;
}

void iPodSerial::sendBytes(size_t length, const byte *pData)
{
    for (size_t i = 0; i < length; i++)
    {
        sendByte(pData[i]);
    }
}

void iPodSerial::sendByte(byte b)
{
    pSerial->print(b, BYTE);
    checksum += b;

//    delay(1);

#ifdef DEBUG
    // likely to slow stuff down!
    if (pDebugPrint)
    {
        pDebugPrint->print("sent byte ");
        pDebugPrint->println(b, HEX);
    }
#endif

}

void iPodSerial::sendParam(unsigned long param)
{
    // parameter (4-byte int sent big-endian)
    sendByte((param & 0xFF000000) >> 24);
    sendByte((param & 0x00FF0000) >> 16);
    sendByte((param & 0x0000FF00) >> 8);
    sendByte((param & 0x000000FF) >> 0);
}

void iPodSerial::sendChecksum()
{
    sendByte((0x100 - checksum) & 0xFF);
}

void iPodSerial::loop()
{
    while (pSerial->available() > 0)
    {
        processResponse();
    }
}

void iPodSerial::processData()
{
#ifdef DEBUG
        dmsg("Ignoring data from iPod...");
        dumpReceive();
#endif
}

void iPodSerial::log(const char *message)
{
    dmsg(message);
}
