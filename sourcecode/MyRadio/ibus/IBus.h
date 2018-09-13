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
#ifndef IBUSCONTROLLER_H_
#define IBUSCONTROLLER_H_

#include "IBusMessage.h"

class IBus
{
	uint8_t buffer[255+4];
	int  buffer_len;
	uint8_t checksum;
	long lastRecvTime;
	long lastSend;
	bool checkCTS;

	void processMessage();
public:
	IBus() { buffer_len = 0; lastSend = 0; checksum=0; checkCTS=true; }
	void setup();
	void loop();
	void sendMessage(IBusMessage *msg);
	void sendMessage(uint8_t source,uint8_t destination,const uint8_t *data,uint8_t len);
	void sendMessage(uint8_t source,uint8_t destination,byte b0, byte b1);
	void inject(uint8_t *msg);
	void inject(uint8_t src,uint8_t dst,uint8_t *msg,uint8_t len);
	void setCheckCTS(bool check) { checkCTS = check; };
};

#define DEVICE_SES 0xB0
#define DEVICE_MFL 0x50 // multi-function steering wheel buttons
#define DEVICE_RAD 0x68
#define DEVICE_TEL 0xC8
#define DEVICE_ANZV 0xE7
#define DEVICE_IKE 0x80 // instrument cluster
#define DEVICE_MID 0xC0
#define DEVICE_DSP 0x6A
#define DEVICE_DSPC 0xEA
#define DEVICE_GLO 0xBF // global broadcast
#define DEVICE_LOC 0xFF // local broadcast

#define IBUS_DELAY_MS 75

extern uint8_t DEVICE_STATUS[1];
extern uint8_t DEVICE_STATUS_READY[2];
extern uint8_t DEVICE_STATUS_READY_AFTER_RESET[2];

extern IBus ibus;

#endif
