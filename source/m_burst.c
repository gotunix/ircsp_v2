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
// $Revision: 1.36 $
// $Author: jovens $
// $Date: 2009/04/11 07:42:26 $
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
// m_burst ()
//
//
//  Parse incoming channels (BURSTs).  This gets semi-complicated
//  because in large channels there may need to be multiple
//  BURST commands (since there is a limit of 512 bytes per line).
//
//  General format for a burst:
//
//   [N] BURST <#channel> <ts> [+modes] [lim] [key] <users> [:%ban...]
//
//  If it is a continuation burst, the first parameters are the same,
//  but will NOT contains modes and may contain only bans, only
//  users, or both.
//------------------------------------------------------------------------



//BUFFER AC B #god 1237427486 ACAAR:o,ACAAV

void m_burst (char *sBuf)
{
	LOG (MAINLOG, "[%s:%d:%s()]:  Starting BURST parser\n",
		__FILE__, __LINE__, __FUNCTION__);

	if (IsProtocolUndernet (config))
	{
		Channel *channelp;
		User *userp;

		char *server, *channel, *modes, *key = NULL, *userlist, *banlist = NULL;
		char *ban, user[4], uflags[3], burst[7];
		int limit = 0, count = 0, flags = 0;
		int i;
		time_t created;

		server = strtok(sBuf, " ");	// Server Numeric
		strtok(NULL, " "); // Burst command
		channel = strtok(NULL, " "); // Channel

		channelp = findChannel (channel);
		if (!channelp)
		{
			int last_mode = 0;
			int last_flag = 0;

			LOG (MAINLOG, "[%s:%d:%s()]:  New Channel [%s] - Adding\n",
				__FILE__, __LINE__, __FUNCTION__, channel);

			created = atol(strtok(NULL, " "));
			modes = strtok(NULL, " ");

			if (modes == NULL)
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  Empty Channel [%s]\n",
					__FILE__, __LINE__, __FUNCTION__, channel);

				channelp = addChannel(channel, flags, created, key, limit);
			}
			else
			{
				created = time(NULL);
				if (modes[0] == '+')
				{
					int pos = 0;

					LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] has MODES\n",
						__FILE__, __LINE__, __FUNCTION__, channel);

					while (modes[++pos] != '\0')
					{
						if (modes[pos] == 'l')
						{
							LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] has MODE +l\n",
								__FILE__, __LINE__, __FUNCTION__, channel);

							limit = atoi(strtok(NULL, " "));
							flags |= CHANNEL_HAS_LIMIT;
						}

						if (modes[pos] == 'k')
						{
							LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] has MODE +k\n",
								__FILE__, __LINE__, __FUNCTION__, channel);

							key = strtok(NULL, " ");
							flags |= CHANNEL_HAS_KEY;
						}

						if (modes[pos] == 'p')
						{
							LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] has MODE +p\n",
								__FILE__, __LINE__, __FUNCTION__, channel);
						}

						if (modes[pos] == 'm')
						{
							LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] has MODE +m\n",
								__FILE__, __LINE__, __FUNCTION__, channel);
						}

						if (modes[pos] == 's')
						{
							LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] has MODE +s\n",
								__FILE__, __LINE__, __FUNCTION__, channel);
						}

						if (modes[pos] == 't')
						{
							LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] has MODE +t\n",
								__FILE__, __LINE__, __FUNCTION__, channel);
						}
					}
					userlist = strtok(NULL, " ");
				}
				else
				{
					userlist = modes;
				}

				banlist = strtok(NULL, "\n");

				channelp = addChannel(channel, flags, created, key, limit);

				if (banlist)
				{
					LOG (MAINLOG, "[%s:%d:%s()]:  Proccessing BAN list [%s]\n",
						__FILE__, __LINE__, __FUNCTION__, banlist);

					banlist += 2;
					ban = strtok(banlist, " ");
					addChannelBan(channelp, ban);
					while (ban = strtok(NULL, " "))
						addChannelBan(channelp, ban);
				}

				count = tokenize(userlist, ',');

				LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] has [%d] users\n",
					__FILE__, __LINE__, __FUNCTION__, channel, count);

				for (i = 1; i < count+1; i++)
				{
					NickList *nicklistp;
					ChanList *chanlistp;

					strncpy(burst, token(userlist, i), 8);
					if (tokenize(burst, ':') == 1)
					{
						if (!last_mode)
						{
							LOG (MAINLOG, "[%s:%d:%s()]:  Nickname [%s] on channel [%s] has no MODES\n",
								__FILE__, __LINE__, __FUNCTION__, token(burst, 1), channel);

							userp = findUserByNumeric(token(burst, 1));
							nicklistp = addChannelNick(channelp, userp);
							chanlistp = addNickChannel(userp, channelp);
						}
						else
						{
							LOG (MAINLOG, "[%s:%d:%s()]:  Nickname [%s] on channel [%s] has LAST_MODE\n",
								__FILE__, __LINE__, __FUNCTION__, token(burst, 1), channel);

							userp = findUserByNumeric(token(burst, 1));
							nicklistp = addChannelNick(channelp, userp);
							chanlistp = addNickChannel(userp, channelp);
						
							if (last_flag & IS_CHANOP)
							{
								LOG (MAINLOG, "[%s:%d:%s()]:  |- IS CHANOP\n",
									__FILE__, __LINE__, __FUNCTION__);
							}

							else if (last_flag & IS_CHANVOICE)
							{
								LOG (MAINLOG, "[%s:%d:%s()]:  |- IS CHANVOICE\n",
									__FILE__, __LINE__, __FUNCTION__);
							}

							chanlistp->flags = last_flag;
							nicklistp->flags = last_flag;
						}
					}
					else
					{
						LOG (MAINLOG, "[%s:%d:%s()]:  Nickname [%s] on channel [%s] has MODES\n",
							__FILE__, __LINE__, __FUNCTION__, token(burst, 1), channel);


						strncpy(uflags, token(burst, 2), 2);
						userp = findUserByNumeric(token(burst, 1));
						nicklistp = addChannelNick(channelp, userp);
						chanlistp = addNickChannel(userp, channelp);

						if (uflags[0] == 'o' || uflags[1] == 'o')
						{
							LOG (MAINLOG, "[%s:%d:%s()]:  Nickname [%s] is a CHANOP on [%s]\n",
								__FILE__, __LINE__, __FUNCTION__, token(burst, 1), channel);
	
							last_flag = 0;
							nicklistp->flags = 0;
							chanlistp->flags = 0;

							nicklistp->flags |= IS_CHANOP;
							chanlistp->flags |= IS_CHANOP;
							last_flag |= IS_CHANOP;

							last_mode = 1;
						}

						else if (uflags[0] == 'v')
						{
							LOG (MAINLOG, "[%s:%d:%s()]:  Nickname [%s] is a CHANVOICE on [%s]\n",
								__FILE__, __LINE__, __FUNCTION__, token(burst, 1), channel);

							nicklistp->flags |= IS_CHANVOICE;
							chanlistp->flags |= IS_CHANVOICE;
							last_flag |= IS_CHANVOICE;
	
							last_mode = 1;
						}
					}
				}
			}
		}
		else
		{
			int last_mode = 0;
			int last_flag = 0;

			LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] exists updating\n",
				__FILE__, __LINE__, __FUNCTION__, channel);

			channelp->created = time(NULL);
			userlist = strtok(NULL, " ");
			banlist = strtok(NULL, " ");

			if (userlist[0] != ':')
			{
				count = tokenize(userlist, ',');
				for (i = 1; i < count+1; i++)
				{
					NickList *nicklistp;
					ChanList *chanlistp;

					strncpy(burst, token(userlist, i), 8);
					if (tokenize(burst, ':') == 1)
					{
						if (!last_mode)
						{
							LOG (MAINLOG, "[%s:%d:%s()]:  Nickname [%s] on channel [%s] has no MODES\n",
								__FILE__, __LINE__, __FUNCTION__, token(burst, 1), channel);

							userp = findUserByNumeric(token(burst, 1));
							nicklistp = addChannelNick(channelp, userp);
							chanlistp = addNickChannel(userp, channelp);
						}
						else
						{
							LOG (MAINLOG, "[%s:%d:%s()]:  Nickname [%s] on channel [%s] has LAST_MODE\n",
								__FILE__, __LINE__, __FUNCTION__, token(burst, 1), channel);

							userp = findUserByNumeric(token(burst, 1));
							nicklistp = addChannelNick(channelp, userp);
							chanlistp = addNickChannel(userp, channelp);
							if (last_flag & IS_CHANOP)
							{
								LOG (MAINLOG, "[%s:%d:%s()]:  |-  IS CHANOP\n",
									__FILE__, __LINE__, __FUNCTION__);
							}
							else if (last_flag & IS_CHANVOICE)
							{
								LOG (MAINLOG, "[%s:%d:%s()]:  |-  IS CHANVOICE\n",
									__FILE__, __LINE__, __FUNCTION__);
							}

							nicklistp->flags = last_flag;
							chanlistp->flags = last_flag;
						}
					}
					else
					{
						LOG (MAINLOG, "[%s:%d:%s()]:  Nickname [%s] on channel [%s] has MODES\n",
							__FILE__, __LINE__, __FUNCTION__, token(burst, 1), channel);

						userp = findUserByNumeric(token(burst, 1));
						nicklistp = addChannelNick(channelp, userp);
						chanlistp = addNickChannel(userp, channelp);
						strncpy(uflags, token(burst, 2), 2);
						if (uflags[0] == 'o' || uflags[1] == 'o')
						{
							LOG (MAINLOG, "[%s:%d:%s()]:  Nickname [%s] is a CHANOP on [%s]\n",
								__FILE__, __LINE__, __FUNCTION__, token(burst, 1), channel);

							last_flag = 0;
							nicklistp->flags = 0;
							chanlistp->flags = 0;

							nicklistp->flags |= IS_CHANOP;
							chanlistp->flags |= IS_CHANOP;
							last_flag |= IS_CHANOP;
							last_mode = 1;
						}
						else if (uflags[0] == 'v')
						{
							LOG (MAINLOG, "[%s:%d:%s()]:  Nickname [%s] is a CHANVOICE on [%s]\n",
								__FILE__, __LINE__, __FUNCTION__, token(burst, 1), channel);

							last_flag = 0;
							nicklistp->flags = 0;
							chanlistp->flags = 0;

							nicklistp->flags |= IS_CHANVOICE;
							chanlistp->flags |= IS_CHANVOICE;
							last_flag |= IS_CHANVOICE;
							last_mode = 1;
						}
					}
				}
			}

			if (banlist)
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  Proccessing BAN list\n",
					__FILE__, __LINE__, __FUNCTION__);

				banlist += 2;
				ban = strtok(banlist, " ");
				addChannelBan(channelp, ban);
				while (ban = strtok(NULL, " "))
					addChannelBan(channelp, ban);
			}
		}
	}
	else
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  This function is not used on anything but UNDERNET\n",
			__FILE__, __LINE__, __FUNCTION__);
	}
}
