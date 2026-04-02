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







//------------------------------------------------------------------------
// m_mode ()
//
//
//  This function has many contingencies to deal with
//  
//  Usermode:
//    [NUM] M <nickname> :<modes>
//
//  Channel Mode:
//    [NUM] M <#channel> <modes> <ts>
//
//  Channel User Mode:
//    [NUM] M <#channel> <modes> [NUM] [NUM] [NUM]
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


// @(#) - (s_parse.c:119) s_parse(): Socket(READ): ACAAF M f00ball :-i

void m_mode (char *sBuf)
{
	LOG (MAINLOG, "[%s:%d:%s()]:  Starting MODE parser\n",
		__FILE__, __LINE__, __FUNCTION__);

	if (IsProtocolUndernet (config))
	{
		User *myuserp;
		User *userp;
		Server *serverp;

		char *argv[9], *ban, *key;
		int limit, flag;
		int p = -1, addModes = 0;

		argv[0] = strtok(sBuf, " "); // Numeric
		strtok(NULL, " "); // Command (SKIP!)
		argv[1] = strtok(NULL, " "); // Nickname OR Channel
		argv[2] = strtok(NULL, " "); // Modes


		if (argv[1][0] != '#')
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  USER MODE\n",
				__FILE__, __LINE__, __FUNCTION__);

			userp = findUser(argv[1]);
			if (!userp)
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  Failed to find USER nickname\n",
					__FILE__, __LINE__, __FUNCTION__);

				return ;
			}

			serverp = findServerByNumeric(userp->server);
			if (!serverp)
			{
				LOG (MAINLOG, "[%s:%d:%s():  Failed to find server\n",
					__FILE__, __LINE__, __FUNCTION__);

				return ;
			}

			while (argv[2][++p] != '\0')
			{
				if (argv[2][p] == '+')
				{
					addModes = 1;
					continue;
				}
				if (argv[2][p] == '-')
				{
					addModes = 0;
					continue;
				}

				if (argv[2][p] == 'o')  // Operator!
				{
					if (addModes)
					{
						myuserp = findUser(config->uworld_nickname);
						SetOperator(userp);
	
						ircsp_notice("OPERATOR DETECTED - %s (%s@%s) on %s\n",
							userp->nickname, userp->username, 
							userp->hostp->hostname, serverp->name);

						ssprintf(s, "%s O %s :Greetings Operator!\n",
							myuserp->numeric, argv[0]);


						serverp->numOpers++;
					}
					else
					{
						DelOperator(userp);


						ircsp_notice("DE-OPER DETECTED - %s (%s@%s) on %s\n",
							userp->nickname, userp->username, 
							userp->hostp->hostname,	serverp->name);

						serverp->numOpers--;
					}
				}
			}
		}
		else
		{
			// argv[0] NUMERIC
			// argv[1] CHANNEL
			// argv[2] MODES

			Channel *channelp;
			User *userp;
			ChanList *chanlistp;
			NickList *nicklistp;

			char *numeric, *key;
			int limit, flag;

			LOG (MAINLOG, "[%s:%d:%s()]:  CHANNEL MODE\n",
				__FILE__, __LINE__, __FUNCTION__);

			channelp = findChannel(argv[1]);
			if (!channelp)
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] not found!\n",
					__FILE__, __LINE__, __FUNCTION__, argv[1]);
				return ;
			}

			while (argv[2][++p] != '\0')
			{
				if (argv[2][p] == '+')
				{
					addModes = 1;
					continue;
				}
				else if (argv[2][p] == '-')
				{
					addModes = 0;
					continue;
				}

				if (argv[2][p] == 'o')
				{
					numeric = strtok(NULL, " ");

					userp = findUserByNumeric(numeric);
					if (!userp)
						continue;


					nicklistp = findChannelNick (channelp, userp);
					chanlistp = findNickChannel (userp, channelp);
					if (!nicklistp || !chanlistp)
						continue;
	
					if (addModes)
					{
						nicklistp->flags |= IS_CHANOP;
						chanlistp->flags |= IS_CHANOP;
						continue;
					}
					else
					{
						nicklistp->flags &= ~IS_CHANOP;
						chanlistp->flags &= ~IS_CHANOP;
						continue;
					}
				}

				if (argv[2][p] == 'v')
				{
					numeric = strtok(NULL, " ");

					userp = findUserByNumeric(numeric);
					if (!userp)
						continue;


					nicklistp = findChannelNick (channelp, userp);
					chanlistp = findNickChannel (userp, channelp);
					if (!nicklistp || !chanlistp)
						continue;

					if (addModes)
					{
						nicklistp->flags |= IS_CHANVOICE;
						chanlistp->flags |= IS_CHANVOICE;
						continue;
					}
					else
					{
						nicklistp->flags &= ~IS_CHANVOICE;
						chanlistp->flags &= ~IS_CHANVOICE;
						continue;
					}
				}

				if (argv[2][p] == 'b')
				{
					ban = strtok(NULL, " ");
					if (addModes)
					{
						addChannelBan (channelp, ban);
						continue;
					}
					else
					{
						delChannelBan (channelp, ban);
						continue;
					}
				}

				if (argv[2][p] == 'k')
				{
					key = strtok(NULL, " ");
					if (addModes)
					{
						strncpy(channelp->key, key, strlen(key) + 5);
						channelp->flags |= CHANNEL_HAS_KEY;
						continue;
					}
					else
					{
						channelp->key = NULL;
						channelp->flags &= ~CHANNEL_HAS_KEY;
						continue;
					}
				}

				if (argv[2][p] == 'l')
				{
					limit = atoi(strtok(NULL, " "));
					if (addModes)
					{
						channelp->limit = limit;
						channelp->flags |= CHANNEL_HAS_LIMIT;
						continue;
					}
					else
					{
						channelp->limit = 0;
						channelp->flags &= ~CHANNEL_HAS_LIMIT;
						continue;
					}
				}
			}
		}

	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Finished MODE parser\n",
		__FILE__, __LINE__, __FUNCTION__);
}
