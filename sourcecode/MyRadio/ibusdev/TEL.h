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
#ifndef TEL_H_
#define TEL_H_

#include "ibus/IBusDevice.h"
#include "MID.h"

class TEL : public IBusDevice
{
	boolean voiceOn;

	void voicePress();
	void voiceRelease();
public:
	void handleMessage(IBusMessage *m);
};

extern TEL tel;

#endif
