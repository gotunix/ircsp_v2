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
// $Revision: 1.10 $
// $Author: jovens $
// $Date: 2009/04/11 06:40:36 $
//------------------------------------------------------------------------






#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "conf.h"
#include "numeric.h"
#include "string.h"
#include "channels.h"
#include "users.h"
#include "servers.h"
#include "ircsp.h"
#include "debug.h"
#include "memory.h"
#include "flags.h"
#include "macros.h"
#include "users.h"
#include "log.h"



//------------------------------------------------------------------------
// addChannel ()
//
//------------------------------------------------------------------------


Channel *addChannel (char *name, int flags, time_t created, char *key, int limit)
{
	Channel *channelp;

	LOG (MAINLOG, "[%s:%d:%s()]:  Running for [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, name);

	channelp = (Channel *)ircsp_calloc(1, sizeof(Channel));
	if (!channelp)
	{
		ERROR (150, "Failed to allocate memory for channelHEAD\n", TRUE);
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for channelHEAD at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, channelp);


	channelp->name = (char *)ircsp_calloc(1, strlen(name) + 5);
	if (!channelp->name)
	{
		ERROR (150, "Failed to allocate memory for NAME\n", TRUE);
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for NAME at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, channelp->name);


	channelp->key = (char *)ircsp_calloc(1, PASSLEN + 5);
	if (!channelp->key)
	{
		ERROR (150, "Failed to allocate memory for KEY\n", TRUE);
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for KEY at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, channelp->key);

	strncpy(channelp->name, name, strlen(name) + 5);

	channelp->flags = flags;
	channelp->created = created;
	channelp->banhead = NULL;
	channelp->nickhead = NULL;

	if (flags & CHANNEL_HAS_LIMIT)
	{
		channelp->limit = limit;
	}
	else
	{
		channelp->limit = 0;
	}

	if (flags & CHANNEL_HAS_KEY)
	{
		strncpy(channelp->key, key, strlen(key) + 5);
	}

	
	LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] added\n",
		__FILE__, __LINE__, __FUNCTION__, channelp->name);


	if (!channelHEAD)
	{
		channelHEAD = channelp;
		channelp->next = NULL;
	}
	else
	{
		channelp->next = channelHEAD;
		channelHEAD = channelp;
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Finished running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, name);

	status->numChannels++;
	return channelp;
}


//------------------------------------------------------------------------
// findChannel ()
//
//------------------------------------------------------------------------


Channel *findChannel (char *name)
{
	Channel *channelp;
	
	LOG (MAINLOG, "[%s:%d:%s()]:  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, name);

	channelp = channelHEAD;
	while (channelp)
	{
		if (!strcasecmp(channelp->name, name))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, channelp->name,
				channelp->name);
				
			return channelp;
		}
		channelp = channelp->next;
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] not found\n",
		__FILE__, __LINE__, __FUNCTION__, name);

	return NULL;
}


//------------------------------------------------------------------------
// delChannel ()
//
//------------------------------------------------------------------------


void delChannel (char *name)
{
	Channel *channelp, *next, *delete, *check;

	LOG (MAINLOG, "[%s:%d:%s()]:  Deleting channel [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, name);

	for (channelp = channelHEAD; channelp; channelp = channelp->next)
	{
		next = channelp->next;
		if (!strcasecmp(channelp->name, name))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Deleting channel [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, name, channelp->name);

			if (channelHEAD == channelp)
			{
				channelHEAD = channelp->next;
			}
			else
			{
				for (delete = channelHEAD; (delete->next != channelp) && delete; delete = delete->next);
				delete->next = channelp->next;
			}

			status->numChannels--;

			ircsp_free(channelp->name);
			ircsp_free(channelp->key);

			channelp->flags = 0;
			channelp->limit = 0;
			channelp->created = 0;

			channelp->numBans = 0;
			channelp->numUsers = 0;

			ircsp_free (channelp);

			check = findChannel(name);
			if (!check)
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] deleted successfully\n",
					__FILE__, __LINE__, __FUNCTION__, name);
			}
			else
			{
				ERROR (999, "Failed to delete channel\n", TRUE);
			}


			return ;
		}
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Channel [%s] not found\n",
		__FILE__, __LINE__, __FUNCTION__, name);
}

//////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------
// addChannelNick ()
//
//------------------------------------------------------------------------


