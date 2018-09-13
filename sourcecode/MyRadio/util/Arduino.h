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
#ifndef ARDUINO_H_
#define ARDUINO_H_

#define LEDPIN 13
#define ERRORLEDPIN 12

class Arduino {
public:
	void setup();
	void loop();
	void blinkLED(long ms);
	void blinkErrorLED(long ms);
	void blinkError(int count);
	void setErrorLED(int state);
};

extern Arduino arduino;

#endif /* ARDUINO_H_ */
