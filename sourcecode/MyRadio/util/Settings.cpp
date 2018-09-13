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
#include "EEPROM.h"
#include "Settings.h"

Settings settings;

uint8_t Settings::readByte(int addr) {
	return EEPROM.read(addr);
}

void Settings::writeByte(int addr,uint8_t val) {
	EEPROM.write(addr,val);
}

uint16_t Settings::readShort(int addr) {
	return (((uint16_t)EEPROM.read(addr)) << 8) | (EEPROM.read(addr+1));
}

void Settings::writeShort(int addr,uint16_t val) {
	EEPROM.write(addr,(val >> 8) & 0xFF);
	EEPROM.write(addr+1,val & 0xFF);
}

uint32_t Settings::readLong(int addr) {
	return (((uint32_t)readShort(addr))<<16) | readShort(addr+2);
}

void Settings::writeLong(int addr,uint32_t val) {
	writeShort(addr,(val >> 16) & 0xFFFF);
	writeShort(addr+2,val & 0xFFFF);
}
