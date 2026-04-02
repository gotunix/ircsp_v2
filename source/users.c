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
// $Revision: 1.21 $
// $Author: jovens $
// $Date: 2009/04/11 06:40:37 $
//------------------------------------------------------------------------






#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>


#include "conf.h"
#include "users.h"

#include "macros.h"
#include "flags.h"
#include "ircsp.h"
#include "misc.h"

#include "users.h"
#include "servers.h"
#include "channels.h"



//////////////////////////////////////////////////////////////////////////
// User Linked Lists
//////////////////////////////////////////////////////////////////////////



//------------------------------------------------------------------------
// addNickname ()
//
//------------------------------------------------------------------------


User *addUser (char *nick, char *user, struct _hostname  *hostp, char *numeric, char *server, time_t stamp)
{
	User *userp;
	
	userp = (User *)ircsp_calloc(1, sizeof(User));
	if (!userp)
	{
		ircsp_debug (
			"[%s:%d:%s()]:  Failed to allocate memory for userHEAD.\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		ircsp_panic ("Failed to allocate memory");
	}
	else
	{
		ircsp_debug (
			"[%s:%d:%s()]:  Memory allocated for userHEAD at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, userp);
	}

	userp->nickname = (char *)ircsp_calloc(1, strlen(nick) + 5);
	if (!userp->nickname)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to allocate memory for NICKNAME\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		ircsp_panic ("Failed to allocate memory");
	}
	else
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Memory allocated for NICKNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, userp->nickname);
	}

	userp->username = (char *)ircsp_calloc(1, strlen(user) + 5);
	if (!userp->username)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to allocate memory for USERNAME\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		ircsp_panic ("Failed to allocate memory");
	}
	else
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Memory allocated for USERNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, userp->username);
	}

	userp->numeric = (char *)ircsp_calloc(1, strlen(numeric) + 5);
	if (!userp->numeric)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to allocate memory for NUMERIC\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		ircsp_panic ("Failed to allocate memory");
	}
	else
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Memory allocated for NUMERIC at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, userp->numeric);
	}

	userp->server = (char *)ircsp_calloc(1, strlen(server) + 5);
	if (!userp->server)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to allocate memory for SERVER\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		ircsp_panic ("Failed to allocate memory");
	}
	else
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Memory allocated for SERVER at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, userp->server);
	}

	ircsp_debug ("@(#) - (%s:%d) %s():  Adding Nickname [%s](%s@%s) - Numeric[%s] on Server[%s]\n",
		__FILE__, __LINE__, __FUNCTION__, nick, user, hostp->hostname, numeric, 
		server);

	strncpy (userp->nickname, nick, strlen(nick) +5);
	strncpy (userp->username, user, strlen(nick) + 5);
	strncpy (userp->numeric, numeric, strlen(numeric) + 5);
	strncpy (userp->server, server, strlen(server) + 5);

	userp->stamp = stamp;
	userp->lastsent = 0L;
	userp->numChans = 0;
	userp->flags = 0;
	userp->flood = 0;

	userp->chanhead = NULL;
	userp->hostp = hostp;
	userp->nservice_kill = 0;

	ircsp_debug ("@(#) - (%s:%d) %s():  Added Nickname[%s][%s](%s@%s)\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname, userp->numeric, 
		userp->username, userp->hostp->hostname);

	if (!userHEAD)
	{
		userHEAD = userp;
		userp->next = NULL;
	}
	else
	{
		userp->next = userHEAD;
		userHEAD = userp;
	}
	
	return userp;
}


//------------------------------------------------------------------------
// findNickname ()
//
//------------------------------------------------------------------------


