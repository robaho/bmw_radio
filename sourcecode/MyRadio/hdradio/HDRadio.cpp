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
#include "hdlisten.h"
#include "hdcommands.h"
#include "HDRadio.h"
#include "util/printf.h"
#include "util/util.h"
#include "util/Settings.h"

HDRadio hdradio;

//#define DEBUG

HDRadio::HDRadio() {
}

void HDRadio::setup() {

	Serial2.begin(115200);
	pinMode(POWER_MUTE_PIN,OUTPUT);

	hdlisten.setSerial(Serial2);
	hdcmds.setSerial(Serial2);

	power=false;
	seekHDOnly = false;
	hdenabled = true;

	hdlisten.setup();
	hdcmds.setup();

	timer = millis();

	lastBAND = BAND_FM;
	lastStation[BAND_FM] = STATION(931,1);
	lastStation[BAND_AM] = STATION(1000,0);

	loadPresets();
}

void HDRadio::loop() {

	hdlisten.loop();
	hdcmds.loop();

	if(millis()-timer>3000) {
		if(!power) {
			hdcmds.request(CMD_POWER);
		}
		hdcmds.request(CMD_SIGNALSTRENGTH);
//		if(hdactive)
//			hdcmds.request(CMD_HDSIGNALSTRENGTH);
		timer = millis();
	}
}

void HDRadio::preset(int n) {
	hdcmds.tune(band,FREQ(presets[band][n]),SUBCHANNEL(presets[band][n]));
}

void HDRadio::savepreset(int n) {
	presets[band][n] = STATION(frequency,subchannel);

	int addr = HDRADIO_ADDR+band*NUM_PRESETS*2+n*2;
	settings.writeShort(addr,presets[band][n]);
}

void HDRadio::presetUpDown(int dir) {
	int current = currentPreset();
	int last=current;

	if(current==-1)
		current=(dir==-1 ? (NUM_PRESETS-1) : 0);
	else {
		current+=dir;
	}

	while(true) {
		if(current==NUM_PRESETS)
			current=0;
		if(current==-1)
			current=NUM_PRESETS-1;

		if(presets[band][current]) {
			preset(current);
			return;
		}

		if(last==-1)
			last=current;
		else if(last==current)
			return;

		current+=dir;
	}
}

int HDRadio::currentPreset() {
	unsigned short current = STATION(frequency,subchannel);

	for(int i=0;i<10;i++) {
		if(presets[band][i]==current) {
			return i;
		}
	}
	return -1;
}

void HDRadio::loadPresets() {
	for(int band=0;band<2;band++) {
		for(int n=0;n<NUM_PRESETS;n++) {
			int addr = HDRADIO_ADDR+band*NUM_PRESETS*2+n*2;
			presets[band][n] = settings.readShort(addr);
			if(SUBCHANNEL(presets[band][n])==15) {
				presets[band][n]=0;
			}
		}
	}
}

void HDRadio::poweredOn() {
	power= true;

	hdactive=false;
	hdstreamlock=false;

	if(ismuted) {
		hdcmds.muteOn();
	}

	HDEnabled(hdenabled);

	hdcmds.sendcommand(CMD_BASS,OP_SET,15);
	hdcmds.sendcommand(CMD_TREBLE,OP_SET,15);
	hdcmds.sendcommand(CMD_VOLUME,OP_SET,70);

	int newband = lastBAND;
	int newfrequency = FREQ(lastStation[lastBAND]);
	int newsubchannel = SUBCHANNEL(lastStation[lastBAND]);

	band=BAND_UNKNOWN;
	frequency=0;
	subchannel=0;

	hdcmds.tune(newband,newfrequency,newsubchannel);
}

void HDRadio::seekUpDown(int dir) {
	dir > 0 ? hdcmds.seekUp() : hdcmds.seekDown();
}

void HDRadio::tuneUpDown(int dir) {
	dir > 0 ? hdcmds.tuneUp() : hdcmds.tuneDown();
}

