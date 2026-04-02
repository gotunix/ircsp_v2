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
// $Revision: 1.38 $
// $Author: jovens $
// $Date: 2009/04/11 06:40:36 $
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


//@(#) - (s_parse.c:98) s_parse(): Read: ACAAA D ADAAE :irc.gotunix.net!evilicey (meep)

void m_kill (char *sBuf)
{
	User *myuserp;
	User *userp;
	Hostname *hostp;
	Server *serverp;
	Admin *adminp;
	Channel *channelp;
	ChanList *chanlistp;

	char *argv[4];

	LOG (MAINLOG, "[%s:%d:%s()]:  Starting KILL parser\n",
		__FILE__, __LINE__, __FUNCTION__);


	if (IsProtocolUndernet (config))
	{
		argv[0] = strtok(sBuf, " "); // Killer Nickname Numeric
		strtok(NULL, " ");
		argv[1] = strtok(NULL, " "); // Killed Nickname Numeric

		myuserp = findUser(config->uworld_nickname);
		if (!myuserp)
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Failed to find UWORLD nickname\n",
				__FILE__, __LINE__, __FUNCTION__);
			return ;
		}

		userp = findUserByNumeric(argv[1]);
		if (!userp)
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Received KILL for non-existant nick\n",
				__FILE__, __LINE__, __FUNCTION__);

			return ;
		}


		LOG (MAINLOG, "[%s:%d:%s()]:  Received KILL for nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, userp->nickname);


		// Check host list first.

		hostp = findHostname (userp->hostp->hostname);
		if (!hostp)
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Hostname [%s] not found\n",
				__FILE__, __LINE__, __FUNCTION__, userp->hostp->hostname);
		}
		else
		{
			if (hostp->numHosts > 1)
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  Hostname [%s] found at [%p], lowering host count\n",
					__FILE__, __LINE__, __FUNCTION__, hostp->hostname, hostp->hostname);

				hostp->numHosts--;
			}
			else
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  Deleting hostname [%s] found at [%p]\n",
					__FILE__, __LINE__, __FUNCTION__, hostp->hostname, 
					hostp->hostname);

				delHostname (hostp->hostname);
				status->numHosts--;
			}
		}

		serverp = findServerByNumeric(userp->server);
		if (!serverp)
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  SERVER not found\n",
				__FILE__, __LINE__, __FUNCTION__);
			return ;
		}
	
		if (IsAdministrator(userp))
		{
			adminp = findAdmin (userp->adminp->nickname);
			if (IsAdminLogin (adminp))
			{
				DelAdminLogin (adminp);
			}
			DelAdministrator (userp);

			userp->adminp = NULL;
			adminp->userp = NULL;
		}
	

		serverp->numUsers--;
		status->numUsers--;


		if (IsOperator(userp))
		{
			serverp->numOpers--;
			status->numOpers--;
		}

		// Remove from channels.

		chanlistp = userp->chanhead;
		while (chanlistp)
		{
			channelp = chanlistp->channelp;
			delChannelNick (channelp, userp);
			delNickChannel (userp, channelp);
			chanlistp = chanlistp->next;
		}

		delUser (userp->nickname);

	}
	
	LOG (MAINLOG, "[%s:%d:%s()]:  Finishing KILL parser\n",
		__FILE__, __LINE__, __FUNCTION__);
}
