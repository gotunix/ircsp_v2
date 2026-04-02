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
// $Revision: 1.43 $
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


void m_nick (char *sBuf)
{
	LOG (MAINLOG, "[%s:%d:%s()]:  Starting NICK parser\n",
		__FILE__, __LINE__, __FUNCTION__);

	if (IsProtocolUndernet (config))
	{
		User *nuserp; 			// New Nick Pointer
		User *cuserp;			// Nick Change Pointer
		Hostname *hostp;		// Host Pointer
		Server *serverp;		// Server Pointer

		char *argv[9];
		time_t stamp;


		argv[0] = strtok(sBuf, " "); // Server numeric
		strtok(NULL, " "); // Nick Command
		argv[1] = strtok(NULL, " ");

		cuserp = findUserByNumeric(argv[0]);
		if (cuserp)
		{
			User *userp;

			LOG (MAINLOG, "[%s:%d:%s()]:  NICK change\n",
				__FILE__, __LINE__, __FUNCTION__);

			userp = findUser(argv[1]);
			if (userp)
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  Nickname in use? Deleting\n",
					__FILE__, __LINE__, __FUNCTION__);

				delUser(argv[1]);
			}
			else
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  Completing NICK change\n",
					__FILE__, __LINE__, __FUNCTION__);

				hostp = findHostname (cuserp->hostp->hostname);
	
				userp = addUser(argv[1], cuserp->username, 
							hostp, cuserp->numeric, 
							cuserp->server, cuserp->stamp);

				if ((IsAdministrator(cuserp)) && (IsOperator(cuserp)))
				{
					Admin *adminp;

					LOG (MAINLOG, "[%s:%d:%s()]:  Nickname [%s] was admin.  Moving flags\n",
						__FILE__, __LINE__, __FUNCTION__, cuserp->nickname);

					adminp = findAdmin(cuserp->adminp->nickname);
				

					SetOperator(userp);
					SetAdministrator(userp);
					DelAdministrator(cuserp);
					DelOperator(cuserp);

					cuserp->adminp = NULL;
					adminp->userp = NULL;

					userp->adminp = adminp;
					adminp->userp = userp;
				}

				else if (IsOperator(cuserp))
				{
					LOG (MAINLOG, "[%s:%d:%s()]:  Nickname [%s] was oper, Moving flags\n",
						__FILE__, __LINE__, __FUNCTION__, cuserp->nickname);

					SetOperator(userp);
					DelOperator(cuserp);
				} 

				LOG (MAINLOG, "[%s:%d:%s()]:  Deleting [%s]\n",
					__FILE__, __LINE__, __FUNCTION__, cuserp->nickname);

				delUser(cuserp->nickname); 
			
				LOG (MAINLOG, "[%s:%d:%s()]:  NICK change completed\n",
					__FILE__, __LINE__, __FUNCTION__);

				return ;
			}
		}
		else
		{
			nuserp = findUser(argv[1]);
			if (nuserp)
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  NICK already in use? Deleting\n",
					__FILE__, __LINE__, __FUNCTION__);

				delUser (argv[1]);
			}
			else
			{
				int is_oper = 0;

				strtok(NULL, " "); // SKIP
				stamp = atol(strtok(NULL, " "));
				argv[2] = strtok(NULL, " "); // Username
				argv[3] = strtok(NULL, " "); // Hostname
				argv[4] = strtok(NULL, " "); // Modes

				serverp = findServerByNumeric (argv[0]);

				if (argv[4][0] != '+')
				{
					is_oper = 0;
				}
				else
				{
					while (*++argv[4] != '\0')
					{
						if (*argv[4] == 'o')
							is_oper = 1;
					}
					strtok(NULL, " "); // Skip
				}
				argv[5] = strtok(NULL, " "); // Numeric

				LOG (MAINLOG, "[%s:%d:%s()]:  New NICK [%s] (%s@%s)  Numeric [%s] Operator [%d] on Server [%s]\n",
					__FILE__, __LINE__, __FUNCTION__, argv[1], argv[2], argv[3], argv[5], is_oper,
					serverp->name);

				// Check hostname * (clone list)

				LOG (MAINLOG, "[%s:%d:%s()]:  Checking HOSTNAME [%s]\n",
					__FILE__, __LINE__, __FUNCTION__, argv[3]);

				hostp = findHostname (argv[3]);
				if (!hostp)
				{
					LOG (MAINLOG, "[%s:%d:%s()]:  Adding new hostname [%s]\n",
						__FILE__, __LINE__, __FUNCTION__, argv[3]);

					hostp = addHostname (argv[3]);
					status->numHosts++;
				}
				else
				{
					LOG (MAINLOG, "[%s:%d:%s()]:  Hostname [%s] found at [%p] - Increasing count\n",
						__FILE__, __LINE__, __FUNCTION__, hostp->hostname, hostp->hostname);

					hostp->numHosts++;

					if (hostp->numHosts >= config->clone_warn)
					{
						ssprintf(s, "%s WA :Possible [\002%d\002] clones detected from [\002%s\002]\n",
							config->server_numeric, hostp->numHosts, 
							hostp->hostname);
					}
				}

				nuserp = addUser (argv[1], argv[2], hostp, argv[5], serverp->numeric, stamp);
				if (is_oper)
				{
					SetOperator (nuserp);
					serverp->numOpers++;
					status->numOpers++;
				}
				serverp->numUsers++;
				status->numUsers++;
			}
		}
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Finishing NICK parser\n",
		__FILE__, __LINE__, __FUNCTION__);
}
