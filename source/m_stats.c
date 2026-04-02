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
// $Revision: 1.29 $
// $Author: jovens $
// $Date: 2009/04/11 07:03:53 $
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


//------------------------------------------------------------------------
//  p_stats ()
//------------------------------------------------------------------------

//@(#) - (s_parse.c:101) s_parse(): Read: ACAAA R u :A


void m_stats (char *sBuf)
{
	LOG (MAINLOG, "[%s:%d:%s()]:  Starting STATS parser\n",
		__FILE__, __LINE__, __FUNCTION__);

	if (IsProtocolUndernet (config))
	{
		User *userp;
		Server *myserverp;

		char *argv[4];

		argv[0] = strtok(sBuf, " ");  // Nickname
		strtok(NULL, " "); // Command
		argv[1] = strtok(NULL, " "); // Stats Paramater


		myserverp = findServerByNumeric(config->server_numeric);

		userp = findUserByNumeric (argv[0]);
		if (!userp)
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Command received from non-existant user [Numeric: %s]\n",
				__FILE__, __LINE__, __FUNCTION__, argv[0]);
			return ;
		}

		if (!strcasecmp(argv[1], "U"))
		{
			int days, hours, mins, secs;
			time_t curtime, uptime;

			LOG (MAINLOG, "[%s:%d:%s()]:  UPTIME\n",
				__FILE__, __LINE__, __FUNCTION__);
	
			curtime = time(NULL);
			uptime = curtime - service->uptime;

			days = uptime/86400;
			hours = (uptime/3600)%24;
			mins = (uptime/60)%60;
			secs = uptime%60;

			ssprintf (s, 
				"%s O %s :\002%s\002 has been up for \002%d days, %d hours %d mins %d seconds\002\n", 
				myserverp->numeric, argv[0], config->server, days, hours, 
				mins, secs);
		}

		else if (!strcasecmp(argv[1], "M"))
		{
			ssprintf (s, 
				"%s O %s :MEMORY USAGE\n",
				myserverp->numeric, argv[0]);

			ssprintf (s,
				"%s O %s :Config - [%u]Bytes\n",
				myserverp->numeric, argv[0],
				sizeof(Config));

			ssprintf (s,
				"%s O %s :Nicknames [%d] - [%u]Bytes\n",
				myserverp->numeric, argv[0],
				status->numUsers, status->numUsers*sizeof(User));

			ssprintf (s,
				"%s O %s :Hostnames [%d] - [%u]Bytes\n",
				myserverp->numeric, argv[0],
				status->numHosts, status->numHosts*sizeof(Hostname));

			ssprintf (s,
				"%s O %s :Channels [%d] - [%u]Bytes\n",
				myserverp->numeric, argv[0],
				status->numChannels, status->numChannels*sizeof(Channel));

			ssprintf(s, 
				"%s O %s :Admins [%d] - [%u]Bytes\n",
				myserverp->numeric, argv[0],
				status->numAdmins, status->numAdmins*sizeof(Admin));

			ssprintf (s,
				"%s O %s :GLines [%d] - [%u]Bytes\n",
				myserverp->numeric, argv[0],
				status->numGlines, status->numGlines*sizeof(Gline));

			ssprintf (s,
				"%s O %s :Servers [%d] - [%u]Bytes\n",
				myserverp->numeric, argv[0],
				status->numServers, status->numServers*sizeof(Server));
		}
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Finished STATS parser\n",
		__FILE__, __LINE__, __FUNCTION__);
}
