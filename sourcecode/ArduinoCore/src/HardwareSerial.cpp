/*
 HardwareSerial.cpp - Hardware serial library for Wiring
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

 Modified 23 November 2006 by David A. Mellis
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "wiring.h"
#include "wiring_private.h"

#include "HardwareSerial.h"

#define RX_BUFFER_SIZE 128
#define TX_BUFFER_SIZE 64

#define RX_BUFFER_MASK ( RX_BUFFER_SIZE - 1 )
#define TX_BUFFER_MASK ( TX_BUFFER_SIZE - 1 )

#if ( RX_BUFFER_SIZE & RX_BUFFER_MASK )
	#error RX buffer size is not a power of 2
#endif
#if ( TX_BUFFER_SIZE & TX_BUFFER_MASK )
	#error TX buffer size is not a power of 2
#endif

struct ring_buffer {
	volatile unsigned char head;
	volatile unsigned char tail;
	volatile int overflow;
	volatile long received;

	ring_buffer() {
		head = 0;
		tail = 0;
		overflow=0;
	}
};

struct rx_ring: ring_buffer {
	unsigned char buffer[RX_BUFFER_SIZE];
};

struct tx_ring: ring_buffer {
	unsigned char buffer[TX_BUFFER_SIZE];
};

rx_ring rx_buffer;

#if defined(__AVR_ATmega1280__)
rx_ring rx_buffer1;
rx_ring rx_buffer2;
rx_ring rx_buffer3;
#endif

tx_ring tx_buffer;

#if defined(__AVR_ATmega1280__)
tx_ring tx_buffer1;
tx_ring tx_buffer2;
tx_ring tx_buffer3;
#endif

// Preinstantiate Objects //////////////////////////////////////////////////////

#if defined(__AVR_ATmega8__)
HardwareSerial Serial(&rx_buffer, &tx_buffer, &UBRRH, &UBRRL, &UCSRA, &UCSRB, &UCSRC, &UDR, RXEN, TXEN, RXCIE, TXCEI, UDRE, U2X);
#else
HardwareSerial Serial(&rx_buffer, &tx_buffer, &UBRR0H, &UBRR0L, &UCSR0A,&UCSR0B, &UCSR0C, &UDR0, RXEN0, TXEN0, RXCIE0, TXCIE0, UDRE0, U2X0);
#endif

#if defined(__AVR_ATmega1280__)
HardwareSerial Serial1(&rx_buffer1, &tx_buffer1, &UBRR1H, &UBRR1L, &UCSR1A, &UCSR1B, &UCSR1C, &UDR1, RXEN1, TXEN1, RXCIE1, TXCIE1, UDRE1, U2X1);
HardwareSerial Serial2(&rx_buffer2, &tx_buffer2, &UBRR2H, &UBRR2L, &UCSR2A, &UCSR2B, &UCSR2C, &UDR2, RXEN2, TXEN2, RXCIE2, TXCIE2, UDRE2, U2X2);
HardwareSerial Serial3(&rx_buffer3, &tx_buffer3, &UBRR3H, &UBRR3L, &UCSR3A, &UCSR3B, &UCSR3C, &UDR3, RXEN3, TXEN3, RXCIE3, TXCIE3, UDRE3, U2X3);
#endif

inline void store_char(unsigned char c, rx_ring *rx_buffer) {
	unsigned char tmphead = rx_buffer->head;
	tmphead++;
	tmphead &= RX_BUFFER_MASK;

	// if we should be storing the received character into the location
	// just before the tail (meaning that the head would advance to the
	// current location of the tail), we're about to overflow the buffer
	// and so we don't write the character or advance the head.

	if (tmphead != rx_buffer->tail) {
		rx_buffer->buffer[rx_buffer->head] = c;
		rx_buffer->head = tmphead;
		rx_buffer->received++;
	} else {
		rx_buffer->overflow++;
	}
}

inline void send_char(tx_ring *buf, volatile uint8_t *_ucsrb, uint8_t mask,	volatile uint8_t *data) {
	unsigned char tmptail = buf->tail;

	/* Check if all data is transmitted */
	if (buf->head != tmptail) {
		buf->tail = (tmptail + 1) & TX_BUFFER_MASK;
		*data = buf->buffer[tmptail];
	}
}

