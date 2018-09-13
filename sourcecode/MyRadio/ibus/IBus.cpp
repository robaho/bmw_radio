/*******************************************************************************
 * Copyright (c) 2011 Robert Engels
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 *
 * Using this software in a commercial application/device requires prior
 * written approval of the author.
 *
 * Contact Robert Engels using rengels@ix.netcom.com
 *
 * Contributors:
 *     Robert Engels - initial API and implementation
 *******************************************************************************/
#include "WProgram.h"

#include "IBus.h"
#include "util/Arduino.h"
#include "ibusdev/TEL.h"
#include "ibusdev/RAD.h"
#include "ibusdev/DSPC.h"
#include "util/printf.h"

#define SENSTAPIN 10
#define INTERRUPT 1 // must be match PIN above !!!

//#define DEBUG

IBus ibus;
IBusMessage msg;

volatile boolean senstaChange = false;

uint8_t DEVICE_STATUS[] = { 0x01 };
uint8_t DEVICE_STATUS_READY[] = { 0x02, 0x00 };
uint8_t DEVICE_STATUS_READY_AFTER_RESET[] = { 0x02, 0x01 };

void senstaISR() {
	senstaChange = true;
	digitalWrite(13,digitalRead(SENSTAPIN));
}

void IBus::setup() {
	Serial1.begin(9600, 'E', 8, 1, false);

	pinMode(SENSTAPIN, INPUT);

	msg.data = buffer + 3;

//	attachInterrupt(INTERRUPT, senstaISR, HIGH);

	dmsg("IBUS Device setup() complete");
}


void IBus::loop() {

	long now = millis();

loop:

	now = millis();

	digitalWrite(13,digitalRead(SENSTAPIN));

	if (!Serial1.available()) {
		if ((buffer_len > 0) && (millis() - lastRecvTime > 10)) {
			dprintf("*** msg timeout, len = %d ",buffer_len);
			ddump("msg",msg.data,msg.len);
			arduino.blinkError(1);
			buffer_len = 0;
			checksum = 0;
		}
		return;
	}

	if(buffer_len==255) {
		buffer_len=0;
		checksum=0;
	}

	uint8_t c = Serial1.read();

	lastRecvTime = now;

	if ((buffer[1] == (buffer_len - 1)) && (buffer_len>=4)) {

		if(c!=(checksum&0xFF)) {
			dprintf("*** checksum err, chk %02X c %02X len = %d ",checksum,c,buffer_len);
			ddump("msg",msg.data,msg.len);
		}

		msg.len = buffer[1]-2;
		msg.src = buffer[0];
		msg.dst = buffer[2];

		processMessage();

		buffer_len = 0;
		checksum = 0;

		goto loop;
	}

	buffer[buffer_len++] = c;
	checksum ^= c;

	goto loop;
}

void IBus::sendMessage(uint8_t source, uint8_t destination, const uint8_t *data, uint8_t datalen) {

	uint8_t checksum = 0;

	checksum ^= source;
	checksum ^= (datalen+2);
	checksum ^= destination;

	for (int i = 0; i < datalen; i++)
		checksum ^= data[i];

#ifdef DEBUG
	dprintf("ibus msg send src %x dst %x msg ",source,destination);
	for(int i=0;i<datalen;i++) {
		dprintf("%02x ",data[i]);
	}
	dprintf("chk %02x\n\r",checksum);
#endif

	retry:

	// do not flood
	while(millis()-lastSend<IBUS_DELAY_MS);

	long start = millis();

	// wait for idle line
	if(checkCTS) {
		while (digitalRead(SENSTAPIN)==HIGH) {
			if(millis()-start>1000) {
#ifdef DEBUG
				dmsg("failed sending ibus msg");
#endif
				return;
			}
		}
	}

	senstaChange = false;

	// send message

	Serial1.write(source);
	Serial1.write(datalen+2);
	Serial1.write(destination);
	Serial1.write(data,datalen);
	Serial1.write(checksum);

//	if (checkCTS && senstaChange) {
//#ifdef DEBUG
//		dmsg("*** error sending msg, retry");
//#endif
//		arduino.setErrorLED(HIGH);
//		goto retry;
//	}

	if (checkCTS && digitalRead(SENSTAPIN)) {
#ifdef DEBUG
		dmsg("*** error sending msg, retry");
#endif
		arduino.setErrorLED(HIGH);
		goto retry;
	}

	arduino.setErrorLED(LOW);

	lastSend = millis();
}

void IBus::sendMessage(IBusMessage *msg) {
	sendMessage(msg->src, msg->dst, msg->data, msg->len);
}

byte msgbuf[16];

void IBus::sendMessage(uint8_t source, uint8_t destination, byte b0, byte b1) {
	msgbuf[0] = b0;
	msgbuf[1] = b1;
	sendMessage(source,destination,msgbuf,2);
}

void IBus::inject(uint8_t *m) {
	msg.src=m[0];
	msg.dst=m[2];
	msg.len = m[1]-2;
	memcpy(msg.data,m+3,msg.len);

	processMessage();
}

void IBus::inject(uint8_t src,uint8_t dst,uint8_t *data,uint8_t len) {
	msg.src=src;
	msg.dst=dst;
	msg.len = len;

	memcpy(msg.data,data,msg.len);

	processMessage();
}

void IBus::processMessage() {

#ifdef DEBUG
	dprintf("ibus msg rcvd src %x dst %x overflow %d ",msg.src,msg.dst,Serial1.overflow());
	ddump("msg",msg.data,msg.len);
#endif

	switch (msg.dst) {
	case DEVICE_TEL:
		tel.handleMessage(&msg);
		break;
	case DEVICE_RAD:
	case DEVICE_LOC:
	case DEVICE_GLO:
		rad.handleMessage(&msg);
		break;
	case DEVICE_MID:
	case DEVICE_ANZV:
		mid.handleMessage(&msg);
		break;
	case DEVICE_DSPC:
		dspc.handleMessage(&msg);
		break;
	default:
		break;
	}
}