User *findUser (char *nick)
{
	User *userp;

	ircsp_debug ("@(#) - (%s:%d) %s():  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, nick);

	userp = userHEAD;

	while (userp)
	{
		if (!strcasecmp(nick, userp->nickname))
		{
			ircsp_debug ("@(#) - (%s:%d) %s():  Nickname[%s] Found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, userp->nickname,
				userp->nickname);
				
			return userp;
		}
		userp = userp->next;
	}

	ircsp_debug ("@(#) - (%s:%d) %s():  Nickname[%s] Not Found.\n",
		__FILE__, __LINE__, __FUNCTION__, nick);

	return NULL;
}


//------------------------------------------------------------------------
// findNicknameByNumeric ()
//
//------------------------------------------------------------------------


User *findUserByNumeric (char *numeric)
{
	User *userp;

	ircsp_debug ("@(#) - (%s:%d) %s():  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, numeric);

	userp = userHEAD;

	while (userp)
	{
		if (!strcasecmp(numeric, userp->numeric))
		{
			ircsp_debug ("@(#) - (%s:%d) %s():  Nickname[%s] found at [%p] by Numeric[%s]\n",
				__FILE__, __LINE__, __FUNCTION__, userp->nickname, userp->nickname, numeric);

			return userp;
		}
		userp = userp->next;
	}

	ircsp_debug ("@(#) - (%s:%d) %s():  Nickname was not found by Numeric[%s].\n",
		__FILE__, __LINE__, __FUNCTION__, numeric);

	return NULL;
}


//------------------------------------------------------------------------
// delNickname ()
//
//------------------------------------------------------------------------


void delUser (char *nick)
{
	User *userp, *next, *delete; 

//	Nickname *check;

	ircsp_debug ("@(#) - (%s:%d) %s():  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, nick);

	for (userp = userHEAD; userp; userp = userp->next)
	{
		next = userp->next;
		if (!strcasecmp(userp->nickname, nick))
		{
			ircsp_debug ("@(#) - (%s:%d) %s():  Deleting Nickname[%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, userp->nickname,
				userp->nickname);

			if (userHEAD == userp)
			{
				userHEAD = userp->next;
			}
			else
			{
				for (delete = userHEAD; (delete->next != userp) && delete; delete = delete->next);
				delete->next = userp->next;
			}

			ircsp_free(userp->nickname);
			ircsp_free(userp->username);
			userp->hostp = NULL;
			ircsp_free(userp->numeric);
			ircsp_free(userp->server);
			ircsp_free(userp);

			return ;
		}

		ircsp_debug ("@(#) - (%s:%d) %s():  Nickname[%s] was not found.\n",
			__FILE__, __LINE__, __FUNCTION__, nick);
	}
}


//------------------------------------------------------------------------
// delNicknameByServer ()
//
//------------------------------------------------------------------------


void delUserByServer (char *server)
{
	User *userp;
	Hostname *hostp;
	Server *serverp;
	Admin *adminp;
	Channel *channelp;
	ChanList *chanlistp;

	ircsp_debug ("@(#) - (%s:%d) %s():  Deleting NICKNAMES on %s\n",
		__FILE__, __LINE__, __FUNCTION__, server);

	for (userp = userHEAD; userp; userp = userp->next)
	{
		if (!strcasecmp(userp->server, server))
		{
			ircsp_debug ("@(#) - (%s:%d) %s():  Deleting Nickname[%s] found at [%p] on Server[%s]\n",
				__FILE__, __LINE__, __FUNCTION__, userp->nickname, 
				userp->nickname, server);

			// Check hosts....

			hostp = findHostname (userp->hostp->hostname);
			if (!hostp)
			{
				ircsp_debug ("@(#) - (%s:%d) %s():  Hostname [%s] not found, possible memory leak?\n",
					__FILE__, __LINE__, __FUNCTION__, 
					userp->hostp->hostname);
			}
			else
			{
				if (hostp->numHosts > 1)
				{
					ircsp_debug ("@(#) - (%s:%d) %s():  Hostname [%s] found at [%p], decreasing host count.\n",
						__FILE__, __LINE__, __FUNCTION__, hostp->hostname,
						hostp->hostname);

					hostp->numHosts--;
				}
				else
				{
					ircsp_debug ("@(#) - (%s:%d) %s():  Hostname [%s] found at [%p], deleting.\n",
						__FILE__, __LINE__, __FUNCTION__, hostp->hostname,
						hostp->hostname);

					delHostname (hostp->hostname);
					status->numHosts--;
				}
			}

			serverp = findServerByNumeric(userp->server);
			if (!serverp) // Add Debugging..
				return ;

			serverp->numUsers--;
			status->numUsers--;

			if (IsOperator(userp))
			{
				serverp->numOpers--;
				status->numOpers--;
			}

			if (IsAdministrator (userp))
			{
				adminp = findAdmin (userp->adminp->nickname);
				if (IsAdminLogin(adminp))
				{
					DelAdminLogin (adminp);
				}
				DelAdministrator (userp);

				userp->adminp = NULL;
				adminp->userp = NULL;
			}

			chanlistp = userp->chanhead;
			while (chanlistp)
			{
				channelp = chanlistp->channelp;
				delChannelNick(channelp, userp);
				delNickChannel(userp, channelp);
				chanlistp = chanlistp->next;
			}
			delUser(userp->nickname);
		}
	}	
}



//////////////////////////////////////////////////////////////////////////
//  NickChan List
//////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------
// addNickChannel ()
//
//------------------------------------------------------------------------


ChanList *addNickChannel (struct _user *userp, struct _channel *channelp)
{
	ChanList *chanlistp;

	chanlistp = (ChanList *)ircsp_calloc(1, sizeof(ChanList));
	if (!chanlistp)
	{
		return NULL;
	}

	chanlistp->channelp = channelp;
	chanlistp->flags = 0;

	if (!userp->chanhead)
	{
		userp->chanhead = chanlistp;
		chanlistp->next = NULL;
	}
	else
	{
		chanlistp->next = userp->chanhead;
		userp->chanhead = chanlistp;
	}

	return chanlistp;
}


//------------------------------------------------------------------------
// findNickChannel ()
//
//------------------------------------------------------------------------


ChanList *findNickChannel (struct _user *userp, struct _channel *channelp)
{
	ChanList *chanlistp;
	chanlistp = userp->chanhead;
	while (chanlistp)
	{
		if (chanlistp->channelp = channelp)
		{
			return chanlistp;
		}
		chanlistp = chanlistp->next;
	}
	return NULL;
}


//------------------------------------------------------------------------
// delNickChannel ()
//
//------------------------------------------------------------------------


void delNickChannel (struct _user *userp, struct _channel *channelp)
{
	ChanList *chanlistp;
	ChanList *prev;

	prev = NULL;
	chanlistp = userp->chanhead;
	while (chanlistp)
	{
		if (chanlistp->channelp == channelp)
		{
			if (!prev)
			{
				userp->chanhead = chanlistp->next;
			}
			else
			{
				prev->next = chanlistp->next;
			}

			ircsp_free (chanlistp);
			return ;
		}
		prev = chanlistp;
		chanlistp = chanlistp->next;
	}
}


//////////////////////////////////////////////////////////////////////////
// Hostname Linked Lists
//////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------
// addHostname ()
//
//------------------------------------------------------------------------


Hostname *addHostname (char *hostname)
{
	Hostname *hostp;

	ircsp_debug("@(#) - (%s:%d) %s():  Running for [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, hostname);


	hostp = (Hostname *)ircsp_calloc(1, sizeof(Hostname));
	if (!hostp)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to allocate memory for hostHEAD.\n",
			__FILE__, __LINE__, __FUNCTION__);
		return NULL;
	}
	else
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Memory allocated for hostHEAD\n",
			__FILE__, __LINE__, __FUNCTION__);
	}

	hostp->hostname = (char *)ircsp_calloc(1, strlen(hostname) + 5);
	if (!hostp->hostname)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to allocate memory for HOSTNAME [%s]\n",
			__FILE__, __LINE__, __FUNCTION__);
		return NULL;
	}
	else
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Memory allocated for HOSTNAME [%s] at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, hostname, hostp->hostname);
	}

	strncpy(hostp->hostname, hostname, strlen(hostname) + 5);
	hostp->numHosts = 1;

	if (!hostHEAD)
	{
		hostHEAD = hostp;
		hostp->next = NULL;
	}
	else
	{
		hostp->next = hostHEAD;
		hostHEAD = hostp;
	}

	ircsp_debug ("@(#) - (%s:%d) %s():  Finished running for [%s]\n",
		__FILE__, __LINE__, __FUNCTION__);	

	return hostp;
}