#if defined(__AVR_ATmega1280__)

SIGNAL(USART0_RX_vect)
{
	unsigned char c = UDR0;
	store_char(c, &rx_buffer);
}

SIGNAL(USART1_RX_vect)
{
	unsigned char c = UDR1;
	store_char(c, &rx_buffer1);
}

SIGNAL(USART2_RX_vect)
{
	unsigned char c = UDR2;
	store_char(c, &rx_buffer2);
}

SIGNAL(USART3_RX_vect)
{
	unsigned char c = UDR3;
	store_char(c, &rx_buffer3);
}

#else

#if defined(__AVR_ATmega8__)
SIGNAL(SIG_UART_RECV)
#else
SIGNAL(USART_RX_vect)
#endif
{
#if defined(__AVR_ATmega8__)
	unsigned char c = UDR;
#else
	unsigned char c = UDR0;
#endif
	store_char(c, &rx_buffer);
}

#endif

#if defined(__AVR_ATmega1280__)

SIGNAL(USART0_TX_vect)
{
	send_char(&tx_buffer,&UCSR0B,TXCIE0,&UDR0);
}

SIGNAL(USART1_TX_vect)
{
	send_char(&tx_buffer1,&UCSR1B,TXCIE1,&UDR1);
}

SIGNAL(USART2_TX_vect)
{
	send_char(&tx_buffer2,&UCSR2B,TXCIE3,&UDR2);
}

SIGNAL(USART3_TX_vect)
{
	send_char(&tx_buffer3,&UCSR3B,TXCIE3,&UDR3);
}

#else

#if defined(__AVR_ATmega8__)
SIGNAL(SIG_UART_TX)
#else
SIGNAL(USART_TX_vect)
#endif
{
#if defined(__AVR_ATmega8__)
	send_char(&tx_buffer,&UCSRB,TXCIE,&UDR);
#else
	send_char(&tx_buffer,&UCSR0B,TXCIE0,&UDR0);
#endif
}
#endif

// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(rx_ring *rx_buffer, tx_ring *tx_buffer,
		volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
		volatile uint8_t *ucsra, volatile uint8_t *ucsrb, volatile uint8_t *ucsrc,
		volatile uint8_t *udr, uint8_t rxen, uint8_t txen, uint8_t rxcie,
		uint8_t txcie, uint8_t udre, uint8_t u2x) {
	_rx_buffer = rx_buffer;
	_tx_buffer = tx_buffer;
	_ubrrh = ubrrh;
	_ubrrl = ubrrl;
	_ucsra = ucsra;
	_ucsrb = ucsrb;
	_ucsrc = ucsrc;
	_udr = udr;
	_rxen = rxen;
	_txen = txen;
	_rxcie = rxcie;
	_txcie = txcie;
	_udre = udre;
	_u2x = u2x;
	_bufferedOutput = true;
}

// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(long baud) {
	uint16_t baud_setting;
	bool use_u2x;

	// U2X mode is needed for baud rates higher than (CPU Hz / 16)
	if (baud > (long)(F_CPU / 16)) {
		use_u2x = true;
	} else {
		// figure out if U2X mode would allow for a better connection

		// calculate the percent difference between the baud-rate specified and
		// the real baud rate for both U2X and non-U2X mode (0-255 error percent)
		uint8_t
				nonu2x_baud_error =
						abs((int)(255-((F_CPU/(16*(((F_CPU/8/baud-1)/2)+1))*255)/baud)));
		uint8_t
				u2x_baud_error =
						abs((int)(255-((F_CPU/(8*(((F_CPU/4/baud-1)/2)+1))*255)/baud)));

		// prefer non-U2X mode because it handles clock skew better
		use_u2x = (nonu2x_baud_error > u2x_baud_error);
	}

	if (use_u2x) {
		*_ucsra = 1 << _u2x;
		baud_setting = (F_CPU / 4 / baud - 1) / 2;
	} else {
		*_ucsra = 0;
		baud_setting = (F_CPU / 8 / baud - 1) / 2;
	}

	// assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
	*_ubrrh = baud_setting >> 8;
	*_ubrrl = baud_setting;

	sbi(*_ucsrb, _rxen);
	sbi(*_ucsrb, _txen);
	sbi(*_ucsrb, _rxcie);
	sbi(*_ucsrb, _txcie);

	_rx_buffer->tail = _rx_buffer->head = 0;
}

