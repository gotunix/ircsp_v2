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
// $Revision: 1.23 $
// $Author: jovens $
// $Date: 2009/04/11 06:55:09 $
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
#include "m_ping.h"
#include "m_parse.h"
#include "debug.h"
#include "ircsp.h"
#include "socket.h"
#include "log.h"
#include "flags.h"
#include "macros.h"

//AC G !1236971750.709779 UWorld.GOTUNIX.NET 1236971750.709779

//@(#) - (socket.c:168) ssprintf():  Socket(SEND): AE Z AC AC -32768 4646262703.916172

// We need to reply with the following:
// <my_numeric> Z <remote_numeric> !<remotets> <difference> <localts>

void m_ping (char *sBuf)
{
	LOG (MAINLOG, "[%s:%d:%s()]:  Starting PING parser\n",
		__FILE__, __LINE__, __FUNCTION__);

	if (IsProtocolUndernet (config))
	{
		char *delim;
		struct timeval orig;
		struct timeval now;
		int diff;

		char *argv[4];

		argv[0] = strtok(sBuf, " "); // Remote Numeric (who)
		strtok(NULL, " ");
		argv[1] = strtok(NULL, " "); // Remote TS w/ !
		strtok(NULL, " "); // Our server name
		argv[2] = strtok(NULL, " "); // Remote TS w/o ! (orig_ts)

		orig.tv_sec = strtoul(argv[2], &delim, 10);
		orig.tv_usec = (*delim == '.') ? strtoul(delim + 1, NULL, 10) : 0;
		gettimeofday(&now, NULL);
		diff = (now.tv_sec - orig.tv_sec) * 1000 + (now.tv_usec - orig.tv_usec) / 1000;	

		ssprintf (s, 
			"%s Z %s %s %d %d.%d\n",
			config->server_numeric, argv[0], argv[1], diff, now.tv_sec, 
			(unsigned)now.tv_usec);

	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Finished PING parser\n",
		__FILE__, __LINE__, __FUNCTION__);
}
