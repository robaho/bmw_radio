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
#include "AudioSwitch.h"
#include "WProgram.h"
#include "util/printf.h"
#include "ibusdev/RAD.h"

//#define DEBUG

AudioSwitch audioswitch;

#define CNTRL1_PIN 5
#define CNTRL2_PIN 6
#define MUTE_PIN 7

void AudioSwitch::setup() {
	pinMode(CNTRL1_PIN,OUTPUT);
	pinMode(CNTRL2_PIN,OUTPUT);
	pinMode(MUTE_PIN,OUTPUT);
	switchToMute();
}
void AudioSwitch::switchToHDRadio() {
#ifdef DEBUG
	dmsg("audio switch: HD RADIO");
#endif
	// output #1
	digitalWrite(CNTRL1_PIN,LOW);
	digitalWrite(CNTRL2_PIN,LOW);
	digitalWrite(MUTE_PIN,HIGH);
}
void AudioSwitch::switchToIPod() {
#ifdef DEBUG
	dmsg("audio switch: IPOD");
#endif
	// output #2
	digitalWrite(CNTRL1_PIN,HIGH);
	digitalWrite(CNTRL2_PIN,LOW);
	digitalWrite(MUTE_PIN,HIGH);
}
void AudioSwitch::switchToAux() {
#ifdef DEBUG
	dmsg("audio switch: AUX");
#endif
	// output #3
	digitalWrite(CNTRL1_PIN,HIGH);
	digitalWrite(CNTRL2_PIN,HIGH);
	digitalWrite(MUTE_PIN,HIGH);
}
void AudioSwitch::switchToMute() {
#ifdef DEBUG
	dmsg("audio switch: MUTE");
#endif
	digitalWrite(MUTE_PIN,LOW);
}

void AudioSwitch::switchToSource(int _source) {
	switch(_source) {
			case SOURCE_FM:
			case SOURCE_AM:
				switchToHDRadio(); break;
			case SOURCE_IPOD:
				switchToIPod(); break;
			case SOURCE_AUX:
				switchToAux(); break;
	}
}
