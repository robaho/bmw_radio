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

#include "TEL.h"
#include "IKE.h"
#include "MID.h"
#include "MFL.h"
#include "util/Arduino.h"


TEL tel;

void TEL::handleMessage(IBusMessage* m) {
	if(m->src!=DEVICE_MFL)
		return;

	if(memcmp(m->data,TEL_PRESS_LONG,sizeof(TEL_PRESS_LONG))==0)
		voicePress();

//	if(memcmp(m->data,TEL_RELEASE,sizeof(TEL_RELEASE))==0)
//		voiceRelease();
}

void TEL::voicePress() {
	// redirect long press as press of "face"

	ibus.sendMessage(DEVICE_MFL,DEVICE_SES,TEL_PRESS,sizeof(TEL_PRESS));

	voiceOn = true;
}

void TEL::voiceRelease() {

	if(voiceOn) {
		ibus.sendMessage(DEVICE_MFL,DEVICE_SES,TEL_RELEASE,sizeof(TEL_RELEASE));
		voiceOn = false;
	}
}
