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
#ifndef IKE_H_
#define IKE_H_

#include "ibus/IBusDevice.h"

class IKE : public IBusDevice
{
	unsigned long scrollTime;
	const char *scrollText;
	int scrollPos;
	uint8_t scrollSource;

	void displayNoScroll(uint8_t source,const char *text);
public:
	void loop();
	void display(uint8_t source,const char *text);
};

extern IKE ike;

#endif /* IKE_H_ */
