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
// $Revision: 1.22 $
// $Author: jovens $
// $Date: 2009/04/11 06:40:37 $
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

int isComment (const char *string)
{
	if ((string[0] == ';') || (string[0] == '#'))
	{
		return 1;
	}
	return 0;
}


int isBlank (const char *string)
{
	int i = 0;
	while (string[i])
	{
		if ((string[i] != ' ') && (string[i] != '\n') && (string[i] != '\t'))
		{
			return 0;
		}
		i++;
	}
	return 1;
}


char *lrange (char *tokenlist, int from)
{
	static char      token[1024];
	unsigned int	    curtok=1, tokenpos=0, curpos=0;

	while (curtok < from)
	{
		if (tokenlist[curpos++] == ' ')
			curtok++;
		else if ((tokenlist[curpos] == '\n') || (tokenlist[curpos] == '\0'))
			return NULL;
	}

	while ((tokenlist[curpos] != '\n') && 
	       (tokenlist[curpos] != '\0') &&
	       (tokenlist[curpos] != '\r'))
	{
		token[tokenpos++] = tokenlist[curpos++];
	}
	token[tokenpos] = '\0';
	return (char *)token;
}


/*
** Translate certain control characters into fields from the help
** files.
**
**     ^U       Nickname of UWorld
**     ^N	Nickname of NService
**     ^C	Nickname of CService
**     ^B       Bold.
*/

char *translate(char *inBuf, char *outBuf)
{
	int inBufPos = -1, outBufPos = -1;
	int pos = -1;
	int cmd;

//	char level[ 8 ];

	while(inBuf[ ++inBufPos ] != '\0')
	{
		if(inBuf[ inBufPos ] == '^')
		{
			switch(inBuf[ ++inBufPos ])
			{
				case 'U': // UWorld Nickname
					while(config->uworld_nickname[++pos] != '\0')
						outBuf[++outBufPos] = config->uworld_nickname[pos];
					break;

				case 'C': // CService Nickname
					while(config->cserve_nickname[++pos] != '\0')
						outBuf[++outBufPos] = config->cserve_nickname[pos];
					break;

				case 'N': // NService Nickname
					while(config->nserve_nickname[++pos] != '\0')
						outBuf[++outBufPos] = config->nserve_nickname[pos];
					break;

				case 'B': 
					outBuf[ ++outBufPos ] = '\002';
					break;
			}
		}
		else
			outBuf[ ++outBufPos ] = inBuf[ inBufPos ];
	}
	outBuf[ outBufPos ] = '\0';
	return outBuf;
}


char *token (char *tokenlist, int index)
{
	static char token[512];
	unsigned int curtok=1, curpos=0, tokenpos=0;

	while (curtok < index)
	{
		if (tokenlist[curpos++] == ' ')
		{
			curtok++;
		}
		if (tokenlist[curpos] == '\n')
		{
			return NULL;
		}
	}

	while ((tokenlist[curpos] != ' ')  && (tokenlist[curpos] != '\r') &&
		(tokenlist[curpos] != '\n') && (tokenlist[curpos] != '\0'))
	{
		token[tokenpos++] = tokenlist[curpos++];
	}

	token[tokenpos] = '\0';
	return (char *)token;
}


int tokenize (char *inlist, char sep)
{
	char outlist[ 1024 ];
	int  pos=0, num=1;

	while ((inlist[pos] != '\n') && (inlist[pos] != '\0'))
	{
		if (inlist[pos] == sep)
		{
			outlist[pos] = ' ';
			++num;
		}
		else
		{
			outlist[pos] = inlist[pos];
		}
		++pos;
	}

	outlist[pos] = '\0';

	strncpy(inlist, outlist, strlen(inlist));
	return num;
}

char *strtoupper(char *string)
{
	int i=0;
	while(i < strlen(string))
	{
		string[i] = toupper(string[i++]);
	}
	return string;
}

int numtokens(char *line)
{
	int pos=0, num_tok = 1;
	for (;pos<strlen(line); ++pos)
	{
		if (line[pos] == ' ')
		{
			num_tok += 1;
		}
	}

	return num_tok;
}

int strcascmp (char *in1, char *in2)
{
	unsigned int pos = 0;

	while((in1[pos] != '\n') && (in1[pos] != '\0'))
	{
		if (tolower(in1[pos]) != tolower(in2[pos]))
		{
			return 1;
		}
		++pos;
	}
	if (strlen(in1) == strlen(in2))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


char *chop (char *chopme)
{
	int x = strlen(chopme) - 1;

	if((chopme[ x ] == '\n') || (chopme[ x ] == '\r'))
	{
		chopme[ x ] = '\0';
	}

	return chopme;
}
   
