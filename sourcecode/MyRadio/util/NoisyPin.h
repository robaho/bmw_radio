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
#ifndef NoisyInput_h
#define NoisyInput_h

#include <inttypes.h>

class NoisyPin
{

public:
	// Initialize
  NoisyPin(uint8_t pin, unsigned long interval_millis ); 
  int update(); 
  int read();
protected:
  unsigned long  previous_millis, interval_millis, last_update;
  uint8_t state;
  uint8_t pin;
};

#endif


