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
#include "util/Arduino.h"
#include "ibus/IBus.h"
#include "ibusdev/TEL.h"
#include "ibusdev/IKE.h"
#include "ibusdev/MID.h"
#include "ibusdev/RAD.h"
#include "ibusdev/DSPC.h"
#include "hdradio/HDRadio.h"
#include "ipod/IPOD.h"
#include "switch/AudioSwitch.h"
#include "util/printf.h"

//#define DEBUG

void setup() {
	Serial.begin(57600); // used for console debugging/messages

	arduino.setup();
	ibus.setup();
	rad.setup();
	audioswitch.setup();

#ifdef DEBUG
	ibus.setCheckCTS(false);
#endif
}

void memoryTest() {
	int byteCounter = 0; // initialize a counter
	byte *byteArray; // create a pointer to a byte array
	// More on pointers here: http://en.wikipedia.org/wiki/Pointer#C_pointers

	// use the malloc function to repeatedly attempt allocating a certain number of bytes to memory
	// More on malloc here: http://en.wikipedia.org/wiki/Malloc
	while ((byteArray = (byte*) malloc(byteCounter * sizeof(byte))) != NULL) {
		byteCounter++; // if allocation was successful, then up the count for the next try
		free(byteArray); // free memory after allocating it
	}

	free(byteArray); // also free memory after the function finishes
	//  return byteCounter; // send back the highest number of bytes successfully allocated

	dprintf("Memory test results: %d bytes free\n\r,byteCounter");
}

unsigned long timer;

uint8_t press[]  = {0x31,0x40,0x01,0xFF};
uint8_t press10[] = {0xC0,0x06,0x68,0x31,0xC0,0x00,0x4A,0xFF};
uint8_t press11[] = {0xC0,0x06,0x68,0x31,0x80,0x00,0x4B,0xFF};
uint8_t press12[] = {0xC0,0x06,0x68,0x31,0xC0,0x00,0x4C,0xFF};
uint8_t press13[] = {0xC0,0x06,0x68,0x31,0x80,0x00,0x4D,0xFF};

uint8_t devicestatus[] = { 0xC0,0x03,0x68,0x01,0xFF };
uint8_t displaystatus[] = { 0xC0,0x05,0xFF,0x20,0x20,0xB0,0xFF };

uint8_t next[] = { 0x50,0x04,0x68,0x3B,0x21,0xFF };
uint8_t prev[] = { 0x50,0x04,0x68,0x3B,0x28,0xFF };

uint8_t nextlong[] = { 0x50,0x04,0x68,0x3B,0x11,0xFF };

void handleInput();

void loop() {

	arduino.loop();
	ibus.loop();

	rad.loop();
	ike.loop();
	mid.loop();
	dspc.loop();

	if(millis()-timer>10000) {
		arduino.blinkErrorLED(50);
		timer = millis();
#ifdef DEBUG
		dmsg("running");
#endif
	}

	handleInput();
}

void handleInput() {
	if(Serial.available()) {
		char c = Serial.read();
		switch(c){
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			press[3] = ((c-'0') | 0x40); // use release button
			ibus.inject(DEVICE_MID,DEVICE_RAD,press,sizeof(press));
			break;
		case '-':
			ibus.inject(press10);
			break;
		case '=':
			ibus.inject(press11);
			break;
		case '_':
			ibus.inject(press12);
			break;
		case '+':
			ibus.inject(press13);
			break;
		case 's':
			ibus.inject(devicestatus);
			break;
		case 'S':
			ipod.playing ? ipod.pause() : ipod.play();
			break;
		case 'd':
			ibus.inject(displaystatus);
			break;
		case 'n':
			ibus.inject(next);
			break;
		case 'p':
			ibus.inject(prev);
			break;
		case 'N':
			hdradio.seekUpDown(1);
			break;
		case 'P':
			hdradio.seekUpDown(-1);
			break;
		case 'q':
			ibus.inject(nextlong);
			break;
		case 'w':
			ibus.inject(next);
			break;
		case 'a':
			audioswitch.switchToMute();
			break;
		case 'b':
			audioswitch.switchToIPod();
			break;
		}
	}
}

int main(void) {
	init();

	setup();

	memoryTest();

	for (;;)
		loop();

	return 0;
}

extern "C" void __cxa_pure_virtual()
{
	cli();
	arduino.setErrorLED(HIGH);
	for (;;);
}