void HardwareSerial::begin(long baud, char parity, int wordlength, int stop, bool bufferedOutput) {
	begin(baud);

	// defaults to 8-bit, no parity, 1 stop bit

	//clear parity, stop bits, word length
	//UCSR0B bit 2=0 for all wordlengths except 9
	//Note: Serial.read routines wont work with 9 bit data as written

	*_ucsrc = *_ucsrc & B11000001;
	*_ucsrb = *_ucsrb & B11111011;

	//set parity
	if ((parity == 'O') | (parity == 'o')) {
		*_ucsrc = *_ucsrc | B00110000;
	} else if ((parity == 'E') | (parity == 'e')) {
		*_ucsrc = *_ucsrc | B00100000;
	} else // ((parity == 'N')|(parity == 'n')))
	{
		*_ucsrc = *_ucsrc | B00000000;
	}

	//set word length
	if (wordlength == 5) {
		*_ucsrc = *_ucsrc | B00000000;
	} else if (wordlength == 6) {
		*_ucsrc = *_ucsrc | B00000010;
	} else if (wordlength == 7) {
		*_ucsrc = *_ucsrc | B00000100;
	} else if (wordlength == 9) {
		*_ucsrc = *_ucsrc | B00000110;
		*_ucsrb = *_ucsrb | B00000100;
	} else // (wordlength == 8)
	{
		*_ucsrc = *_ucsrc | B00000110;
	}

	//set stop bits
	if (stop == 1) {
		*_ucsrc = *_ucsrc | B00000100;
	} else // (stop == 2)
	{
		*_ucsrc = *_ucsrc | B00000000;
	}

	_bufferedOutput = bufferedOutput;
}


void HardwareSerial::end() {
	cbi(*_ucsrb, _rxen);
	cbi(*_ucsrb, _txen);
	cbi(*_ucsrb, _rxcie);
	cbi(*_ucsrb, _txcie);
}

uint8_t HardwareSerial::available(void) {
	return ((RX_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) & RX_BUFFER_MASK);
}

int HardwareSerial::read(void) {
	unsigned char tail = _rx_buffer->tail;
	// if the head isn't ahead of the tail, we don't have any characters
	if (_rx_buffer->head == tail) {
		return -1;
	} else {
		unsigned char c = _rx_buffer->buffer[tail];
		tail++;
		_rx_buffer->tail = tail & RX_BUFFER_MASK;
		return c;
	}
}

void HardwareSerial::flush() {
	cli();
	_rx_buffer->head = _rx_buffer->tail;
	_tx_buffer->head = _tx_buffer->tail;
	sei();
}

void HardwareSerial::write(uint8_t c) {
	if(_bufferedOutput)
		writeBuffered(c);
	else
		writeNonBuffered(c);
}

void HardwareSerial::writeBuffered(uint8_t c) {

	unsigned char tmphead = (_tx_buffer->head + 1) & TX_BUFFER_MASK;

	// Wait for free space in buffer
	while (tmphead == _tx_buffer->tail);

	cbi(*_ucsrb, _txcie);
//	cli();

	bool empty = (_tx_buffer->head==_tx_buffer->tail) && ((*_ucsra) & (1 << _udre));

	if(empty ) {
		sbi(*_ucsrb, _txcie);
//		sei();
		*_udr = c;
	} else {
		_tx_buffer->buffer[_tx_buffer->head] = c; // Store data in buffer
		_tx_buffer->head = tmphead; // Store new index
//		sei();
		sbi(*_ucsrb, _txcie);
	}

}


void HardwareSerial::writeNonBuffered(uint8_t c)
{
  while (!((*_ucsra) & (1 << _udre)))
    ;

  *_udr = c;
}


int HardwareSerial::overflow() {
	return _rx_buffer->overflow;
}

long HardwareSerial::received() {
	return _rx_buffer->received;
}

