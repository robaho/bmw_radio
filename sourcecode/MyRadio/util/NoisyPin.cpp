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
#include "NoisyPin.h"
#include "printf.h"

//#define DEBUG
#define DEBUG_PIN = 8

NoisyPin::NoisyPin(uint8_t pin, unsigned long interval_millis) {
	this->interval_millis = interval_millis;
	last_update = previous_millis = millis();
	state = digitalRead(pin);
	this->pin = pin;
}

int NoisyPin::update() {

	if(millis()-last_update>100) {
#ifdef DEBUG
		if(pin==DEBUG_PIN)
			dprintf("millis %d since update\n\r",(int)(millis()-last_update));
#endif
		previous_millis = millis();
	}

	last_update=millis();

	int now = digitalRead(pin);
	if(now!=state) {
#ifdef DEBUG
		if(pin==DEBUG_PIN && millis()%100==0)
			dprintf("pin %d state = %d now = %d\n\r",pin,state,now);
#endif
		if(millis()-previous_millis > interval_millis) {
			state = now;
			previous_millis = millis();
			return 1;
		}
	}
	else {
#ifdef DEBUG
		if(pin==DEBUG_PIN && millis()%100==0)
			dprintf("pin %d state = %d now = %d RESET\n\r",pin,state,now);
#endif
		previous_millis=millis();
	}

	return 0;

}

int NoisyPin::read() {
	return (int) state;
}
