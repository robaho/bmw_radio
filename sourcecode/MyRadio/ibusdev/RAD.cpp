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
#include "RAD.h"
#include "MID.h"
#include "MFL.h"
#include "IKE.h"
#include "DSPC.h"

#include "hdradio/HDRadio.h"
#include "ipod/IPOD.h"
#include "switch/AudioSwitch.h"
#include "util/printf.h"
#include "util/util.h"
#include "util/NoisyPin.h"
#include "util/Settings.h"
#include <avr/sleep.h>

//#define DEBUG

RAD rad;

#define TELEPHONE_MUTE_PIN 9

bool debugon=false;
static char debugbuf[64];
static int debugindex = 0;

static NoisyPin telmute = NoisyPin(TELEPHONE_MUTE_PIN,500);

void sleepNow();

RAD::RAD() {
	menu = 0;
	source = 0;
	dspmode=0;
	seekingDir=0;
	ispowerOn = false;

	MFLtune=0;
	lastupdate=0;
	lastcheck=0;
	lastMIDinput=0;
	powerOnDelay=0;

	muted = false;

	for(int i=0;i<TONE_COUNT;i++)
		tone[i]=0;
}

void RAD::setup() {

	hdradio.setup();
	ipod.setup();

	muteOn();

	pinMode(TELEPHONE_MUTE_PIN,INPUT);
	digitalWrite(TELEPHONE_MUTE_PIN,HIGH);

	ispowerOn = (byte)settings.readByte(SETTING_POWER);
	source = (byte)settings.readByte(SETTING_SOURCE);

	tone[TONE_BASS]=(byte)settings.readByte(SETTING_TONE_BASS);
	tone[TONE_TREBLE]=(byte)settings.readByte(SETTING_TONE_TREBLE);
	tone[TONE_BALANCE]=(byte)settings.readByte(SETTING_TONE_BALANCE);
	tone[TONE_FADER]=(byte)settings.readByte(SETTING_TONE_FADER);

	byte checksum = (byte)settings.readByte(SETTING_CHECKSUM);

	hdradio.seekHDOnly = (bool)settings.readByte(SETTING_SEEK_HD_ONLY);

	hdradio.hdenabled = (bool)settings.readByte(SETTING_HD_ENABLED);
	ipod.charging = (bool)settings.readByte(SETTING_IPOD_CHARGE);
	volume = (byte)settings.readByte(SETTING_VOLUME);

	byte chk = ~(ispowerOn^source^tone[TONE_BASS]^volume);

	if(chk!=checksum) {
		ispowerOn=false;
		source=SOURCE_FM;
		tone[TONE_BASS]=0;
		tone[TONE_TREBLE]=0;
		tone[TONE_BALANCE]=0;
		tone[TONE_FADER]=0;
		hdradio.hdenabled=true;
		ipod.charging=true;
	}

	hdradio.HDEnabled(hdradio.hdenabled);
	ipod.chargingOnOff(ipod.charging);

	if(ispowerOn)
		powerOn();

#ifdef DEBUG
	dprintf("restored power to %d\n\r",ispowerOn);
#endif
}

void RAD::powerOn() {
#ifdef DEBUG
	dmsg("RAD power ON !");
#endif
	ispowerOn=true;
	hdradio.powerOn();
	powerOnDelay = millis() + 3000;
}

void RAD::powerOff() {
#ifdef DEBUG
	dmsg("RAD power OFF !");
#endif
	ibus.sendMessage(DEVICE_RAD,DEVICE_LOC,0x36, 0xAF); // source off
	mid.clear(DEVICE_RAD);
	delay(1000);
	muteOn();
	hdradio.powerOff();
	ispowerOn=false;

//	sleepNow();
}

void RAD::poweredOn() {
#ifdef DEBUG
	dmsg("powered ON");
#endif
	ibus.sendMessage(DEVICE_RAD,DEVICE_LOC,0x36, 0xA1); // source tuner/tape
	delay(1000);

	switchToSource(source);

	muteOff();

	sendVolume();
	updateTONE();
	dspc.powerOn();

	displayMenu();
	displaySource();
	powerOnDelay=0;
}

