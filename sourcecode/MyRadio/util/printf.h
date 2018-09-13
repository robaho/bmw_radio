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
#ifndef __PRINTF__
#define __PRINTF__

#include <stdarg.h>

void tiny_sprintf(char* s,const char *fmt, ...);
void tiny_dprintf(const char *fmt, ...);

void ddump(const char*title,byte *data,int datalen);
void dmsg(const char*msg);
void dmsg(const char*msg1,const char*msg2);

#define sprintf tiny_sprintf
#define dprintf tiny_dprintf

#endif
