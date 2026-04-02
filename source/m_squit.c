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
// $Revision: 1.25 $
// $Author: jovens $
// $Date: 2009/04/11 07:04:46 $
//------------------------------------------------------------------------






#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <netinet/in.h>


#include "conf.h"
#include "flags.h"
#include "users.h"
#include "macros.h"
#include "memory.h"
#include "numeric.h"
#include "m_create.h"
#include "m_kill.h"
#include "m_nick.h"
#include "m_ping.h"
#include "m_quit.h"
#include "m_squit.h"
#include "rehash.h"
#include "m_parse.h"
#include "synch.h"
#include "write.h"
#include "debug.h"
#include "ircsp.h"
#include "channels.h"
#include "servers.h"
#include "match.h"
#include "misc.h"
#include "m_burst.h"
#include "m_join.h"
#include "m_mode.h"
#include "m_part.h"
#include "m_privmsg.h"
#include "m_server.h"
#include "m_stats.h"
#include "socket.h"
#include "strings.h"
#include "uworld.h"
#include "log.h"

//  Socket(READ): ACAAA SQ irc2.gotunix.net 123688   2105 :irc1

void m_squit (char *sBuf)
{
	LOG (MAINLOG, "[%s:%d:%s()}:  Starting SQUIT parser\n",
		__FILE__, __LINE__, __FUNCTION__);

	if (IsProtocolUndernet (config))
	{
		Server *myserverp; // IRCSP Server
		Server *serverp;  // Squited Server
		Server *check;

		char *argv[2];

		strtok(sBuf, " ");
		strtok(NULL, " ");
		argv[0] = strtok(NULL, " ");

		myserverp = findServerByNumeric(config->server_numeric);
	
		serverp = findServer(argv[0]);
		if (!serverp)
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Received SQUIT for non-existant server [%s]\n",
				__FILE__, __LINE__, __FUNCTION__, serverp->name);
			return ;
		}

		// Check and see if there were any servers linked to argv[0]

		LOG (MAINLOG, "[%s:%d:%s()]:  SQUIT on server [%s] numeric [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, serverp->name,
			serverp->numeric);

		for (check = serverHEAD; check; check = check->next)
		{
			if (!strcasecmp(check->uplink, serverp->numeric))
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  Server [%s] had Server [%s] linked to it\n",
					__FILE__, __LINE__, __FUNCTION__, serverp->name, check->name);

				delUserByServer(check->numeric);
				delServer(check->name);
			}
		}

		delUserByServer(serverp->numeric);
		delServer(serverp->name);
	}


	LOG (MAINLOG, "[%s:%d:%s()]:  Finished SQUIT parser\n",
		__FILE__, __LINE__, __FUNCTION__);
}
