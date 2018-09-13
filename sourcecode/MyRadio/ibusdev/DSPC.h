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
#ifndef DSPC_H_
#define DSPC_H_

#include "ibus/IBusDevice.h"

enum FREQUENCY { F_80HZ, F_200HZ, F_500HZ, F_1KHZ, F_2KHZ, F_5KHZ, F_12KHZ };
enum MODE { CONCERT_HALL, JAZZ_CLUB, CATHEDRAL, MEMORY1, MEMORY2, MEMORY3, DSP_OFF  };
enum STATE { S_MODE, S_REVERB, S_ROOMSIZE, S_FREQUENCY };

class DSPC : public IBusDevice
{
	int boost[7];
	int	 reverb;
	int  roomsize;

	FREQUENCY curfreq;
	MODE mode;
	STATE state;
	int memory; // 0 = MEMORY 1, 1 = MEMORY 2, 2 = MEMORY 3

	bool reset;
	bool needMemoryInfo;
	unsigned long lastrequest;

	void displayDSP();
	bool isMemory() { return mode >= MEMORY1 && mode <= MEMORY2; }
	void flat();

public:
	DSPC() { mode = MEMORY1; curfreq = F_80HZ; state=S_MODE; };
	void loop();
	void handleMIDbutton(int button);
	void handleMessage(IBusMessage *);
	void wakeup();
	void powerOn();
	void displayMenu();
};

extern DSPC dspc;

#endif
