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
#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "WProgram.h"

#define RAD_ADDR 100
#define HDRADIO_ADDR 1000

class Settings {
public:
	uint8_t readByte(int addr);
	void writeByte(int addr,uint8_t val);
	uint16_t readShort(int addr);
	void writeShort(int addr,uint16_t val);
	uint32_t readLong(int addr);
	void writeLong(int addr,uint32_t val);
};

extern Settings settings;

#endif /* SETTINGS_H_ */
