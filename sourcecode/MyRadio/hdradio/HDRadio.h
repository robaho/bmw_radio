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
#ifndef HDRADIO_H_
#define HDRADIO_H_

#include "hddefs.h"

#define STATION(freq,sub) (((sub<<12)&0xF000) | freq)
#define FREQ(station) (station & 0x0FFF)
#define SUBCHANNEL(station) ((station>>12) & 0x000F)
#define NUM_PRESETS 10

class HDRadio
{
	friend class HDListen;
	friend class HDCommands;

private:
	unsigned char  lastBAND;
	uint16_t lastStation[2]; // one for each band
	uint16_t presets[2][NUM_PRESETS]; // one bank for each band

	unsigned long timer;

	bool power;
	bool ischanged;
	bool ismuted;

	void poweredOn();
	void markChanged();

	char station[64+1];
	char callsign[64+1];
	char title[64+1];
	char artist[64+1];

	void tune(int band,int frequency,int subchannel=0);

	int band;
	int frequency;
	int subchannel;
	int subchannelbit;
	unsigned char subchannelmask;

	bool hdactive;
	bool hdstreamlock;
	bool mute;

	int signalstrength;

	void loadPresets();
	void savePresets();

public:
	HDRadio();
	void setup();
	void loop();

	bool isChanged();
	bool isMuted() { return ismuted; }

	void seekUpDown(int dir);
	void tuneUpDown(int dir);

	bool seekHDOnly;
	bool hdenabled;

	void bandAM();
	void bandFM();

	void preset(int n);
	int  currentPreset();
	void savepreset(int n);
	void presetUpDown(int dir);

	void powerOn();
	void powerOff();
	bool isOn();

	void muteOn();
	void muteOff();

	void HDEnabled(bool b);

	void displayInformation();

	const char *getStation(bool longmode, bool showsignal);
	const char* getTitleArtist();

	int getSignalStrength() { return signalstrength; }
};

extern HDRadio hdradio;

#endif /* HDRADIO_H_ */
