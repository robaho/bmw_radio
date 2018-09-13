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
#include "DSPC.h"
#include "MID.h"
#include "util/printf.h"

DSPC dspc;

static char buffer[32];

static const char* freqlabels[] = { "80hz","200hz","500hz","1Khz","2Khz","5Khz","12Khz" };
static const char* modelabels[] = { "CONCERT HALL","JAZZ CLUB","CATHEDRAL", "MEMORY 1","MEMORY 2","MEMORY 3","DSP OFF" };

void DSPC::loop() {
	if(needMemoryInfo && millis()-lastrequest>1000) {
		ibus.sendMessage(DEVICE_DSPC,DEVICE_DSP,0x34,0x08);
		lastrequest=millis();
	}
}

void DSPC::handleMessage(IBusMessage *m) {
	if(m->data[0]==0x01) {
		if(reset) {
			ibus.sendMessage(DEVICE_DSPC,DEVICE_LOC,DEVICE_STATUS_READY_AFTER_RESET,sizeof(DEVICE_STATUS_READY_AFTER_RESET));
			reset=false;
		} else {
			ibus.sendMessage(DEVICE_DSPC,DEVICE_LOC,DEVICE_STATUS_READY,sizeof(DEVICE_STATUS_READY));
		}
		return;
	}

	if(m->data[0]==0x35) {
		needMemoryInfo=false;
		// update from DSP
		mode = MODE(m->data[1] - 1);
		reverb = m->data[2] & 0x0F;
		if(m->data[2]&0x10)
			reverb*=-1;
		roomsize = m->data[3] & 0x0F;
		if(m->data[3]&0x10)
			roomsize*=-1;
		for(int f = 0; f < 7; f++) {
			boost[f] = m->data[4+f] & 0x0F;
			if(m->data[4+f] & 0x10)
				boost[f]*=-1;
		}
	}
}

void DSPC::powerOn() {
	needMemoryInfo=true;
	reset=true;
	lastrequest=millis();
}

void DSPC::handleMIDbutton(int button) {

	if(button>=0 && button<=1) {
		if(state==S_MODE) {
			if(mode==DSP_OFF)
				mode=CONCERT_HALL;
			else
				mode=MODE(mode+1);
			ibus.sendMessage(DEVICE_DSPC,DEVICE_DSP,0x34,0x09+mode);
		} else {
			state = S_MODE;
		}
		displayMenu();
		displayDSP();
	}

	if(!isMemory())
		return;

	if(button==10) {
		flat();
	}

	if(button>=2 && button<=3) {
		if(state==S_REVERB) {
			reverb+= button==2 ? -1 : 1;
			if(reverb<0)
				reverb=0;
			if(reverb>7)
				reverb=7;

			buffer[0]=0x34;
			buffer[1]=0x94 + memory;
			buffer[2]=reverb & 0x0F ;
			ibus.sendMessage(DEVICE_DSPC,DEVICE_DSP,(const uint8_t *)buffer,3);
		} else
			state = S_REVERB;
	}

	if((button>=4 && button<=5)) {
		if(state==S_ROOMSIZE) {
			roomsize+= button==4 ? -1 : 1;
			if(roomsize<0)
				roomsize=0;
			if(roomsize>7)
				roomsize=7;

			buffer[0]=0x34;
			buffer[1]=0x94 + memory;
			buffer[2]=(roomsize & 0x0F) | 0x20;
			ibus.sendMessage(DEVICE_DSPC,DEVICE_DSP,(const uint8_t *)buffer,3);
		} else
			state = S_ROOMSIZE;
	}

	if(button==6) {
		if(state==S_FREQUENCY) {
			if(curfreq==F_80HZ)
				curfreq=F_12KHZ;
			else
				curfreq = FREQUENCY(curfreq-1);
		} else {
			state = S_FREQUENCY;
		}
	}

	if(button==7) {
		if(state==S_FREQUENCY) {
			if(curfreq==F_12KHZ)
				curfreq=F_80HZ;
			else
				curfreq = FREQUENCY(curfreq+1);
		} else {
			state = S_FREQUENCY;
		}
	}

	if(state==S_FREQUENCY && (button>=8 && button<=9)) {
		int b = boost[curfreq];
		b += (button==8) ? -1 : 1;
		if(b<-10)
			b=-10;
		if(b>10)
			b=10;
		boost[curfreq] = b;
		buffer[0]=0x34;
		buffer[1]=0x14 + memory;
		buffer[2]=(((curfreq*2)<<4) & 0xF0) | ((b < 0 ? (0x10 | (abs(b) & 0x0F)) : (b & 0x0F)));
		ibus.sendMessage(DEVICE_DSPC,DEVICE_DSP,(const uint8_t *)buffer,3);
	}

	displayDSP();
}

void DSPC::flat() {
	reverb=0;

	buffer[0]=0x34;
	buffer[1]=0x94 + memory;
	buffer[2]=reverb & 0x0F ;
	ibus.sendMessage(DEVICE_DSPC,DEVICE_DSP,(const uint8_t *)buffer,3);

	roomsize=0;
	buffer[0]=0x34;
	buffer[1]=0x94 + memory;
	buffer[2]=(roomsize & 0x0F) | 0x20;
	ibus.sendMessage(DEVICE_DSPC,DEVICE_DSP,(const uint8_t *)buffer,3);

	for(int i=0;i<7;i++) {
		boost[i]=0;
		buffer[0]=0x34;
		buffer[1]=0x14 + memory;
		buffer[2]=(((i*2)<<4) & 0xF0);
		ibus.sendMessage(DEVICE_DSPC,DEVICE_DSP,(const uint8_t *)buffer,3);
	}
}


void DSPC::wakeup() {
	state=S_MODE;
	displayDSP();
}

void DSPC::displayDSP() {

	if(needMemoryInfo) {
		mid.display(DEVICE_RAD,"NO DSP AMP");
		return;
	}

	if(!isMemory()) {
		mid.display(DEVICE_RAD,modelabels[mode]);
		return;
	}

	switch(state) {
	case S_MODE:
		mid.display(DEVICE_RAD,modelabels[mode]);
		break;
	case S_REVERB:
		sprintf(buffer,"M%d REVERB %-d",memory+1,reverb);
		mid.display(DEVICE_RAD,buffer);
		break;
	case S_ROOMSIZE:
		sprintf(buffer,"M%d ROOMSIZE %-d",memory+1,roomsize);
		mid.display(DEVICE_RAD,buffer);
		break;
	case S_FREQUENCY:
		sprintf(buffer,"M%d %s %-d",memory+1,freqlabels[curfreq],boost[curfreq]);
		mid.display(DEVICE_RAD,buffer);
		break;
	}
}

void DSPC::displayMenu() {
	if(isMemory()) {
		midmenu.set(7," MODE ","-REVERB+","-ROOMSZ+","- FREQ +","- BOOST+","FLAT","\xAD");
	} else {
		midmenu.set(7," MODE ","     ","     ","     ","     "," ","\xAD");
	}
}
