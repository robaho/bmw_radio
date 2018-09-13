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
#ifndef IPOD_H_
#define IPOD_H_

#include "AdvancedRemote.h"
#include "SimpleRemote.h"
#include "SerialPort.h"

#define IPODTX_PIN 15

class IPOD : public AdvancedRemote::CallbackHandler {
private:
	AdvancedRemote advancedRemote;
	SimpleRemote simpleRemote;

	unsigned long lastActivity;

	long itemcount;
	long itempos;

	unsigned long itemchange;

	long songcount;
	long songpos;

	unsigned long songchange;

	long lastsong;

	unsigned long lastmoderecv;
	bool lastlockmode;

	unsigned long getNameTime;

	void setMode(AdvancedRemote::ItemType mode);
	void changeItem(int val,int offset=1);
	void changeSong(int val);

	void getCurrentItemName();

    void handleFeedback(AdvancedRemote::Feedback feedback, byte cmd);
    void handleItemCount(unsigned long count);
    void handleItemName(unsigned long offet, const char *itemName);
    void handlePlaylistPosition(unsigned long playlistPosition);
    void handleTitle(const char *title);
    void handleArtist(const char *artist);
    void handleAlbum(const char *album);
//    void handlePolling(unsigned long elapsedTimeMs);
    void handleShuffleMode(AdvancedRemote::ShuffleMode mode);
//    void handleRepeatMode(RepeatMode mode);
    void handleCurrentPlaylistSongCount(unsigned long count);
    void handleTimeAndStatus(unsigned long trackLengthInMilliseconds,
                             unsigned long elapsedTimeInMilliseconds,
                             AdvancedRemote::PlaybackStatus status);
    void handleRemoteMode(bool advanced);

	void songUp();
	void songDown();
	void shuffleOn();
	void shuffleOff();
	void lock();
	void unlock();

	void connected();
	void disconnected();

	bool ischanged;

	char title[64+1];
	char album[64+1];
	char artist[64+1];

	void ipod_delay(long ms);

public:
	IPOD();

	bool playing;
    bool shuffle;
    bool isconnected;
    bool locked;
    bool charging;

	AdvancedRemote::ItemType mode;

	void setup();
	void loop();

	void modePlaylist() { setMode(AdvancedRemote::ITEM_PLAYLIST); };
	void modeAlbum() { setMode(AdvancedRemote::ITEM_ALBUM); };
	void modeArtist() { setMode(AdvancedRemote::ITEM_ARTIST); };
	void modeGenre() { setMode(AdvancedRemote::ITEM_GENRE); };

	void itemUpDown(int dir,int offset=1);
	void songUpDown(int dir);

	void play();
	void pause();

	void lockOnOff(bool onoff);
	void shuffleOnOff(bool shuffle);
	void chargingOnOff(bool charging);

	void playlistUpDown(int dir);
	void albumUpDown(int dir);
	void artistUpDown(int dir);

	void displayInformation();

	const char* getTitleArtist();
	bool isChanged();
};

extern IPOD ipod;

#endif /* IPOD_H_ */
