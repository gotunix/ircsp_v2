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
// $Revision: 1.28 $
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

u_long btx = 0, brx = 0;


void ircsp_connect (void)
{
	struct hostent *hp;
	struct sockaddr_in sin;

	ircsp_debug ("@(#) - (%s:%d) %s():  Conecting to UPLINK [%s:%d].\n",
		__FILE__, __LINE__, __FUNCTION__, config->uplink, config->uplink_port);

	if ((hp = gethostbyname(config->uplink)) == NULL)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Unknown hostname [%s].\n",
			__FILE__, __LINE__, __FUNCTION__, config->uplink);
		ircsp_panic ("ERROR - Unknown hostname");
	}

	if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Client:Socket.\n",
			__FILE__, __LINE__, __FUNCTION__);
		ircsp_panic ("Client:Socket");
	}

	sin.sin_family = AF_INET;
	sin.sin_port = htons(config->uplink_port);
	bcopy (hp->h_addr, &sin.sin_addr, hp->h_length);

	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Client:Connect.\n",
			__FILE__, __LINE__, __FUNCTION__);
		ircsp_debug ("Client:Connect");
	}

	serv = fdopen (s, "a+");
	service->connected = 1;
}

int ssprintf (int socket, const char *fmt, ...)
{
	int len;
	char *string;
	va_list args;

	string = (char *)ircsp_calloc(1, BUFFER_SIZE);
	va_start (args, fmt);
	vsnprintf (string, BUFFER_SIZE, fmt, args);
	va_end (args);

	ircsp_debug ("@(#) - (%s:%d) %s(): Socket(SEND):  %s",
		__FILE__, __LINE__, __FUNCTION__, string);

	send (socket, string, strlen(string), 0);
	len = strlen (string);
	ircsp_free (string);
	return (len);
} 
/*
int ssprintf (int socket, const char *fmt, ...)
{
	char	buf[BUFFER_SIZE];
	int	bwritten;

	va_list msg;

	if (s == -1)
	{
		return (-1);
	}

	va_start(msg, fmt);
	vsprintf(buf, fmt, msg);

	strcat (buf, "\0");
	va_end(msg);

	ircsp_debug ("@(#) - (%s:%d) %s():  Socket(SEND): %s",
		__FILE__, __LINE__, __FUNCTION__, buf);

	bwritten = write(s, buf, strlen(buf));

	btx += bwritten;
	return bwritten;
} */

int ssread (int s, char *buf)
{
	char inc;
	int bufnum, n;
	fd_set fds;
	struct timeval wait;


	bufnum = 0;

	if (s == -1)
	{
		return (-1);
	}

	wait.tv_sec = 0L;
	wait.tv_usec = 2500L;
	FD_ZERO(&fds);
	FD_SET(s, &fds);

	if (select(s+1, &fds, NULL, 0, &wait) > 0)
	{
		do
		{
			n = read(s, &inc, 1);
			if (n == 0)
			{
				return -1;
			}

			if (bufnum < BUFFER_SIZE - 1)
			{
				buf[bufnum++] = inc;
			}
		}
		while (inc != '\n');
		buf[bufnum] = '\0';

		brx += bufnum;
		return bufnum;
	}
	return 0;
}
