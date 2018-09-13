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
#include "IPOD.h"
#include "util/util.h"
#include "util/printf.h"
#include "util/NoisyPin.h"

IPOD ipod;

//#define DEBUG

#define IPOD_POWER_PIN 8
#define IPOD_CHARGE_PIN 11

static NoisyPin ipodpower = NoisyPin(IPOD_POWER_PIN,200);

void IPOD::handleFeedback(AdvancedRemote::Feedback feedback, byte cmd)
{
	if (feedback != AdvancedRemote::FEEDBACK_SUCCESS)
	{
		advancedRemote.getSongCountInCurrentPlaylist();
		advancedRemote.getPlaylistPosition();
		dprintf("Giving up as cmd 0x%x feedback wasn't SUCCESS, = %d\n\r",cmd,feedback);
		return;
	}

	switch (cmd)
	{
		case AdvancedRemote::CMD_SWITCH_TO_MAIN_LIBRARY_PLAYLIST:
		break;

		case AdvancedRemote::CMD_SWITCH_TO_ITEM:
		break;

		case AdvancedRemote::CMD_PLAYBACK_CONTROL:
		break;

		case AdvancedRemote::CMD_EXECUTE_SWITCH_AND_JUMP_TO_SONG:
			advancedRemote.getSongCountInCurrentPlaylist();
		break;

		case AdvancedRemote::CMD_JUMP_TO_SONG_IN_CURRENT_PLAYLIST:
			advancedRemote.getPlaylistPosition();
		break;

		case AdvancedRemote::CMD_SET_SHUFFLE_MODE:
		break;

		case AdvancedRemote::CMD_SET_REPEAT_MODE:
		break;

		default:
			dprintf("got feedback 0x%2x we didn't expect\n\r",cmd);
		break;
	}
}

void IPOD::handleTimeAndStatus(unsigned long trackLengthInMilliseconds,
        unsigned long elapsedTimeInMilliseconds,
        AdvancedRemote::PlaybackStatus status) {

//	if( != (status==AdvancedRemote::STATUS_PLAYING)playing) {
//		advancedRemote.controlPlayback(AdvancedRemote::PLAYBACK_CONTROL_PLAY_PAUSE);
//	}
}

void IPOD::handleItemCount(unsigned long count) {

	itemcount = count;
}

void IPOD::handleItemName(unsigned long pos, const char *name) {
#ifdef DEBUG
	dmsg("received item name");
#endif
	strncpy(title,name,64);
	strncpy(album,"",64);
	strncpy(artist,"",64);
	ischanged = true;
	getNameTime=0;
}

void IPOD::handleTitle(const char *name) {
	bool changed = strcmp(name,title);
	strncpy(title,name,64);
	if(changed) {
		strncpy(album,"",64);
		strncpy(artist,"",64);
		ischanged = changed;
	}
}

void IPOD::handleArtist(const char *name) {
	bool changed = strcmp(name,artist);
	strncpy(artist,name,64);
	if(changed) {
		ischanged = changed;
	}
}

void IPOD::handleAlbum(const char *name) {
	bool changed = strcmp(name,album);
	strncpy(album,name,64);
	if(changed) {
		ischanged = changed;
	}
}

void IPOD::handleRemoteMode(bool advanced) {
#ifdef DEBUG
	dprintf("setting locked to %d, connected is %d, songchange %ld itemchange %ld\n\r",advanced,isconnected,songchange,itemchange);
#endif
	locked = advanced;
}

void IPOD::handlePlaylistPosition(unsigned long pos) {
	if(pos==-1) {
		lastsong=0xFFFFFFFF;
		songpos=0;
		advancedRemote.jumpToSongInCurrentPlaylist(songpos);
	} else {
		lastsong = songpos = pos;
		advancedRemote.getTitle(songpos);
		advancedRemote.getAlbum(songpos);
		advancedRemote.getArtist(songpos);
	}
}

void IPOD::handleCurrentPlaylistSongCount(unsigned long count) {
	songcount = count;
	advancedRemote.getPlaylistPosition();
}

void IPOD::handleShuffleMode(AdvancedRemote::ShuffleMode mode) {
	if(mode==AdvancedRemote::SHUFFLE_MODE_OFF)
		shuffle = false;
	else
		shuffle = true;
}

IPOD::IPOD() {
	itemcount=songcount=itemchange=songchange=0;
	isconnected=false;
	locked=true;
	getNameTime=0;
	lastActivity=0;
	mode=AdvancedRemote::ITEM_PLAYLIST;
	advancedRemote.setCallbackHandler(*this);
}

