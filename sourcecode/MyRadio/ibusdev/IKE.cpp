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
#include "IKE.h"
#include "scroll.h"
#include "util/printf.h"

#define MAX_IKE_DISPLAY 20

//#define DEBUG

IKE ike;

uint8_t ikebuffer[128];

void IKE::display(uint8_t source,const char *text)
{
	scrollTime = 0;

	if(strlen(text) > MAX_IKE_DISPLAY)
	{
		scrollPos = 0;
		scrollTime = millis()+SCROLLTIME*6;
		scrollText = text;
		scrollSource = source;
	}

	displayNoScroll(source,text);
}

void IKE::displayNoScroll(uint8_t source,const char *text)
{
	int len = min(strlen(text),MAX_IKE_DISPLAY);

	ikebuffer[0] = 0x23;
	ikebuffer[1] = 0x42;
	ikebuffer[2] = (len<MAX_IKE_DISPLAY) ? 0x20 : 0x00;

	memcpy(ikebuffer+3,text,len);

	ibus.sendMessage(source,DEVICE_IKE,ikebuffer,len+3);

#ifdef DEBUG
	dprintf("IKE: %s\n\r",text);
#endif
}

void IKE::loop() {
	if(scrollTime==0 || millis()<scrollTime)
		return;

	if(scrollPos==999) {
		displayNoScroll(scrollSource, scrollText);
		scrollTime=0;
	} else if(strlen(scrollText)-scrollPos < MAX_IKE_DISPLAY) {
		scrollPos = strlen(scrollText)-MAX_IKE_DISPLAY;
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
