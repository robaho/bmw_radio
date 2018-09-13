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
#include "ibus/IBus.h"
#include "MID.h"
#include "scroll.h"
#include "util/printf.h"

MID mid;

#define MAX_TOP_LEFT 24
#define SHORT_TOP_LEFT 11
#define MAX_TOP_RIGHT 20
#define MAX_BUTTON_LABEL 4

#define CLEAREOL 0x20
#define MORETEXT 0x40

#define NEXTBUTTON 0x04;
#define NEXTFULL 0x02;
#define NEXTHALF 0x01;

//#define DEBUG

Menu midmenu;

void MID::display(uint8_t source,const char* text, bool longmode)
{
	scrollTime = 0;
	maxtopleft = longmode ? MAX_TOP_LEFT : SHORT_TOP_LEFT;

//	const char* cp = text;
//	dprintf("MID TEXT: %s\n\r",cp);
//	dprintf("MID HEX : ");
//	while(*cp!=0) {
//		dprintf("%02x ",*cp);
//		cp++;
//	}
//	dprintf("\n\r");

	if(strlen(text) > maxtopleft)
	{
		scrollPos = 0;
		scrollTime = millis()+SCROLLTIME*6;
		scrollText = text;
		scrollSource = source;
	}

	displayNoScroll(source,text);
}

void MID::displayNoScroll(uint8_t source,const char* text)
{
	int len = min(strlen(text),maxtopleft);

	midbuffer[0] = 0x23;
	midbuffer[1] = (maxtopleft==MAX_TOP_LEFT ? 0xE0 : 0x00); // allows use of entire line, but we only use 24
	midbuffer[2] = 0x20; // clear

	memcpy(midbuffer+3,text,len);
	memset(midbuffer+3+len,' ',maxtopleft-len);

#ifdef DEBUG
	dmsg("MIDMAIN",text);
#endif

	ibus.sendMessage(source,DEVICE_MID,midbuffer,maxtopleft+3);

	int clocklen = strlen(lastclock);
	if(maxtopleft==MAX_TOP_LEFT && clocklen>0) {
		midbuffer[0] = 0x23;
		midbuffer[1] = 0xE0; // allows use of entire line, but we only use 24
		midbuffer[2] = 0x80;
		midbuffer[3]=' ';

		memcpy(midbuffer+4,lastclock,clocklen);
		ibus.sendMessage(source,DEVICE_MID,midbuffer,clocklen+4);
	}
}

void MID::clear(uint8_t source) {

	midbuffer[0] = 0x21;
	midbuffer[1] = 0x00;
	midbuffer[2] = 0x04;
	midbuffer[3] = 0x20;

	ibus.sendMessage(source,DEVICE_MID,midbuffer,4);

	midbuffer[0] = 0x23;
	midbuffer[1] = 0x00;
	midbuffer[2] = 0x20;

	ibus.sendMessage(source,DEVICE_MID,midbuffer,3);

	scrollTime=0;

#ifdef DEBUG
	dmsg("MID CLEARED");
#endif
}

void MID::displayClock(uint8_t source,const char* text)
{
	int len = min(strlen(text),MAX_TOP_RIGHT);

	midbuffer[0] = 0x23;
	midbuffer[1] = 0x00;
	midbuffer[2] = 0x22;

	memcpy(midbuffer+3,text,len);

	ibus.sendMessage(source,DEVICE_MID,midbuffer,len+3);
}

void MID::handleMessage(IBusMessage *m) {
	if(m->dst!=DEVICE_ANZV)
		return;
	if(m->src!=DEVICE_IKE)
		return;
	if(m->data[0]==0x24 && m->data[1]==0x01) { // display clock text
		int tlen = m->len-3;
		memcpy(lastclock,m->data+3,tlen);
		lastclock[tlen]=0;
	}
}

void MID::displayMenu(uint8_t source,Menu *menu)
{
	midbuffer[0] = 0x21;
	midbuffer[1] = 0x01;
	midbuffer[2] = 0x00;
	midbuffer[3] = CLEAREOL;

	int len=4;
	int button=0;
	int startbutton=0;
	bool lastfull=false;

#ifdef DEBUG
	dprintf("MIDMENU: debug=%d :",debugindex);
	for(int i=0;i<menu->nbuttons;i++) {
		dprintf("B%d %s ",i,menu->button[i]);
	}
	dmsg("");
#endif

	for(int i=0;i<menu->nbuttons;i++) {
		const char* cp = menu->button[i];
		int cplen = strlen(cp);
		int needed = ((cplen>4) ? 9 : 5);
		if(len+needed>(30-4)) {
			midbuffer[3] = ((startbutton==0) ? 0x60 : 0x40) | startbutton;
			ibus.sendMessage(source,DEVICE_MID,midbuffer,len);
			startbutton = button;
			len = 4;
			lastfull=false;
		}
		if(cplen>4) {
			midbuffer[len++]=0x06;
			memcpy(midbuffer+len,cp,cplen);
			len+=cplen;
			for(int j=cplen;j<8;j++) {
				midbuffer[len++]=' ';
			}
			button+=2;
			lastfull=true;
		} else {
			if(lastfull)
				midbuffer[len++]=0x05;
			memcpy(midbuffer+len,cp,cplen);
			len+=cplen;
			midbuffer[len++]=0x05;
			button++;
			lastfull=false;
		}
	}
	midbuffer[3] = startbutton;
	ibus.sendMessage(source,DEVICE_MID,midbuffer,len);
}

void MID::loop()
{
	if(scrollTime==0 || millis()<scrollTime)
		return;

	if(scrollPos==999) {
		displayNoScroll(scrollSource, scrollText);
		scrollTime=0;
	} else if(strlen(scrollText)-scrollPos < maxtopleft) {
		scrollPos = strlen(scrollText)-maxtopleft;
		// leave trailing message up a while
		displayNoScroll(scrollSource, scrollText+scrollPos);
		scrollPos=999;
		scrollTime = millis()+SCROLLTIME*6;
	} else {
		displayNoScroll(scrollSource, scrollText+scrollPos);
		scrollPos+=SCROLLX;
		scrollTime = millis()+SCROLLTIME;
	}
}

Menu::Menu(){
	nbuttons=0;
}

void Menu::set(int nargs,...) {
	va_list varg;
	va_start(varg,nargs);

	for(int i=0;i<nargs;i++) {
		const char * cp = va_arg(varg, const char *);
		int len = min(8,strlen(cp));
		strncpy(button[i],cp,len);
		button[i][len]=0;
	}

	nbuttons = nargs;
}

void Menu::setActive(int num) {
	int len = strlen(button[num]);
	for(int i=0;i<len;i++) {
		if(button[num][i]==' ') {
			button[num][i]='*';
			return;
		}
	}
	strcpy((button[num]+len),"*");
}

bool MID::isScrolling() {
	return scrollTime!=0;
}