NickList *addChannelNick (struct _channel *channelp, struct _user *userp)
{
	NickList *nicklistp;

	nicklistp = (NickList *)ircsp_calloc(1, sizeof(NickList));
	if (!nicklistp)
	{
		// Debugging ?
		return NULL;
	}

	nicklistp->userp = userp;
	nicklistp->flags = 0;

	if (!channelp->nickhead)
	{
		channelp->nickhead = nicklistp;
		nicklistp->next = NULL;
	}
	else
	{
		nicklistp->next = channelp->nickhead;
		channelp->nickhead = nicklistp;
	}

	return nicklistp;
}


//------------------------------------------------------------------------
// findChannelNick ()
//
//------------------------------------------------------------------------


NickList *findChannelNick (struct _channel *channelp, struct _user *userp)
{
	NickList *nicklistp;
	nicklistp = channelp->nickhead;
	while (nicklistp)
	{
		if (nicklistp->userp == userp)
			return nicklistp;
		nicklistp = nicklistp->next;
	}
	return NULL;
}


//------------------------------------------------------------------------
// delChannelNick ()
//
//------------------------------------------------------------------------


void delChannelNick (struct _channel *channelp, struct _user *userp)
{
	NickList *prev;
	NickList *nicklistp;

	prev = NULL;
	nicklistp = channelp->nickhead;

	while (nicklistp)
	{
		if (nicklistp->userp == userp)
		{
			if (!prev)
			{
				channelp->nickhead = nicklistp->next;
			}
			else
			{
				prev->next = nicklistp->next;
			}

			ircsp_free (nicklistp);
		}
		prev = nicklistp;
		nicklistp = nicklistp->next;
	}
}


//////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------
// addChannelBan ()
//
//------------------------------------------------------------------------


Ban *addChannelBan (Channel *channelp, char *mask)
{
	Ban *banp;
	banp = (Ban *)ircsp_calloc(1, sizeof(Ban));
	if (!banp)
	{
		ERROR (150, "Failed to allocate memory for banHEAD\n", TRUE);
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for banHEAD at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, banp);

	banp->mask = (char *)ircsp_calloc(1, NICKLEN + USERLEN + HOSTLEN + 10);
	if (!banp->mask)
	{
		ERROR (150, "Failed to allocate memory for MASK\n", TRUE);
	}
	
	LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for MASK at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, banp->mask);

	strncpy (banp->mask, mask, NICKLEN +HOSTLEN + USERLEN + 3);

	LOG (MAINLOG, "[%s:%d:%s()]:  Ban [%s] added at [%p] to channel [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, banp->mask, banp->mask,
		channelp->name);

	if (!channelp->banhead)
	{
		channelp->banhead = banp;
		banp->next = NULL;
	}
	else
	{
		banp->next = channelp->banhead;
		channelp->banhead = banp;
	}

	return banp;
}


//------------------------------------------------------------------------
// findChannelBan ()
//
//------------------------------------------------------------------------



Ban *findChannelBan (Channel *channelp, char *mask)
{
	Ban *banp;
	
	LOG (MAINLOG, "[%s:%d:%s()]:  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, mask);
		
		
	banp = channelp->banhead;
	while (banp)
	{
		if (!strcasecmp(banp->mask, mask))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Ban [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, banp->mask,
				banp->mask);
				
			return banp;
		}
		banp = banp->next;
	}
	
	LOG (MAINLOG, "[%s:%d:%s()]:  Ban not found\n",
		__FILE__, __LINE__, __FUNCTION__);
		
	return NULL;
}


//------------------------------------------------------------------------
// delChannelBan ()
//
//------------------------------------------------------------------------


void delChannelBan (Channel *channelp, char *mask)
{
	Ban *banp, *next, *delete, *check;
	
	LOG (MAINLOG, "[%s:%d:%s()]:  Running on [%s / %s]\n",
		__FILE__, __LINE__, __FUNCTION__, channelp->name, mask);


	for (banp = channelp->banhead; banp; banp = next)
	{
		next = banp->next;
		if (!strcasecmp(banp->mask, mask))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Deleting ban [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, banp->mask,
				banp->mask);
				
			if (channelp->banhead == banp)
			{
				channelp->banhead = banp->next;
			}
			else
			{
				for (delete = channelp->banhead; (delete->next != banp) && delete; delete = delete->next);
				delete->next = banp->next;
			}


			check = findChannelBan (channelp, mask);
			if (!check)
			{
				LOG (MAINLOG, "[%s:%d:%s()]  Channel Ban deleted successfully\n",
					__FILE__, __LINE__, __FUNCTION__);
			}
			else
			{
				ERROR (999, "Failed to delete channel ban\n", TRUE);
			}
			
			ircsp_free (banp->mask);
			ircsp_free (banp);

			return;
		}
	}
}

