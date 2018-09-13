//.'
/*
  HardwareSerial.h - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef HardwareSerial_h
#define HardwareSerial_h

#include <inttypes.h>

#include "SerialPort.h"

struct rx_ring;
struct tx_ring;

class HardwareSerial : public SerialPort
{
  private:
    rx_ring *_rx_buffer;
    tx_ring *_tx_buffer;
    volatile uint8_t *_ubrrh;
    volatile uint8_t *_ubrrl;
    volatile uint8_t *_ucsra;
    volatile uint8_t *_ucsrb;
    volatile uint8_t *_ucsrc;
    volatile uint8_t *_udr;
    uint8_t _rxen;
    uint8_t _txen;
    uint8_t _rxcie;
    uint8_t _txcie;
    uint8_t _udre;
    uint8_t _u2x;
    bool	_bufferedOutput;
    void writeBuffered(uint8_t);
    void writeNonBuffered(uint8_t);
  public:
    HardwareSerial(rx_ring *rx_buffer,tx_ring *tx_buffer,
      volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
      volatile uint8_t *ucsra, volatile uint8_t *ucsrb, volatile uint8_t *ucsrc,
      volatile uint8_t *udr,
      uint8_t rxen, uint8_t txen, uint8_t rxcie, uint8_t txcie, uint8_t udre, uint8_t u2x);
    void begin(long);
    void begin(long baud,char parity, int wordlength, int stop, bool bufferedOutput=true);
    void end();
    uint8_t available(void);
    int read(void);
    void flush(void);
    virtual void write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
    int overflow();
    long received();
};

extern HardwareSerial Serial;

#if defined(__AVR_ATmega1280__)
extern HardwareSerial Serial1;
extern HardwareSerial Serial2;
extern HardwareSerial Serial3;
#endif

#endif