//------------------------------------------------------------------------
// findHostname ()
//
//------------------------------------------------------------------------

Hostname *findHostname (char *hostname)
{
	Hostname *hostp;

	ircsp_debug ("@(#) - (%s:%d) %s():  Running for [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, hostname);

	hostp = hostHEAD;
	while (hostp)
	{
		if (!strcasecmp(hostp->hostname, hostname))
		{
			ircsp_debug ("@(#) - (%s:%d) %s():  Hostname [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, hostp->hostname,
				hostp->hostname);

			return hostp;
		}
		hostp = hostp->next;
	}

	ircsp_debug ("@(#) - (%s:%d) %s():  Hostname [%s] not found\n",
		__FILE__, __LINE__, __FUNCTION__, hostname);

	return NULL;
}

//------------------------------------------------------------------------
// delHostname ()
//
//------------------------------------------------------------------------

void delHostname (char *hostname)
{
	Hostname *hostp, *next, *delete, *check;

	ircsp_debug ("@(#) - (%s:%d) %s():  Running for [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, hostname);

	for (hostp = hostHEAD; hostp; hostp = next)
	{
		next = hostp->next;
		if (!strcasecmp(hostp->hostname, hostname))
		{
			ircsp_debug ("@(#) - (%s:%d) %s():  Hostname [%s] found at [%p], deleting.\n",
				__FILE__, __LINE__, __FUNCTION__, hostp->hostname,
				hostp->hostname);

			if (hostHEAD == hostp)
			{
				hostHEAD = hostp->next;
			}
			else
			{
				for (delete = hostHEAD; (delete->next != hostp) && delete; delete = delete->next);
				delete->next = hostp->next;
			}

			ircsp_free (hostp->hostname);
			ircsp_free (hostp);

			check = findHostname (hostname);
			if (!check)
			{
				ircsp_debug ("@(#) - (%s:%d) %s():  Hostname [%s] was deleted successfully.\n",
					__FILE__, __LINE__, __FUNCTION__, hostname);
			}
			else
			{
				ircsp_debug ("@(#) - (%s:%d) %s():  Hostname [%s] was not deleted.\n",
					__FILE__, __LINE__, __FUNCTION__, hostname);

				// here we could have it attempt to delete again, 
				// or restart if needed.
			}


			ircsp_debug ("@(#) - (%s:%d) %s():  Finished running on [%s]\n",
				__FILE__, __LINE__, __FUNCTION__, hostname);
			return ;
		}
	}
}




//////////////////////////////////////////////////////////////////////////
//  IRCSP Admin Database and linked lists
//////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------
// addAdmin ()
//
//------------------------------------------------------------------------


Admin *addAdmin (char *nickname, int level, char *hostmask, int flags, time_t lastseen, int suspended, char *password)
{
	Admin *adminp;


	ircsp_debug ("@(#) - (%s:%d) %s():  Running for %s (%s)\n",
		__FILE__, __LINE__, __FUNCTION__, nickname, 
		hostmask);

	adminp = (Admin *)ircsp_calloc(1, sizeof(Admin));
	if (!adminp)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to allocate memory for adminHEAD\n",
			__FILE__, __LINE__, __FUNCTION__);
		return NULL;
	}
	else
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Memory allocated for adminHEAD\n",
			__FILE__, __LINE__, __FUNCTION__);
	}

	adminp->nickname = (char *)ircsp_calloc(1, strlen(nickname) + 5);
	if (!adminp->nickname)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to allocate memory for NICKNAME\n",
			__FILE__, __LINE__, __FUNCTION__);
		return NULL;
	}
	else
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Memory allocated for NICKNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, adminp->nickname);
	}

	adminp->hostmask = (char *)ircsp_calloc(1, strlen(hostmask) + 5);
	if (!adminp->hostmask)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to allocate memory for HOSTMASK\n",
			__FILE__, __LINE__, __FUNCTION__);
		return NULL;
	}
	else
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Memory allocated for HOSTMASK at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, adminp->hostmask);
	}

	adminp->password = (char *)ircsp_calloc(1, strlen(password) + 5);
	if (!adminp->password)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to allocate memory for PASSWORD\n",
			__FILE__, __LINE__, __FUNCTION__);
		return NULL;
	}
	else
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Memory allocated for PASSWORD at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, adminp->password);
	}

	strncpy(adminp->nickname, nickname, strlen(nickname) + 5);
	strncpy(adminp->hostmask, hostmask, strlen(hostmask) + 5);
	strncpy(adminp->password, password, strlen(password) + 5);

	adminp->level = level;
	adminp->flags = flags;
	adminp->lastseen = lastseen;
	adminp->suspended = suspended;
	adminp->userp = NULL;

	ircsp_debug ("@(#) - (%s:%d) %s():  Admin[%s] (%s) Added.\n",
		__FILE__, __LINE__, __FUNCTION__, nickname,
		hostmask);

	if (!adminHEAD)
	{
		adminHEAD = adminp;
		adminp->next = NULL;
	}
	else
	{
		adminp->next = adminHEAD;
		adminHEAD = adminp;
	}

	ircsp_debug ("@(#) - (%s:%d) %s():  Finished running on %s (%s)\n",
		__FILE__, __LINE__, __FUNCTION__, nickname,
		hostmask);

	status->numAdmins++;

	return adminp;
}