void HDRadio::powerOn() {
	if(power)
		return;
#ifdef DEBUG
	dmsg("hdradio power ON !");
#endif
	Serial2.flush();
	digitalWrite(POWER_MUTE_PIN,HIGH); // bring TTL high, which brings -12v on RS232 to turn on/disable mute on radio
	timer=0; // so we check right away
}

void HDRadio::powerOff() {
#ifdef DEBUG
	dmsg("hdradio power OFF !");
#endif
	digitalWrite(POWER_MUTE_PIN,LOW); // bring TTL low, which brings +12v on RS232 to turn on/disable mute on radio
	Serial2.flush();
	power=false;
}

bool HDRadio::isOn() {
	return power;
}

void HDRadio::bandAM() {
	hdcmds.tune(BAND_AM,FREQ(lastStation[BAND_AM]),SUBCHANNEL(lastStation[BAND_AM]));
}

void HDRadio::bandFM() {
	hdcmds.tune(BAND_FM,FREQ(lastStation[BAND_FM]),SUBCHANNEL(lastStation[BAND_FM]));
}

void HDRadio::tune(int band,int frequency,int subchannel){
	hdcmds.tune(band,frequency,subchannel);
}

void HDRadio::muteOn() {
	ismuted=true;
	hdcmds.muteOn();
}

void HDRadio::muteOff() {
	ismuted=false;
	hdcmds.muteOff();
}

void HDRadio::HDEnabled(bool b) {
	hdenabled = b;
	b ? hdcmds.HDOn() : hdcmds.HDOff();
}

bool HDRadio::isChanged() {
	bool temp = ischanged;
	ischanged = false;
	return temp;
}

void HDRadio::markChanged() {
	ischanged = true;

	if(subchannelbit==0)
		subchannel=0;
	else {
		for(int i=1;i<0xFF;i++) {
			if(subchannelbit == (1<<(i-1))) {
				subchannel = i;
				break;
			}
		}
	}

	lastBAND = band;
	lastStation[lastBAND] = STATION(frequency,subchannel);
}

char hdradio_buffer[128];

const char * HDRadio::getStation(bool longmode, bool showsignal) {

	if(!power || band==BAND_UNKNOWN)
		return "PLEASE WAIT";

	char * cp = hdradio_buffer;

	if(band==BAND_AM) {
		sprintf(cp,"AM %4d ",frequency);
	} else {
		sprintf(cp,"FM %3d.%d ",frequency/10,frequency%10);
	}

	cp = cp + strlen(cp);

	if(hdactive) {
		if(subchannel>0) {
			sprintf(cp,"HD%d",subchannel);
		} else {
			strcpy(cp,"HD ");
		}
	} else {
		if(band==BAND_FM)
			strcpy(cp,"ST ");
	}

	if(!longmode)
		return hdradio_buffer;

	cp = cp + strlen(cp);

 	sprintf(cp," %4s ",callsign);

 	cp+=6;

	if(showsignal) {
		int level = (signalstrength/2)-1;
		if(level<0)
			level=0;
		if(level>6)
			level=6;

		for(int i=0;i<level;i++) {
			*cp++=(0xB7-i);
		}
		*cp++=0;
	}

	return hdradio_buffer;
}

const char * HDRadio::getTitleArtist() {

	if(!power || band==BAND_UNKNOWN)
		return "";

	char * cp = hdradio_buffer;

	if(empty(title) && empty(artist))
		return station;

	if(notempty(title)) {
		strcpy(cp,title);
		cp+=strlen(cp);

		if(notempty(artist)) {
			strcpy(cp," / ");
			cp+=3;
			strcpy(cp,artist);
		}
	}

	return hdradio_buffer;
}

void HDRadio::displayInformation() {
	dmsg(getStation(true,false));
	if(strlen(station)>0) {
		dmsg("station",station);
	}
	if(strlen(title)>0)
	{
		dmsg("title",title);
	}
	if(strlen(artist)>0)
	{
		dmsg("artist",artist);
	}
}
