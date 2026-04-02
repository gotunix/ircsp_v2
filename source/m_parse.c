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
// $Revision: 1.51 $
// $Author: jovens $
// $Date: 2009/04/11 07:15:01 $
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
#include "version.h"
#include "log.h"
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


void m_parse (char *sBuf)
{
	LOG (MAINLOG, "[%s:%d:%s()]:  Startng PARSER\n",
		__FILE__, __LINE__, __FUNCTION__);


	LOG (MAINLOG, "[%s:%d:%s()]:  Socket(READ): %s",
		__FILE__, __LINE__, __FUNCTION__, sBuf);

	if (IsProtocolUndernet (config))
	{
		if ((!strcmp(token(sBuf, 2), "EOB_ACK")) || (!strcmp(token(sBuf, 2), "EA")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  End of BURST (EA)\n",
				__FILE__, __LINE__, __FUNCTION__);

			if (!service->initial_burst)
			{
				service->initial_burst = 1;

				LOG (MAINLOG, "[%s:%d:%s()]:  SPAWNING services\n",
					__FILE__, __LINE__, __FUNCTION__);
	
				if (config->uworld_enable)
				{
					ircsp_spawnUW ();
				}

				if (config->cserve_enable)
				{
					ircsp_spawnCS ();
				}

				if (config->nserve_enable)
				{
					ircsp_spawnNS ();
				}

				synch ();
			}
		}


		else if ((!strcmp(token(sBuf, 1), "SERVER")) || (!strcmp(token(sBuf, 1), "S")) ||
				   (!strcmp(token(sBuf, 2), "SERVER")) || (!strcmp(token(sBuf, 2), "S")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  SERVER [S]\n",
				__FILE__, __LINE__, __FUNCTION__);

			m_server (sBuf);
		}

		else if ((!strcmp(token(sBuf, 2), "END_OF_BURST")) || (!strcmp(token(sBuf, 2), "EB")))
		{
			Server *serverp;

			LOG (MAINLOG, "[%s:%d:%s()]:  End of BURST [EB]\n",
				__FILE__, __LINE__, __FUNCTION__);

			serverp = findServerByNumeric (token(sBuf, 1));
			if (!serverp)
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  Server not found\n",
					__FILE__, __LINE__, __FUNCTION__);

				return ;
			}

			serverp->flags &= ~SERVER_IN_BURST;
			if (serverp->flags & SERVER_IS_MY_UPLINK)
			{
				Server *myserverp;
				myserverp = findServer(config->server);
				ssprintf (s, 
					"%s EA\r\n", myserverp->numeric);

				ssprintf (s, 
					"%s EB\r\n", myserverp->numeric);
			}
		}

		else if ((!strcmp(token(sBuf, 2), "PING")) || (!strcmp(token(sBuf, 2), "G")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  PING [G]\n",
				__FILE__, __LINE__, __FUNCTION__);

			m_ping (sBuf);
		}

		else if ((!strcmp(token(sBuf, 2), "NICK")) || (!strcmp(token(sBuf, 2), "N")) ||
				   (!strcmp(token(sBuf, 1), "NICK")) || (!strcmp(token(sBuf, 1), "N")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  NICK [N]\n",
				__FILE__, __LINE__, __FUNCTION__);

			m_nick (sBuf);
		}

		else if ((!strcmp(token(sBuf, 2), "QUIT")) || (!strcmp(token(sBuf, 2), "Q")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  QUIT [Q]\n",
				__FILE__, __LINE__, __FUNCTION__);

			m_quit (sBuf);
		}

		else if ((!strcmp(token(sBuf, 2), "KILL")) || (!strcmp(token(sBuf, 2), "D")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  KILL [D]\n",
				__FILE__, __LINE__, __FUNCTION__);

			m_kill (sBuf);
		}

		else if ((!strcmp(token(sBuf, 2), "PRIVMSG")) || (!strcmp(token(sBuf, 2), "P")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  PRIVMSG [P]\n",
				__FILE__, __LINE__, __FUNCTION__);

			m_privmsg (sBuf);
		}

		else if ((!strcmp(token(sBuf, 2), "VERSION")) || (!strcmp(token(sBuf, 2), "V")))
		{
			Server *myserverp;

			LOG (MAINLOG, "[%s:%d:%s()]  VERSION [V]\n",
				__FILE__, __LINE__, __FUNCTION__);

			myserverp = findServerByNumeric(config->server_numeric);
			ssprintf(s, "%s O %s :IRCSP [%s] Copyright (C) 1998-2009 GOTUNIX Networks.  All rights reserved.\n",
				myserverp->numeric, token(sBuf, 1), version_number);
		}

		else if ((!strcmp(token(sBuf, 2), "INFO")) || (!strcmp(token(sBuf, 2), "F")))
		{
			Server *myserverp;

			LOG (MAINLOG, "[%s:%d:%s()]:  INFO [F]\n",
				__FILE__, __LINE__, __FUNCTION__);


			myserverp = findServerByNumeric(config->server_numeric);

			ssprintf (s, 
				"%s O %s :Internet Relay Chat Services Package [IRCSP] - Version: %s\n",
				myserverp->numeric, token(sBuf, 1), version_number);
			ssprintf (s, 
				"%s O %s :Programming by: \002evilicey\002 (Justin Ovens)\n",
				myserverp->numeric, token(sBuf, 1));
			ssprintf (s, 
				"%s O %s :Original Programming by: \002moof\002 (Anthony Soriano)\n",
				myserverp->numeric, token(sBuf, 1));
			ssprintf (s, 
				"%s O %s :End of /INFO\n",
				myserverp->numeric, token(sBuf, 1));
		}

		else if ((!strcmp(token(sBuf, 2), "STATS")) || (!strcmp(token(sBuf, 2), "R")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  STATS [R]\n",
				__FILE__, __LINE__, __FUNCTION__);

			m_stats (sBuf);
		}

		else if ((!strcmp(token(sBuf, 2), "SQUIT")) || (!strcmp(token(sBuf, 2), "SQ")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  SQUIT [SQ]\n",
				__FILE__, __LINE__, __FUNCTION__);

			m_squit (sBuf);
		}

		else if ((!strcmp(token(sBuf, 2), "MODE")) || (!strcmp(token(sBuf, 2), "M")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  MODE [M]\n",
				__FILE__, __LINE__, __FUNCTION__);

			m_mode (sBuf);
		}

		else if ((!strcmp(token(sBuf, 2), "JOIN")) || (!strcmp(token(sBuf, 2), "J")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  JOIN [J]\n",
				__FILE__, __LINE__, __FUNCTION__);

			m_join (sBuf);
		}

		else if ((!strcmp(token(sBuf, 2), "BURST")) || (!strcmp(token(sBuf, 2), "B")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  BURST [B]\n",
				__FILE__, __LINE__, __FUNCTION__);
	
			m_burst (sBuf);
		}

		else if ((!strcmp(token(sBuf, 2), "PART")) || (!strcmp(token(sBuf, 2), "L")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  PART [L]\n",
				__FILE__, __LINE__, __FUNCTION__);

			m_part (sBuf);
		}

		else if ((!strcmp(token(sBuf, 2), "DESTRUCT")) || (!strcmp(token(sBuf, 2), "DE")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  DESTRUCT [DE]\n",
				__FILE__, __LINE__, __FUNCTION__);

			m_destruct (sBuf);
		}

		else if ((!strcmp(token(sBuf, 2), "CREATE")) || (!strcmp(token(sBuf, 2), "C")))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  CREATE [C]\n",
				__FILE__, __LINE__, __FUNCTION__);

			m_create (sBuf);
		}

		LOG (MAINLOG, "[%s:%d:%s()]:  End of parser\n",
			__FILE__, __LINE__, __FUNCTION__);
	}
}