//------------------------------------------------------------------------
// findAdmin ()
//
//------------------------------------------------------------------------


Admin *findAdmin (char *nickname)
{
	Admin *adminp;

	ircsp_debug ("@(#) - (%s:%d) %s():  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, nickname);

	adminp = adminHEAD;
	while (adminp)
	{
		if (!strcasecmp(adminp->nickname, nickname))
		{
			ircsp_debug ("@(#) - (%s:%d) %s():  Admin [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, adminp->nickname,
				adminp->nickname);
			return adminp;
		}
		adminp = adminp->next;
	}

	ircsp_debug ("@(#) - (%s:%d) %s():  Admin [%s] not found\n",
		__FILE__, __LINE__, __FUNCTION__, nickname);

	return NULL;
}


//------------------------------------------------------------------------
// delAdmin ()
//
//------------------------------------------------------------------------


void delAdmin (char *nickname)
{
	Admin *adminp, *next, *delete, *check;

	ircsp_debug ("@(#) - (%s:%d) %s():  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, nickname);

	for (adminp = adminHEAD; adminp; adminp = next)
	{
		next = adminp->next;

		if (!strcasecmp(adminp->nickname, nickname))
		{
			ircsp_debug ("@(#) - (%s:%d) %s():  Admin [%s] found at [%p], Deleting\n",
				__FILE__, __LINE__, __FUNCTION__, adminp->nickname,
				adminp->nickname);

			if (adminHEAD == adminp)
			{
				adminHEAD = adminp->next;
			}
			else
			{
				for (delete = adminHEAD; (delete->next != adminp) && delete; delete = delete->next);
				delete->next = adminp->next;
			}

			ircsp_free (adminp->nickname);
			ircsp_free (adminp->hostmask);
			ircsp_free (adminp->password);

			adminp->level = 0;
			adminp->flags = 0;
			adminp->lastseen = 0;
			adminp->suspended = 0;
			adminp->numHosts = 0;

			ircsp_free (adminp);

			check = findAdmin (nickname);
			if (!check)
			{
				ircsp_debug ("@(#) - (%s:%d) %s():  Admin [%s] was deleted successfully\n",
					__FILE__, __LINE__, __FUNCTION__, nickname);
				status->numAdmins--;
			}
			else
			{
				ircsp_debug ("@(#) - (%s:%d) %s():  Admin [%s] was not deleted successfully.\n",
					__FILE__, __LINE__, __FUNCTION__, nickname);
			}


			ircsp_debug ("@(#) - (%s:%d) %s():  Finished running on [%s]\n",
				__FILE__, __LINE__, __FUNCTION__, nickname);
			return ;
		}
	}
}


//------------------------------------------------------------------------
//  loadAdminDB ()
//
//------------------------------------------------------------------------


int loadAdminDB (void)
{
	FILE *f;
	Admin *adminp;
	char inBuf[256], *argv[9];

	ircsp_debug ("@(#) - (%s:%d) %s():  Loading Admin Database\n",
		__FILE__, __LINE__, __FUNCTION__);


	f = fopen(config->admin_db, "r");
	if (!f)
	{
		ircsp_panic ("Unable to open admin configuration file!");
	}

	while (!feof(f))
	{
		if (fgets(inBuf, 256, f))
		{
			if ((inBuf[0] <= 32) || (inBuf[0] == ';'))
			{
				continue;
			}

			argv[0] = strtok(inBuf, ":"); // Nickname
			argv[1] = strtok(NULL, ":");  // Level
			argv[2] = strtok(NULL, ":");	// Password
			argv[3] = strtok(NULL, ":");	// Hostmask
			argv[4] = strtok(NULL, ":");	// Flags
			argv[5] = strtok(NULL, ":");	// Last Seen
			argv[6] = strtok(NULL, ":");	// Suspended

			adminp = addAdmin(argv[0], atoi(argv[1]), argv[3], atoi(argv[4]), atol(argv[5]), atol(argv[6]), argv[2]);

			if (atol(argv[6]) & ADMIN_SUSPENDED)
			{
				SetAdminSuspended (adminp);
			}
		}
	}
	fclose(f);
	return 1;
}