void RAD::saveSettings() {
	byte chk = ~(ispowerOn^source^tone[TONE_BASS]^volume);
	settings.writeByte(SETTING_POWER,ispowerOn);
	settings.writeByte(SETTING_SOURCE,source);
	settings.writeByte(SETTING_TONE_BASS,tone[TONE_BASS]);
	settings.writeByte(SETTING_TONE_TREBLE,tone[TONE_TREBLE]);
	settings.writeByte(SETTING_TONE_BALANCE,tone[TONE_BALANCE]);
	settings.writeByte(SETTING_TONE_FADER,tone[TONE_FADER]);
	settings.writeByte(SETTING_SEEK_HD_ONLY,hdradio.seekHDOnly);
	settings.writeByte(SETTING_HD_ENABLED,hdradio.hdenabled);
	settings.writeByte(SETTING_IPOD_CHARGE,ipod.charging);
	settings.writeByte(SETTING_VOLUME,volume);
	settings.writeByte(SETTING_CHECKSUM,chk);
}

void RAD::loop() {

	unsigned long now = millis();

	ipod.loop();
	hdradio.loop();

	telmute.update();

	if(!telmute.read()) {
		muteOn();
	} else {
		muteOff();
	}

	if(!ispowerOn) {
		return;
	}

	if(powerOnDelay && (millis()-powerOnDelay>0)) {
		poweredOn();
	}

	if(debugon)
		return;

	if(menu==MENU_TONE || menu==MENU_DSP) {
		if(millis()-lastMIDinput>10000) {
			sourceswitched=true;
			switchToAUDIO();
		} else {
			return;
		}
	}

	if(menu!=MENU_MAIN && menu!=MENU_SOURCE && menu!=MENU_OPTIONS) {
		return;
	}

	if(seekingDir!=0 && ((now - lastupdate) > seekdelay)) {
		seekUpDown(seekingDir);
		if(MFLtune)
			MFLtune=now;
	}

	if(now-MFLtune>5000)
		MFLtune=0;

	displaySource();
}

void RAD::muteOn() {
	if(!muted) {
		dmsg("radio mute on");
		audioswitch.switchToMute();
		if(isipod())
			ipod.pause();
		if(istuner())
			hdradio.muteOn();
		muted=true;
		ibus.sendMessage(DEVICE_RAD,DEVICE_LOC,0x36, 0xAF); // source off
		powerOnDelay=0;
	}
}

void RAD::muteOff() {
	if(muted) {
		dmsg("radio mute off");
		if(isipod())
			ipod.play();
		if(istuner())
			hdradio.muteOff();
		switchToSource(source);
		muted=false;
		powerOnDelay=millis()+2000;
	}
}

void RAD::displaySource() {
	switch(source) {
	case SOURCE_FM:
	case SOURCE_AM:

		if(sourceswitched || hdradio.isChanged() || (!longmode && !station_displayed)) {

#ifdef DEBUG
	dprintf("displaying tuner source\n\r");
#endif
			sourceswitched=false;

			mid.display(DEVICE_RAD,hdradio.getStation(longmode,true));

			if(MFLtune)
				ike.display(DEVICE_RAD,hdradio.getStation(true,false));

			station_displayed=true;

			if(menu==MENU_SOURCE && preset!=hdradio.currentPreset())
				displayMenu();

			lastupdate=millis();
		}

		if(!longmode || seekingDir!=0) {
			// don't do scrolling of song information when not in audio mode - only 11 chars
			return;
		}

		if(!mid.isScrolling() && millis()-lastupdate>15000) {
			lastupdate=millis();
			const char *cp;
			if(station_displayed) {
				cp = hdradio.getTitleArtist();
				if(empty(cp)) {
					// don't have any other information, so nothing to do
					return;
				}
			} else {
				cp = hdradio.getStation(longmode,true);
			}
			station_displayed^=true;
			mid.display(DEVICE_RAD,cp);
		}
		break;
	case SOURCE_IPOD:
		if(sourceswitched || ipod.isChanged() || millis()-lastupdate>15000) {
#ifdef DEBUG
	dprintf("displaying source ipod\n\r");
#endif
			sourceswitched=false;
			const char*msg = ipod.getTitleArtist();
			if(ipod.locked && MFLtune) {
				ike.display(DEVICE_RAD,msg);
			}
			mid.display(DEVICE_RAD,msg,longmode);
			lastupdate=millis();
		}
		break;
	case SOURCE_AUX:
		if(sourceswitched) {
			sourceswitched=false;
			mid.display(DEVICE_RAD,"AUX",false);
			lastupdate=millis();
		}
		break;
	}
}

