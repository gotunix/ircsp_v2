/*
                  ___   ____     ____   ____    ____
                 |_ _| |  _ \   / ___| / ___|  |  _ \
                  | |  | |_) | | |     \___ \  | |_) |
                  | |  |  _ <  | |___   ___) | |  __/
                 |___| |_| \_\  \____| |____/  |_|

                INTERNET RELAY CHAT SERVICES PACKAGE
===========================================================================
AUTHORS: Justin Ovens <jovens@gotunix.net>
Copyright (C) 1998 - 2009 GOTUNIX Networks.   All rights reserved.


This file is part of the IRCSP [Internet Relay Chat Services Package]
source code.

Redistribution and use in source and binary forms, with or without
modification are permitted provided that the following conditions are
met:

        Redistribution of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

        Redistribution in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in
        the documentation and/or other materials provided with the
        distribution.

        Neither the name of GOTUNIX Networks nor the names of its
        contributors may be used to endorce or promote products derived
        from this software without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
===========================================================================
*/
//------------------------------------------------------------------------
// $File: $
// $Revision: 1.26 $
// $Author: jovens $
// $Date: 2009/04/11 07:22:39 $
//------------------------------------------------------------------------






#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>


#include "debug.h"
#include "socket.h"
#include "ircsp.h"
#include "conf.h"
#include "users.h"
#include "write.h"
#include "misc.h"
#include "macros.h"
#include "flags.h"
#include "uworld.h"
#include "log.h"


//
// ircsp_warning ()
//
//  Send out warning notices... 
//

void ircsp_warning (char *format, ...)
{
	User *myuserp;

	char buf[256];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);

	myuserp = findUser (config->uworld_nickname);
	if (myuserp)
		ssprintf(s, "%s O %s :\002WARNING\002 - %s\n",
			myuserp->numeric, config->operchannel_name, buf);
}
	
//
// ircsp_notice ()
//
//  Send out notices
//

void ircsp_notice (char *format, ...)
{
	User *myuserp;

	char buf[256];
	va_list msg;

	va_start(msg, format);
	vsprintf(buf, format, msg);


	myuserp = findUser (config->uworld_nickname);

	if (config->operchannel_notice == 1)
	{
		if (myuserp)
		{
			ssprintf(s, 
				"%s O %s :\002NOTICE\002 - %s\n",
				myuserp->numeric, config->operchannel_name, buf);
		}
	}

	else if (config->operchannel_notice == 2)
	{
		if (myuserp)
		{
			ssprintf (s,
				"%s WA :\002NOTICE\002 - %s\n",
				config->server_numeric, buf);
		}
	}
}


//
// checkGline ()
//
//  Check for expired glines and remove them.

void checkGlines (void)
{
	Gline *glinep;
	time_t curtime;

	LOG (MAINLOG, "[%s:%d:%s()]:  Checking for expired glines\n",
		__FILE__, __LINE__, __FUNCTION__);

	curtime = time(NULL);

	for (glinep = glineHEAD; glinep; glinep = glinep->next)
	{
		if (glinep->expires < curtime)
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Removing expired GLINE for [%s]\n",
				__FILE__, __LINE__, __FUNCTION__, glinep->mask);
			ssprintf(s, "%s WA :Removing Expired GLINE for [%s]\n",
				config->server_numeric, glinep->mask);


			ssprintf(s, "%s GL * -%s %lu %lu\n",
				config->server_numeric, glinep->mask, (time(NULL) - 60), 
				time(NULL));

			delGline(glinep->mask);
		}
	}

	uw_saveGlineDB ();

	LOG (MAINLOG, "[%s:%d:%s()]:  Expired GLINEs removed\n",
		__FILE__, __LINE__, __FUNCTION__);
}


