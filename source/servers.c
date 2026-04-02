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
// $Revision: 1.31 $
// $Author: jovens $
// $Date: 2009/04/11 07:28:31 $
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


Server *addServer (char *name, char *numeric, time_t created)
{
	Server *serverp;

	LOG (MAINLOG, "[%s:%d:%s()]:  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, name);

	serverp = (Server *)ircsp_calloc(1, sizeof(Server));
	if (!serverp)
	{
		ERROR (150, "Failed to allocate memory for serverHEAD\n", TRUE);
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for serverHEAD at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, serverp);

	serverp->name = (char *)ircsp_calloc(1, strlen(name) +5);
	if (!serverp->name)
	{
		ERROR (150, "Failed to allocate memory for NAME\n", TRUE);
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for NAME at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, serverp->name);

	serverp->numeric = (char *)ircsp_calloc(1, strlen(numeric) + 5);
	if (!serverp->numeric)
	{
		ERROR (150, "Failed to allocate memory for NUMERIC\n", TRUE);
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for NUMERIC at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, serverp->numeric);

	serverp->uplink = (char *)ircsp_calloc(1, strlen(numeric) + 5);
	if (!serverp->uplink)
	{
		ERROR(150, "Failed to allocate memory for UPLINK\n", TRUE);
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for UPLINK at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, serverp->uplink);

	strncpy(serverp->name, name, strlen(name) + 5); 
	strncpy(serverp->numeric, numeric, strlen(numeric) + 5); 
	serverp->created = created;
	serverp->numUsers = serverp->numOpers = 0;
	serverp->flags |= SERVER_IN_BURST;

	LOG (MAINLOG, "[%s:%d:%s()]:  Server [%s] added\n",
		__FILE__, __LINE__, __FUNCTION__, serverp->name);

	if (!serverHEAD)
	{
		serverHEAD = serverp;
		serverp->next = NULL;
	}
	else
	{
		serverp->next = serverHEAD;
		serverHEAD = serverp;
	}
	status->numServers++;

	LOG (MAINLOG, "[%s:%d:%s()]:  Finished running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, name);

	return serverp;
}


Server *findServer (char *name)
{
	Server *serverp;

	LOG (MAINLOG, "[%s:%d:%s()]:  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, name);

	serverp = serverHEAD;
	while (serverp)
	{
		if (!strcasecmp(name, serverp->name))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Server [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, serverp->name,
				serverp->name);

			return serverp;
		}
		serverp = serverp->next;
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Server [%s] not found\n",
		__FILE__, __LINE__, __FUNCTION__, name);

	return NULL;
}


Server *findServerByNumeric (char *numeric)
{
	Server *serverp;

	LOG (MAINLOG, "[%s:%d:%s()]:  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, numeric);

	serverp = serverHEAD;
	while (serverp)
	{
		if (!strcasecmp(serverp->numeric, numeric))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Server [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, serverp->name,
				serverp->name);

			return serverp;
		}
		serverp = serverp->next;
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Server [%s] not found\n",
		__FILE__, __LINE__, __FUNCTION__, numeric);

	return NULL;
}


void delServer (char *name)
{
	Server *serverp, *next, *delete, *check;


	LOG (MAINLOG, "[%s:%d:%s()]:  Deleting server [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, name);

	for (serverp = serverHEAD; serverp; serverp = next)
	{
		next = serverp->next;
		if (!strcasecmp(serverp->name, name))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Deleting server [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, serverp->name,
				serverp->name);

			if (serverHEAD == serverp)
			{
				serverHEAD = serverp->next;
			}
			else
			{
				for (delete = serverHEAD; (delete->next != serverp) && delete; delete = delete->next);
				delete->next = serverp->next;
			}

			serverp->flags = 0;
			serverp->numUsers = 0;
			serverp->numOpers = 0;

			ircsp_free (serverp->name);
			ircsp_free (serverp->numeric);
			ircsp_free (serverp->uplink);
			ircsp_free (serverp);

			check = findServer (name);
			if (!check)
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  Server deleted successfully\n",
					__FILE__, __LINE__, __FUNCTION__);
			}
			else
			{
				ERROR (200, "Failed to delete server\n", TRUE);
			}

			status->numServers--;

			return ;
		}
	}
}