void RAD::handleMessage(IBusMessage *m) {

	if(m->src==DEVICE_RAD) {
		// ignore our own messages
		return;
	}

	if(m->data[0]==0x11) { // ignition status
		if(m->data[1]==0x00) {
			audioswitch.switchToMute();
			hdradio.powerOff();
			ipod.pause();
			saveSettings();
		}
		return;
	}

	if(m->data[0]==0x01) {
		ibus.sendMessage(DEVICE_RAD,DEVICE_LOC,DEVICE_STATUS_READY,sizeof(DEVICE_STATUS_READY));
		ibus.sendMessage(DEVICE_RAD,DEVICE_DSP,DEVICE_STATUS,sizeof(DEVICE_STATUS));
		return;
	}

	if(m->data[0]==0x32) { // volume up/down
		int count = (m->data[1] >> 4) & 0x0F;
		if(!(m->data[1] & 0x0F))
			count*=-1;
		volume+=count;
		if(volume<0)
			volume=0;
		if(volume>64)
			volume=64;
		return;
	}

	switch(m->src) {
	case DEVICE_MID:
		handleMID(m); break;
	case DEVICE_MFL:
		handleMFL(m); break;
	case DEVICE_DSP:
		handleDSP(m); break;
	}
}

void RAD::sendVolume() {
	int count = volume/3;
	for(int i=0;i<count;i++) {
		ibus.sendMessage(DEVICE_RAD,DEVICE_DSP,0x32, ((3 << 4) & 0xF0) | 0x01);
	}
	count = volume%3;
	if(count>0)
		ibus.sendMessage(DEVICE_RAD,DEVICE_DSP,0x32, ((count << 4) & 0xF0) | 0x01);
}

#define TONE_VALUE(x) ((abs(tone[x]) & 0x0F) | (tone[x] < 0 ? 0x10 : 0x00))

void RAD::updateTONE() {
	ibus.sendMessage(DEVICE_RAD,DEVICE_LOC,0x36, 0x60 | TONE_VALUE(TONE_BASS));
	ibus.sendMessage(DEVICE_RAD,DEVICE_LOC,0x36, 0xC0 | TONE_VALUE(TONE_TREBLE));
	ibus.sendMessage(DEVICE_RAD,DEVICE_LOC,0x36, 0x80 | TONE_VALUE(TONE_FADER));
	ibus.sendMessage(DEVICE_RAD,DEVICE_LOC,0x36, 0x40 | TONE_VALUE(TONE_BALANCE));
}

void RAD::switchToSource(int _source) {
#ifdef DEBUG
	dprintf("switching to source %d\n\r",_source);
#endif
	source = _source;
	sourceswitched=true;
	seekingDir=0;

	switch(source) {
	case SOURCE_FM:
		switchToFM(); break;
	case SOURCE_AM:
		switchToAM(); break;
	case SOURCE_IPOD:
		switchToIPOD(); break;
	case SOURCE_AUX:
		switchToAUX(); break;
	}
}

void RAD::handleDSP(IBusMessage *m) {

	if(m->data[0]==0x02) {
		bool afterreset = (m->data[1]==0x01);

		ibus.sendMessage(DEVICE_RAD,DEVICE_LOC,0x36, 0x30 | (dspmode & 0x0F));

		if(afterreset) {
			ispowerOn ? powerOn() : powerOff();
			dspreset = true;
		}
	}
}

void RAD::handleMFL(IBusMessage *m) {
#ifdef DEBUG
	dprintf("mfl message %x %x\n\r",m->data[0],m->data[1]);
#endif

	if(!ispowerOn)
		return;

	if(m->data[0]!=0x3B)
		return;


	if(scanmode) {
		scanmode=false;
		displayMenu();
		return;
	}

	if(m->data[1]==0x40) {
		sourceswitched=true;
		return;
	}

//	bool press = (m->data[1] & 0xF0) == 0x00;
	bool presslong = (m->data[1] & 0x10) == 0x10;
	bool release = (m->data[1] & 0x20) == 0x20;

	int dir=0;
	switch(m->data[1] & 0x0F) {
	case 0x01: // next press
		dir=1;
		break;
	case 0x08: // prev press
		dir=-1;
		break;
	}

	MFLtune = millis();

	if(presslong && seekingDir==0) {
		seekingDir=dir;
		seekdelay=isipod() ? 500 : 2000;
		seekstart = millis();
		seekUpDown(dir);
	}

	if(release) {
		if(seekingDir!=0) {
			seekingDir=0;
			return;
		} else {
			tuneUpDown(dir);
		}
	}

}

