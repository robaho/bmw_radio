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
#include "Arduino.h"
#include "Blinker.h"

Arduino arduino;

// LED connected to digital pin 13

Blinker led = Blinker(LEDPIN);
Blinker error = Blinker(ERRORLEDPIN);

void Arduino::setup() {
}

void Arduino::blinkLED(long ms) {
	led.blink(ms);
}

void Arduino::blinkErrorLED(long ms) {
	error.blink(ms);
}

void Arduino::blinkError(int count) {
	for(int i=0;i<count;i++) {
		setErrorLED(HIGH);
		delay(30);
		setErrorLED(LOW);
		delay(30);
	}
}

void Arduino::loop()
{
	long now = millis();

	led.maybeOff(now);
	error.maybeOff(now);
}

void Arduino::setErrorLED(int state)
{
	digitalWrite(ERRORLEDPIN,state);
}