void IPOD::setup() {

#ifdef DEBUG
	advancedRemote.setLogPrint(Serial);
	advancedRemote.setDebugPrint(Serial);
	simpleRemote.setLogPrint(Serial);
	simpleRemote.setDebugPrint(Serial);
#endif

	advancedRemote.setSerial(Serial3);
	simpleRemote.setSerial(Serial3);

	advancedRemote.setup();

	unlock();
	pause();

	lastActivity = millis();
	isconnected = false;

	pinMode(IPOD_CHARGE_PIN,OUTPUT);

	chargingOnOff(charging);
}

unsigned long lastout = 0;

void IPOD::loop() {

	advancedRemote.loop();

	unsigned long now = millis();

	ipodpower.update();

	bool pwr = ipodpower.read();

	if(isconnected && !pwr) {
		disconnected();
	} else if(!isconnected && pwr){
		connected();
	}

#ifdef DEBUG
	if(now -lastout > 1000) {
		dprintf("ipod power level = %d\n\r",digitalRead(IPOD_POWER_PIN));
		lastout = now;
	}
#endif

	if(!isconnected)
		return;

	if(!locked)
		return;

	if(itemchange && now-itemchange>5000) {
#ifdef DEBUG
		dprintf("changing item to %d\n\r",itempos);
#endif
		advancedRemote.switchToItem(mode,itempos);
		advancedRemote.executeSwitchAndJumpToSong(0xFFFFFFFF);
		songpos=lastsong=0;
		itemchange=0;
		play();
		advancedRemote.switchToMainLibraryPlaylist();
	}

	if(!itemchange && songchange && now-songchange>3000) {
		if(songpos!=lastsong) {
#ifdef DEBUG
			dprintf("changing song to %d\n\r",songpos);
#endif
			advancedRemote.jumpToSongInCurrentPlaylist(songpos);
			lastsong=songpos;
			play();
		}
		songchange=0;
		itemchange=0;
	}

	if(!itemchange && !songchange && (now - lastActivity > 5000)) {
		advancedRemote.getSongCountInCurrentPlaylist();
		advancedRemote.getPlaylistPosition();
		lastActivity = millis();
	}
}

void IPOD::connected() {

	isconnected = true;

	delay(3000);

	dprintf("iPhone/iPod connected, power pin = %d\n\r",ipodpower.read());
	Serial3.flush();

	lockOnOff(lastlockmode);

	if(lastlockmode)
		shuffleOnOff(shuffle);

	if(playing)
		play();
	else
		pause();

	ischanged=true;
}

void IPOD::disconnected() {
	isconnected = false;
	Serial3.flush();
	dprintf("iPhone/iPod disconnected, power pin = %d\n\r",ipodpower.read());

	strcpy(title,"");
	strcpy(artist,"");

	locked=false;
	ischanged=true;
}

void IPOD::itemUpDown(int dir,int offset) {
	dir==1 ? changeItem(+1,offset) : changeItem(-1,offset);
}

void IPOD::playlistUpDown(int dir) {
	dir==1 ? simpleRemote.sendNextPlaylist() : simpleRemote.sendPreviousPlaylist();
	simpleRemote.sendButtonReleased();
}

void IPOD::albumUpDown(int dir) {
	dir==1 ? simpleRemote.sendNextAlbum() : simpleRemote.sendPreviousAlbum();
	simpleRemote.sendButtonReleased();
}

void IPOD::changeItem(int val,int offset) {
	if(songchange)
		return;

	long now = millis();
	if(now-getNameTime<5000)
		return;
	getNameTime=now;

	long temp = itempos;

	itempos += (val*offset);

	if(itempos>=itemcount)
		itempos=0;
	if(itempos<0)
		itempos=itemcount-1;

	getCurrentItemName();

	lastActivity = millis();

	if(temp==itempos)
		return;

	itemchange = lastActivity;

//	if(mode==AdvancedRemote::ITEM_PLAYLIST)
//		lastplaylist = itempos;
}

void IPOD::songUpDown(int dir) {
	dir==1 ? songUp() : songDown();
}

void IPOD::songUp() {
	if(locked)
		changeSong(+1);
	else {
		simpleRemote.sendSkipForward();
		simpleRemote.sendButtonReleased();
	}
}

void IPOD::changeSong(int val) {
	if(itemchange)
		return;

	strcpy(album,"");
	strcpy(artist,"");

	songpos += val;
	if(songpos>=songcount)
		songpos = 0;
	if(songpos<0)
		songpos = songcount-1;
	if(songpos<0)
		songpos = 0;

	advancedRemote.getTitle(songpos);

	songchange = lastActivity = millis();
}

void IPOD::songDown() {
	if(locked)
		changeSong(-1);
	else {
		simpleRemote.sendSkipBackward();
		simpleRemote.sendButtonReleased();
	}
}

