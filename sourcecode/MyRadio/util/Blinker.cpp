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
#include "Blinker.h"

Blinker::Blinker(int pin)
{
	this->pin = pin;
	pinMode(pin, OUTPUT);
}

void Blinker::blink(long ms) {
	if(on)
		offtime += ms;
	else {
		on = true;
		offtime = millis() + ms;
		digitalWrite(pin,HIGH);
	}
}

void Blinker::maybeOff(long now)
{
	if(on && (now > offtime)){
		digitalWrite(pin,LOW);
		on = false;
	}
}
