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
#ifndef MFL_H_
#define MFL_H_

#include "WProgram.h"

const uint8_t TEL_PRESS[] = { 0x3B, 0x80 };
const uint8_t TEL_PRESS_LONG[] = { 0x3B, 0x90 };
const uint8_t TEL_RELEASE[] = { 0x3B, 0xA0 };

const uint8_t VOLUME_UP[] = { 0x32, 0x11 };
const uint8_t VOLUME_DOWN[] = { 0x32, 0x10 };
const uint8_t NEXT_PRESS[] = { 0x3B, 0x01 };
const uint8_t NEXT_PRESS_LONG[] = { 0x3B, 0x11 };
const uint8_t NEXT_RELEASE[] = { 0x3B, 0x21 };
const uint8_t PREV_PRESS[] = { 0x3B, 0x08 };
const uint8_t PREV_PRESS_LONG[] = { 0x3B, 0x18 };
const uint8_t PREV_RELEASE[] = { 0x3B, 0x28 };
const uint8_t RADIO_TELEPHONE[] = { 0x01 };

#endif
