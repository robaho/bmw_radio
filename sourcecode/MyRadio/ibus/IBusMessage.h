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

#ifndef IBUSMESSAGE_H_
#define IBUSMESSAGE_H_

class IBusMessage
{
	friend class IBus;
public:
	IBusMessage();
	uint8_t *data;
	uint8_t src;
	uint8_t dst;
	uint8_t len; // length of data valid array
};

#endif
