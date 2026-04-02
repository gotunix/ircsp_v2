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
// $Date: 2009/04/11 07:22:39 $
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


const char convert2y[] = {
	'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
	'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
	'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
	'w','x','y','z','0','1','2','3','4','5','6','7','8','9','[',']'
};


// Converts a character to its (base64) numnick value. 
const unsigned int convert2n[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	52,53,54,55,56,57,58,59,60,61, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
	15,16,17,18,19,20,21,22,23,24,25,62, 0,63, 0, 0,
	0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
	41,42,43,44,45,46,47,48,49,50,51, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


unsigned int base64toint (const char* s)
{
	unsigned int i = convert2n[(unsigned char) *s++];
	while (*s)
	{
		i <<= NUMNICKLOG;
		i += convert2n[(unsigned char) *s++];
	}
	return i;
}


char* intobase64 (char *buf, unsigned int v, unsigned int count)
{
	buf[count] = '\0';
	while (count > 0) 
	{
		buf[--count] = convert2y[(v & NUMNICKMASK)];
		v >>= NUMNICKLOG;
	}
	return buf;
}


//////////////////////////////////////////////////////////////////////////
// Linked list for Nick Numerics
//////////////////////////////////////////////////////////////////////////


NickNumeric *addNickNumeric (char *numeric)
{
	NickNumeric *nnumericp;

	LOG (MAINLOG, "[%s:%d:%s()]:  Running for [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, numeric);

	nnumericp = (NickNumeric *)ircsp_calloc(1, sizeof(NickNumeric));
	if (!nnumericp)
	{
		ERROR (150, "Failed to allocate memory for numericHEAD\n", TRUE);
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for numericHEAD at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, nnumericp);

	nnumericp->numeric = (char *)ircsp_calloc(1, strlen(numeric) + 5);
	if (!nnumericp->numeric)
	{
		ERROR (150, "Failed to allocate memory for NUMERIC\n", TRUE);
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for NUMERIC at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, nnumericp->numeric);

	strncpy (nnumericp->numeric, numeric, strlen(numeric) + 5);

	status->numNumeric++;


	LOG (MAINLOG, "[%s:%d:%s()]:  Numeric [%s] added\n",
		__FILE__, __LINE__, __FUNCTION__, numeric);

	if (!nicknumericHEAD)
	{
		nicknumericHEAD = nnumericp;
		nnumericp->next = NULL;
	}
	else
	{
		nnumericp->next = nicknumericHEAD;
		nicknumericHEAD = nnumericp;
	}


	LOG (MAINLOG, "[%s:%d:%s()]:  Finished on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, numeric);

	return nnumericp;
}


NickNumeric *findNickNumeric (char *numeric)
{
	NickNumeric *nnumericp;

	LOG (MAINLOG, "[%s:%d:%s()]:  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, numeric);

	nnumericp = nicknumericHEAD;
	while (nnumericp)
	{
		if (!strcasecmp(nnumericp->numeric, numeric))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Numeric [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, nnumericp->numeric,
				nnumericp->numeric);

			return nnumericp;
		}
		nnumericp = nnumericp->next;
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Numeric [%s] not found\n",
		__FILE__, __LINE__, __FUNCTION__, numeric);

	return NULL;
}


void delNickNumeric (char *numeric)
{
	NickNumeric *nnumericp, *next, *delete, *check;

	LOG (MAINLOG, "[%s:%d:%s()]:  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, numeric);

	for (nnumericp = nicknumericHEAD; nnumericp; nnumericp = next)
	{
		next = nnumericp->next;
		if (!strcasecmp(nnumericp->numeric, numeric))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Deleting numeric [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, numeric, nnumericp->numeric);

			if (nicknumericHEAD == nnumericp)
			{
				nicknumericHEAD = nnumericp->next;
			}
			else
			{
				for (delete = nicknumericHEAD; (delete->next != nnumericp) && delete; delete = delete->next);
				delete->next = nnumericp->next;
			}

			ircsp_free (nnumericp->numeric);
			ircsp_free (nnumericp);

			status->numNumeric--;


			check = findNickNumeric (numeric);
			if (!check)
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  Numeric [%s] was deleted successfully\n",
					__FILE__, __LINE__, __FUNCTION__, numeric);
			}
			else
			{
				ERROR (150, "Failed to delete numeric\n", TRUE);
			}

			return ;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//  End of Linked List code
///////////////////////////////////////////////////////////////////////////




char *getFreeNumeric (char *numeric)
{
	NickNumeric *nnumericp;

	int yyPos = 0;
	int yPos = 0;
	int xPos = 0;


	LOG (MAINLOG, "[%s:%d:%s()]:  Getting a free NUMERIC\n",
		__FILE__, __LINE__, __FUNCTION__);

	numeric[0] = config->server_numeric[0];
	numeric[1] = config->server_numeric[1];


	for (xPos = 0; xPos < config->xPos; xPos++)
	{
		for (yPos = 0; yPos < config->yPos; yPos++)
		{
			for (yyPos = 0; yyPos < config->yyPos; yyPos++)
			{
				numeric[2] = convert2y[xPos];
				numeric[3] = convert2y[yPos];
				numeric[4] = convert2y[yyPos];
				numeric[5] = '\0';

				nnumericp = findNickNumeric (numeric);
				if (!nnumericp)
				{
					LOG (MAINLOG, "[%s:%d:%s()]:  Free numeric found [%s]\n",
						__FILE__, __LINE__, __FUNCTION__, numeric);

					return numeric;
				}

			}
		}
	}
}
