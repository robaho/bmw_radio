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
#ifndef MID_H_
#define MID_H_

#include "ibus/IBusDevice.h"


class Menu
{
	friend class MID;
	char button[12][8+1];
	int nbuttons;
public:
	Menu();
	void set(int nbuttons,...);
	void setActive(int button);
};

class MID : public IBusDevice
{
	uint8_t midbuffer[48];

	char lastclock[16]; // contains last clock display text from IKE to MID

	unsigned long scrollTime;
	const char *scrollText;
	int scrollPos;
	uint8_t scrollSource;
	int debugindex;
	unsigned int maxtopleft;

	void displayNoScroll(uint8_t source,const char* text);
public:
	MID() { strcpy(lastclock,""); debugindex = 0; }
	void loop();
	void display(uint8_t source,const char* text,bool longmode=true);
	void displayClock(uint8_t source,const char* text);
	void displayMenu(uint8_t source,Menu *menu);
	bool isScrolling();
	void clear(uint8_t source);
	void debug(int index);
	void handleMessage(IBusMessage *m);
};

extern MID mid;
extern Menu midmenu; // global menu

#endif /* MID_H_ */