void RAD::seekUpDown(int dir) {

	lastupdate=millis();

	unsigned long now = millis();

	lastseek=now;

	if (istuner()) {
		if(MFLtune || scanmode)
			hdradio.seekUpDown(dir);
		else
			hdradio.tuneUpDown(dir);
	}

	if (isipod()) {
		int offset = 1;

		if(now-seekstart>2500)
			seekdelay=250;

		if(now-seekstart>5000)
			seekdelay=50;

		if(now-seekstart>7500)
			offset = 2;

		if(ipod.locked)
			ipod.itemUpDown(dir,offset);
		else
			ipod.albumUpDown(dir);
	}
}

void RAD::tuneUpDown(int dir) {
	if(millis()-lastseek<5000) {
		seekstart=millis();
		seekUpDown(dir);
		return;
	}
	if (istuner()) {
		MFLtune ? hdradio.presetUpDown(dir) : hdradio.tuneUpDown(dir);
	}
	if (isipod())
		ipod.songUpDown(dir);
}

void RAD::handleMID(IBusMessage *m) {
	switch(m->data[0]) {
	case 0x20: // MID display status
	{
		bool audio_onoff = m->data[1] & 0x20;
//		bool presslong = m->data[1] & 0x40;

//		dprintf("mid display 0x20 status audio onoff = %d, long = %d ",audio_onoff,long_format);

		if(audio_onoff) {
			ispowerOn^=true;
			ispowerOn ? powerOn() : powerOff();

			saveSettings();
		}

		if(!ispowerOn)
			return;

		if((m->data[2] & 0x10)) {
			// use "long" mode if showing audio menu
			// FIXME - if not showing clock (i.e. BC), should really only use short format
			longmode=true;
		} else {
			longmode=false;
		}

		if(m->data[1] & 0x01) {
			// audio button pressed on MID
			menu = MENU_MAIN;
			displayMenu();
		} else if (m->data[1] & 0x10 || m->data[2] & 0x01) {
			displayMenu();
		}

		if(m->data[2] & 0x20 || m->data[2] & 0x01) {
			displaySource();
		}
	}
	break;
	case 0x31: // MID button
	{
		int  button = m->data[3] & 0x0F;
		bool press = ((m->data[3] & 0xF0) == 0x00);
		bool presslong = ((m->data[3] & 0xF0) == 0x20);
		bool release = ((m->data[3] & 0xF0) == 0x40);

		handleMIDbutton(button,press,presslong,release);
	}
	break;
	case 0x32: // volume control
	{
		int count = (m->data[1] >> 4) & 0x0F;
		if(!(m->data[1] & 0x0F))
			count*=-1;
		volume+=count;
		if(volume<0)
			volume=0;
		if(volume>64)
			volume=64;
	}
	break;
	case 0x22: // acknowledge text display
		// not sure this needs to be handled
	break;
	default:
		if(m->dst==DEVICE_RAD) {
			dprintf("unknown MID->RAD message %02X\n\r",m->data[0]);
		}
	}
}