void IPOD::setMode(AdvancedRemote::ItemType type) {

	if(mode!=type) {
		advancedRemote.switchToMainLibraryPlaylist();
		itempos=0;
	}

	mode = type;

	if(!locked)
		return;

	strcpy(title,"");
	strcpy(album,"");
	strcpy(artist,"");

	itemcount=-1;
	unsigned long now=millis();
	while(itemcount==-1 && (millis()-now<5000)) {
		advancedRemote.getItemCount(mode);
		ipod_delay(500);
	}

	getCurrentItemName();

	itemchange=lastActivity=millis();

	ischanged=true;
}

void IPOD::play() {
	playing = true;
	simpleRemote.sendJustPlay();
	simpleRemote.sendButtonReleased();
}

void IPOD::pause() {
	playing = false;
	simpleRemote.sendJustPause();
	simpleRemote.sendButtonReleased();
}

void IPOD::shuffleOnOff(bool onoff) {
	onoff ? shuffleOn() : shuffleOff();

}
void IPOD::shuffleOn() {
	shuffle = true;
	if (locked) {
		advancedRemote.setShuffleMode(AdvancedRemote::SHUFFLE_MODE_SONGS);
		advancedRemote.getPlaylistPosition();
	} else {
		simpleRemote.sendToggleShuffle();
		simpleRemote.sendButtonReleased();
	}
}

void IPOD::shuffleOff() {
	shuffle = false;
	if (locked) {
		advancedRemote.setShuffleMode(AdvancedRemote::SHUFFLE_MODE_OFF);
		advancedRemote.getPlaylistPosition();
	} else {
		simpleRemote.sendToggleShuffle();
		simpleRemote.sendButtonReleased();
	}
}

void IPOD::lock() {

	long now = millis();

	if(!isconnected) {
		return;
	}

	while(!locked) {
		advancedRemote.enable();
		ipod_delay(500);
		advancedRemote.getRemoteMode();
		ipod_delay(500);
		if(millis()-now>5000) {
			locked=true;
			unlock();
			return;
		}
	}

	play();

	advancedRemote.switchToMainLibraryPlaylist();
	advancedRemote.getItemCount(mode);
	advancedRemote.getSongCountInCurrentPlaylist();
	advancedRemote.getPlaylistPosition();
	advancedRemote.getShuffleMode();
	advancedRemote.setRepeatMode(AdvancedRemote::REPEAT_MODE_OFF);

	ischanged=true;
}

void IPOD::unlock() {

	long now = millis();

	while(locked) {
		advancedRemote.disable();
		advancedRemote.getRemoteMode();
		ipod_delay(500);
		if(millis()-now>2000)
			break;
	}

	locked=false;

	if(playing)
		play();

	itempos=0;
	itemchange=songchange=0;
	mode=AdvancedRemote::ITEM_PLAYLIST;

	ischanged=true;
}

void IPOD::lockOnOff(bool onoff) {
	lastlockmode = onoff;
	onoff ? lock() : unlock();
}

void IPOD::getCurrentItemName() {
#ifdef DEBUG
	dprintf("getting item name: mode = %d, itempos = %d\n\r",mode,itempos);
#endif

	advancedRemote.getItemNames(mode, itempos, 1);
}

void IPOD::chargingOnOff(bool b) {
	charging = b;
	digitalWrite(IPOD_CHARGE_PIN,charging ? LOW : HIGH); // PNP transistor so 0V to turn ON
}

bool IPOD::isChanged() {
	bool temp = ischanged;
	ischanged = false;
	return temp;
}

char ipod_buffer[128];

const char *IPOD::getTitleArtist() {
	if(!isconnected)
		return "NO IPOD";

	if(!locked) {
		return "IPOD";
	}

	char * cp = ipod_buffer;

	strcpy(cp,"");

	if(notempty(title)) {
		strcpy(cp,title);

		if(notempty(artist)) {
			strcat(cp," / ");
			strcat(cp,artist);
		}
	}

	return ipod_buffer;
}

void IPOD::displayInformation() {
	dmsg("Title",ipod.title);
	if (strlen(ipod.artist) > 0) {
		dmsg("artist",ipod.artist);
	}
	if (strlen(ipod.album) > 0) {
		dmsg("album",ipod.album);
	}
	dprintf("song %d / %d item %d / %d changed %d shuffle %d locked %d mode %d overflow %d\n\r",
			songpos,songcount,itempos,itemcount,ischanged,shuffle,locked,mode,Serial1.overflow());
}

void IPOD::ipod_delay(long ms) {
	unsigned long end = millis()+ms;

	while(millis()<end) {
		advancedRemote.loop();
	}
}
