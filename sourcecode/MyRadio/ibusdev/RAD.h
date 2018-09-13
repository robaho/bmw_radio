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
#ifndef RAD_H_
#define RAD_H_

#include "ibus/IBusDevice.h"
#include "MID.h"
#include "util/Settings.h"

#define SOURCE_FM 	0
#define SOURCE_AM 	1
#define SOURCE_IPOD 2
#define SOURCE_AUX  3

#define MENU_MAIN    0
#define MENU_SOURCE  1
#define MENU_TONE    2
#define MENU_OPTIONS 3
#define MENU_DSP	 4

#define TONE_BASS    	0
#define TONE_TREBLE  	1
#define TONE_FADER   	2
#define TONE_BALANCE 	3
#define TONE_COUNT		4

#define SETTING_UPDOWN_MODE 0
#define UPDOWN_MODE_SEEK	0
#define UPDOWN_MODE_PRESET	1

#define SETTING_SEEK_MODE	1
#define SEEK_MODE_HD		0
#define SEEK_MODE_ALL		1

#define SETTING_CHECKSUM (RAD_ADDR)
#define SETTING_POWER	 (RAD_ADDR+1)
#define SETTING_SOURCE	 (RAD_ADDR+2)
#define SETTING_TONE_BASS	 (RAD_ADDR+3)
#define SETTING_TONE_TREBLE	 (RAD_ADDR+4)
#define SETTING_TONE_BALANCE (RAD_ADDR+5)
#define SETTING_TONE_FADER	 (RAD_ADDR+6)
#define SETTING_SEEK_HD_ONLY (RAD_ADDR+7)
#define SETTING_HD_ENABLED (RAD_ADDR+8)
#define SETTING_IPOD_CHARGE (RAD_ADDR+9)
#define SETTING_VOLUME (RAD_ADDR+10)

class RAD : public IBusDevice {
	byte source; /* 0 = tuner_fm, 1 = tuner_am, 2 = ipod, 3 = aux */
	byte menu;   /* 0 = main, 1 = audio (by source), 2 = tone, 3 = options */

	int tone[TONE_COUNT];

	byte dspmode;
	bool dspreset;

	unsigned long lastcheck;
	unsigned long lastupdate;

	bool station_displayed;

	bool longmode; // true if the MID is in audio mode

	int  seekingDir;
	unsigned long seekstart;
	unsigned long lastseek;
	unsigned int seekdelay;
	bool scanmode;

	unsigned long MFLtune; // tune initiation from MFL time
	int	 preset;
	unsigned long lastMIDinput;
	bool sourceswitched;
	int  volume;

	bool ispowerOn;
	unsigned long powerOnDelay;

	bool muted;

	void muteOn();
	void muteOff();

	void handleMID(IBusMessage *m);
	void handleMFL(IBusMessage *m);
	void handleDSP(IBusMessage *m);
	void handleMIDbutton(int num,bool press,bool presslong,bool release);
	bool handleMIDtune(int num,bool _press,bool _presslong,bool _release);

	void updateDSP(bool after_reset);
	void sendVolume();
	void updateTONE();

	void displayMenu();
	void displaySource();

	void switchToSource(int source);

	void switchToFM();
	void switchToAM();
	void switchToIPOD();
	void switchToAUX();
	void switchToTONE();
	void switchToDSP();
	void switchToOPTIONS();
	void switchToAUDIO();

	void tuneUpDown(int dir);
	void seekUpDown(int dir);
	void toneUpDown(int index, int dir);

	void debug();
	void saveSettings();

	void powerOn();
	void powerOff();
	void poweredOn();

public:
	RAD();
	void setup();
	void loop();
	void handleMessage(IBusMessage *m);

	bool istuner() { return (source==SOURCE_FM || source==SOURCE_AM); }
	bool isipod() { return source==SOURCE_IPOD; }
};

extern RAD rad;

#endif