void RAD::handleMIDbutton(int button,bool _press,bool _presslong,bool _release) {
//	dprintf("button %d press %d presslong %d release %d\n\r",button,_press,_presslong,_release);

	lastMIDinput = millis();
	MFLtune=0;

	switch(menu) {
	case MENU_MAIN:
		if(handleMIDtune(button,_press, _presslong,_release))
			return;

		if(!_release)
			return;

		if(button>=0 && button<=7) {
			switchToSource(button/2);
		}
		if(button==10) {
			switchToTONE();
		}
		if(button==11) {
			switchToOPTIONS();
		}
	break;
	case MENU_SOURCE:

		if(scanmode && button!=10) {
			scanmode=false;
			seekingDir=0;
			displayMenu();
			return;
		}

		if(handleMIDtune(button,_press,_presslong,_release))
			return;

		if(button==11 && _release) {
			switchToAUDIO();
			return;
		}

		switch(source) {
		case SOURCE_AM:
		case SOURCE_FM:
			if(button==10 && _release) {
				scanmode^=true;
				seekingDir=scanmode ? 1 : 0;
				seekdelay=2000;
				displayMenu();
				return;
			}

			if(button>10) {
				return;
			}

			// button 0-9 handling

			if(_presslong) {
				hdradio.savepreset(button);
				displayMenu();
				return;
			}

			if(_release)
				hdradio.preset(button);

			break;
		case SOURCE_IPOD:

			if(!_release)
				return;

			switch(button) {
			case 0:
			case 1:
				if(!ipod.locked)
					ipod.playlistUpDown(button==0 ? -1 : 1);
				else {
					ipod.modePlaylist();
					displayMenu();
				}
				break;
			case 2:
			case 3:
				if(!ipod.locked)
					ipod.albumUpDown(button==2 ? -1 : 1);
				else {
					ipod.modeArtist();
					displayMenu();
				}
				break;
			case 4:
			case 5:
				if(ipod.locked) {
					ipod.modeAlbum();
					displayMenu();
				}
				break;
			case 6:
			case 7:
				if(ipod.locked) {
					ipod.modeGenre();
					displayMenu();
				}
				break;
			case 8:
				ipod.lockOnOff(!ipod.locked);
				displayMenu();
				break;
			case 9:
				ipod.shuffleOnOff(!ipod.shuffle);
				displayMenu();
				break;
			}
			break;
		}
		break;
	case MENU_TONE:
		if(!_release)
			return;

		if(button==10) {
			switchToDSP();
			return;
		}

		if(button==11) {
			switchToAUDIO();
			return;
		}

		if(button>=8 && button<=9) {
			for(int i=0;i<TONE_COUNT;i++)
				tone[i]=0;
			updateTONE();
			mid.display(DEVICE_RAD,"FLAT");
			return;
		}

		if(button>=0 && button<=7) {
			int tonemode = button/2;
			int dir = (button%2==1) ? 1 : -1;
			toneUpDown(tonemode,dir);
		}
		break;
	case MENU_DSP:
		if(!_release)
			return;

		if(button==11) {
			switchToAUDIO();
			return;
		}

		dspc.handleMIDbutton(button);

		break;
	case MENU_OPTIONS:
		if(!_release)
			return;
		if(button==6) {
#ifdef DEBUG_ENABLED
			debug();
#endif
		}
		if(button==11) {
			debugon = false;
			debugindex=0;
			switchToAUDIO();
		}
		if(button==0 || button==1) {
			hdradio.seekHDOnly^=true;
			displayMenu();
		}
		if(button==2 || button==3) {
			hdradio.HDEnabled(hdradio.hdenabled^true);
			displayMenu();
		}
		if(button==4 || button==5) {
			ipod.chargingOnOff(ipod.charging^true);
			displayMenu();
		}
	break;
	}
}

bool RAD::handleMIDtune(int num,bool _press,bool _presslong,bool _release) {
	if(num!=12 && num!=13) {
		return false;
	}

	if(_presslong && seekingDir==0) {
		seekingDir = (num==12 ? -1 : 1);
		seekdelay = 500;
		seekstart = millis();
		seekUpDown(seekingDir);
		return true;
	}

	if(!_release)
		return true;

	if(seekingDir!=0) {
		seekingDir=0;
		return true;
	}

	if(isipod() && ipod.locked)
		seekUpDown(num==12 ? -1 : 1);
	else
		tuneUpDown(num==12 ? -1 : 1);

	return true;
}

static char tonebuffer[16];
static const char *tonenames[TONE_COUNT] = { "BASS","TREBLE","FADER","BALANCE" };

void RAD::toneUpDown(int tonemode,int dir) {
	tone[tonemode]+=dir;

	if(tone[tonemode]<-15)
		tone[tonemode]=-15;

	if(tone[tonemode]>15)
		tone[tonemode]=15;

	sprintf(tonebuffer,"%s %-d",tonenames[tonemode],tone[tonemode]);

	mid.display(DEVICE_RAD,tonebuffer,false);
	if(dir!=0)
		updateTONE();
}

void RAD::switchToFM() {
	source = SOURCE_FM;
	ipod.pause();
	hdradio.bandFM();
	hdradio.muteOff();
	audioswitch.switchToHDRadio();
	menu = MENU_SOURCE;
	displayMenu();
}

void RAD::switchToAM() {
	source = SOURCE_AM;
	ipod.pause();
	hdradio.bandAM();
	hdradio.muteOff();
	audioswitch.switchToHDRadio();
	menu = MENU_SOURCE;
	displayMenu();
}

void RAD::switchToIPOD() {
	source = SOURCE_IPOD;
	hdradio.muteOn();
	ipod.play();
	audioswitch.switchToIPod();
	menu = MENU_SOURCE;
	displayMenu();
}

void RAD::switchToAUX() {
	source = SOURCE_AUX;
	hdradio.muteOn();
	ipod.pause();
	audioswitch.switchToAux();
	menu = MENU_MAIN;
	displayMenu();
}

void RAD::switchToTONE() {
	menu = MENU_TONE;
	displayMenu();
	mid.display(DEVICE_RAD," ",false);
}

void RAD::switchToDSP() {
	menu = MENU_DSP;
	displayMenu();
	dspc.wakeup();
}

void RAD::switchToOPTIONS() {
	menu = MENU_OPTIONS;
	displayMenu();
}

void RAD::switchToAUDIO() {
	menu = MENU_MAIN;
	displayMenu();
}

void RAD::displayMenu() {

//	dprintf("audio menu = %d source = %d\n\r",menu,source);

	switch(menu) {
	case MENU_MAIN:
		midmenu.set(8," FM     "," AM     "," IPOD   "," AUX    ","","","TONE"," SET");
		midmenu.setActive(source);
		break;
	case MENU_TONE:
		midmenu.set(7,"- BASS +","-TREBLE+","-FADER +","-BALNCE+"," FLAT ","DSP","\xAD");
		break;
	case MENU_DSP:
		dspc.displayMenu();
		break;
	case MENU_OPTIONS:
		midmenu.set(10," SEEK HD"," HDENABL"," IPDCHRG","","","","","","","\xAD");
		if(hdradio.seekHDOnly)
			midmenu.setActive(0);
		if(hdradio.hdenabled)
			midmenu.setActive(1);
		if(ipod.charging)
			midmenu.setActive(2);
		break;
	case MENU_SOURCE:
		switch(source) {
		case SOURCE_FM:
		case SOURCE_AM:
		{
			preset = hdradio.currentPreset();
			midmenu.set(12," 1","2 "," 3","4 "," 5","6 "," 7","8 "," 9","10 "," SC","\xAD");
			if(preset!=-1) {
				midmenu.setActive(preset);
			}
			if(scanmode) {
				midmenu.setActive(10);
			}
		}
		break;
		case SOURCE_IPOD:
			if(ipod.locked)
				midmenu.set(8," PLYLST"," ARTIST"," ALBUM" ," GENRE"," LCK","RND "," ","\xAD");
			else
				midmenu.set(8,"-PLYLST+","-ALBUM +","     ","      "," LCK","RND "," ","\xAD");

			if(ipod.locked) {
				midmenu.setActive(4);
				midmenu.setActive(ipod.mode-1);
			}
			if(ipod.shuffle)
				midmenu.setActive(5);
			break;
		}
		break;
		default:
			dprintf("*** unknown menu value %d\n\r",menu);
	}

	mid.displayMenu(DEVICE_RAD,&midmenu);
	station_displayed=false;
	displaySource();
}

void RAD::debug() {

//	if(debugindex==0) {
//		sprintf(debugbuf,"R:%d O:%d",(int)Serial3.received(),(int)Serial3.overflow());
//		mid.display(DEVICE_RAD,debugbuf,true);
//	}

	mid.display(DEVICE_RAD,dspreset? "DSP OK":"DSP NOT RESET");

	return;

//	debugbuf[0] = 0x23;
//	debugbuf[1] = 0xE0;
//	debugbuf[2] = debugindex-1;
//
//	debugbuf[3]='0'+debugindex-1;
//
//	ibus.sendMessage(DEVICE_RAD,DEVICE_MID,(const uint8_t *)debugbuf,(uint8_t)4);
//
//	debugbuf[0] = 0x23;
//	debugbuf[1] = 0xE0;
//	debugbuf[2] = debugindex-1;
//
//	debugbuf[3]='0'+debugindex-1;
//
//	ibus.sendMessage(DEVICE_RAD,DEVICE_MID,(const uint8_t *)debugbuf,(uint8_t)4);
//
//	debugindex++;

}
