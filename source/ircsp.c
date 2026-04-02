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
// $Revision: 1.129 $
// $Author: jovens $
// $Date: 2009/04/11 07:38:55 $
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


#include "version.h"
#include "log.h"
#include "conf.h"
#include "flags.h"
#include "macros.h"
#include "users.h"
#include "memory.h"
#include "numeric.h"
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
#include "socket.h"
#include "strings.h"
#include "uworld.h"


static const char copyright[] = \
	"@(#)  IRCSP [Internet Relay Chat Service Package]\n" \
	"@(#)  Copyright (C) 1998 - 2009 GOTUNIX Networks.\n";

//------------------------------------------------------------------------
// API:  ircsp_exit ()
//------------------------------------------------------------------------


void ircsp_exit (int exit_code, char *user, char *message)
{
	User *userp;
	Hostname *hostp;
	Server *serverp;
	Admin *adminp;			// Done
	Gline *glinep;			// Done
	Channel *channelp;
	ChanList *chanlistp;
	Ban *banp;
	NickNumeric *nnumericp;		// Done
	NickJupe *njupep;		// Done


	LOG (MAINLOG, "[%s:%d:%s()]:  IRCSP Exiting [Exit Code: %d] - [Exit Requested By: %s] - [Exit Reason: %s]\n",
		__FILE__, __LINE__, __FUNCTION__, exit_code, user, message);
	 
	 
	for (njupep = nickjupeHEAD; njupep; njupep = njupep->next)
	{
		delNickJupe(njupep->nickname);
	}

	for (nnumericp = nicknumericHEAD; nnumericp; nnumericp = nnumericp->next)
	{
		delNickNumeric (nnumericp->numeric);
	}
	
	for (glinep = glineHEAD; glinep; glinep = glinep->next)
	{
		delGline (glinep->mask);
	}
	
	for (adminp = adminHEAD; adminp; adminp = adminp->next)
	{
		if (IsAdminLogin(adminp))
		{
			DelAdminLogin(adminp);
			adminp->userp = NULL;
		}
		delAdmin (adminp->nickname);
	}

	for (userp = userHEAD; userp; userp = userp->next)
	{
		if (IsAdministrator(userp))
		{
			userp->adminp = NULL;
			DelAdministrator(userp);
		}
		
		hostp = findHostname (userp->hostp->hostname);
		if (hostp->numHosts > 1)
		{
			hostp->numHosts--;
		}
		else
		{
			delHostname(userp->hostp->hostname);
		}
		
		userp->hostp = NULL;
		
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
	
	for (serverp = serverHEAD; serverp; serverp = serverp->next)
	{
		delServer (serverp->name);
	}
	
	for (channelp = channelHEAD; channelp; channelp = channelp->next)
	{
		banp = channelp->banhead;
		while (banp)
		{
			delChannelBan(channelp, banp->mask);
			banp = banp->next;
		}
		delChannel(channelp->name);
	}

	ircsp_free (config->server);
	ircsp_free (config->server_comment);
	ircsp_free (config->server_numeric);
	ircsp_free (config->uplink);
	ircsp_free (config->uplink_pass);
	ircsp_free (config->uworld_nickname);
	ircsp_free (config->uworld_username);
	ircsp_free (config->uworld_hostname);
	ircsp_free (config->uworld_realname);
	ircsp_free (config->uworld_ircmodes);
	ircsp_free (config->cserve_nickname);
	ircsp_free (config->cserve_username);
	ircsp_free (config->cserve_hostname);
	ircsp_free (config->cserve_ircmodes);
	ircsp_free (config->nserve_nickname);
	ircsp_free (config->nserve_username);
	ircsp_free (config->nserve_hostname);
	ircsp_free (config->nserve_ircmodes);
	ircsp_free (config->operchannel_name);
	ircsp_free (config->operchannel_modes);
	ircsp_free (config->deny_message);
	ircsp_free (config->jupe_username);
	ircsp_free (config->jupe_hostname);
	ircsp_free (config->jupe_realname);
	ircsp_free (config->jupe_ircmodes);
	ircsp_free (config->jupe_database);
	ircsp_free (config->admin_db);
	ircsp_free (config->gline_db);
	ircsp_free (config->uw_cmddb);


	ircsp_showleaks ();


	LOG (MAINLOG, "[%s:%d:%s()]:  Shutdown complete, Exiting.\n",
		__FILE__, __LINE__, __FUNCTION__);

	exit (exit_code);
}


//------------------------------------------------------------------------
// API:  ircsp_panic ()
//------------------------------------------------------------------------

void ircsp_panic (char *message)
{
	LOG (MAINLOG, "[%s:%d:%s()]:  !PANIC! [Message: %s]\n",
		__FILE__, __LINE__, __FUNCTION__);

	(void)printf("!PANIC! [Message: %s]\n",
		message);

	ircsp_exit (100, "Panic", "PANIC!");
}

//------------------------------------------------------------------------
// ircsp_sighup
//------------------------------------------------------------------------

void ircsp_sighup (int num)
{
	ircsp_rehash ();
}


//------------------------------------------------------------------------
// ircsp_sigsegv
//------------------------------------------------------------------------

void ircsp_sigsegv (int num)
{
	LOG (MAINLOG, "[%s:%d:%s()]:  SIGSEGV Received [PANIC]\n",
		__FILE__, __LINE__, __FUNCTION__);
		
	ssprintf(s, "%s WA :Signal SEGV! Ahh Dying....\n",
		config->server_numeric);
	ircsp_panic ("SIGSEGV");
}

//------------------------------------------------------------------------
// ircsp_sigint
//------------------------------------------------------------------------

void ircsp_sigint (int num)
{
	LOG (MAINLOG, "[%s:%d:%s:() SIGINT Received (QUITING)\n",
		__FILE__, __LINE__, __FUNCTION__);
		
	ssprintf(s, "%s WA :Signal INT (Quiting)\n",
		config->server_numeric);

	ircsp_exit (0, "SIGINT", "Ending");
}

//------------------------------------------------------------------------
// ircsp_sigterm ()
//------------------------------------------------------------------------

void ircsp_sigterm (int num)
{
	ircsp_exit (200, "SIGTERM", "Exiting");
}

//------------------------------------------------------------------------
//  API:  ircsp_signals
//------------------------------------------------------------------------

int ircsp_signals (void)
{
	struct sigaction sa;
	sa.sa_handler = ircsp_sighup;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGHUP, &sa, (struct sigaction *)0);

	sa.sa_handler = ircsp_sigsegv;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGSEGV, &sa, (struct sigaction *)0);

	sa.sa_handler = ircsp_sigint;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, (struct sigaction *)0);

	sa.sa_handler = ircsp_sigterm;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGTERM, &sa, (struct sigaction *)0);

	return 0;
}

//------------------------------------------------------------------------
// ircsp_startup ()
//
//   Initializes all of the lists and pointers to keep them from pointing
//   to micellaneous garbage.
//------------------------------------------------------------------------

void ircsp_startup (int argc, char *argv[])
{
	MAINLOG = fopen (__MAINLOG__, "a");
	if (!MAINLOG)
	{
		ERROR (103, "Unable to open log file\n", TRUE);
	}


	LOG (MAINLOG, "[%s:%d:%s()]:  IRCSP [Codename: %s] [Version: %s] Starting\n",
		__FILE__, __LINE__, __FUNCTION__, code_name, version_number);

	ircsp_initmemory ();


	LOG (MAINLOG, "[%s:%d:%s()]:  Allocating memory for serviceHEAD\n",
		__FILE__, __LINE__, __FUNCTION__);

	service = (Service *)ircsp_calloc(1, sizeof(Service));
	if (!service)
	{
		ERROR (150, "Failed to allocate memory for serviceHEAD\n", TRUE);
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for serviceHEAD at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, service);

	
	service->argc = argc;
	service->argv = argv;
	service->pid = getpid ();
	service->uid = getuid ();
	service->euid = geteuid ();
	service->uptime = time(NULL);
	service->connected = 0;
	service->initial_burst = 0;
	service->rehashing = 0;
	service->last_save_cycle = time(NULL);
	service->last_gline_cycle = time(NULL);
	service->last_nick_cycle = time(NULL);


	LOG (MAINLOG, "[%s:%d:%s()]:  Allocating memory for statusHEAD\n",
		__FILE__, __LINE__, __FUNCTION__);

	status = (Status *)ircsp_calloc(1, sizeof(Status));
	if (!status)
	{
		ERROR (150, "Failed to allocate memory for statusHEAD\n", TRUE);
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for statusHEAD at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, status);

	status->numUsers = 0;
	status->numServers = 0;
	status->numChannels = 0;
	status->numOpers = 0;
	status->numAdmins = 0;
	status->numHosts = 0;
	status->numGlines = 0;
	status->numNumeric = 0;


	nicknumericHEAD = NULL;
	serverHEAD = NULL;
	userHEAD = NULL;
	hostHEAD = NULL;
	adminHEAD = NULL;
	channelHEAD = NULL;

}

//------------------------------------------------------------------------
//  main ()
//
//
//  Lets keep this as short as we can.  Setup our enviornment, 
//  load our config files, connect to our UPLINK, and move
//  to our main loop.
//
//------------------------------------------------------------------------


int main (int argc, char *argv[])
{
	char inBuf[2048];

	// Command line parameters

	while (--argc > 0 && (*++argv)[0] == '-')
	{
		char *p = argv[0] + 1;
		int flag = *p++;

		if (flag == '\0' || *p == '\0')
		{
			if (argc > 1 && argv[1][0] != '0')
			{
				p = *++argv;
				argc -= 1;
			}
			else
			{
				p = "";
			}
		}

		switch (flag)
		{
			int i;
			case 'V':
			case 'v':
				(void)printf("---------------------------------------------------\n");
				for (i = 0; info_text[i]; i++)
				{
					(void)printf("%s\n", info_text[i]);
				}
				(void)printf("---------------------------------------------------\n");
				(void)printf("IRCSP Version Information:  \n");
				(void)printf("IRCSP Codename: %s\n", code_name);
				(void)printf("IRCSP Version: %s\n", version_number);
				(void)printf("IRCSP BUILD: %s\n", version_build);
				(void)printf("---------------------------------------------------\n");
				exit(0);
				break;
		}
	}


	ircsp_startup (argc, argv);

	// Load and parse our configuration file.
	ircsp_loadConfig ();  
	loadAdminDB ();
	uw_loadGlineDB ();
	uw_loadCommand ();
	uw_loadNickJupe ();


	if (config->fork)
	{
		FILE *pidFile;
		if (fork())
			exit(0);

		// Dump out a PID file.

		pidFile = fopen("ircsp.pid", "w");
		if (pidFile)
			fprintf(pidFile, "%d\n", service->pid);
		fclose(pidFile);
	}



	ircsp_connect ();

	if (service->connected)
	{
		//  We are connected to our uplink, bring up our server,
		//  and start our loop.

		LOG (MAINLOG, "[%s:%d:%s()]:  Connected to UPLINK, Logging in\n",
			__FILE__, __LINE__, __FUNCTION__);

		ircsp_login ();
		while (s)
		{
			if (ssread(s, inBuf))
			{
				LOG (MAINLOG, "[%s:%d:%s()]:  Moving to PARSER\n",
					__FILE__, __LINE__, __FUNCTION__);

				m_parse (inBuf);
			}

			if ((time(NULL) - service->last_save_cycle) > config->save_cycle)
			{
				ircsp_notice (
					"\002CYCLE\002 - Auto saving databases [Cycle: %d]\n",
					config->save_cycle);

				if (!service->rehashing)
				{
					writeAdminDB ();
					uw_saveGlineDB ();
					uw_saveCommand ();
					uw_saveNickJupe ();
				}
				service->last_save_cycle = time(NULL);
			}

			if ((time(NULL) - service->last_gline_cycle) > config->gline_cycle)
			{
				ircsp_notice (
					"\002CYCLE\002 - Checking for expired GLINES [Cycle: %d]\n",
					config->gline_cycle);


				if (!service->rehashing)
				{
					checkGlines ();
				}
				service->last_gline_cycle = time(NULL);
			}
		}
	}

	// We should never reach this point.
	ircsp_exit(0,"System","Program Stop");
	return 0;
}

//------------------------------------------------------------------------
//  ircsp_login ()
//
//  Send our password, bring up our server, and add our server to 
//  the server linked list.
//------------------------------------------------------------------------

void ircsp_login (void)
{
	Server *myserverp;

	LOG (MAINLOG, "[%s:%d:%s()]:  Logging into UPLINK\n",
		__FILE__, __LINE__, __FUNCTION__);

	if (IsProtocolUndernet (config))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  |-  UNDERNET\n",
			__FILE__, __LINE__, __FUNCTION__);

		// Send login data (password, and server)
		ssprintf (s, 
			"PASS :%s\n", config->uplink_pass);

		ssprintf (s, 
			"SERVER %s 1 %d %d J10 %s]]] :%s\n",
			config->server, time(NULL), time(NULL), config->server_numeric,
			config->server_comment);



		LOG (MAINLOG, "[%s:%d:%s()]:  Adding server[%s][%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->server,
			config->server_numeric);

		// Add our server to the linked list.
		myserverp = addServer(config->server, config->server_numeric, time(NULL));
	}
}

//------------------------------------------------------------------------
//  ircsp_spawn ()
//
//  Spawn service nicks, and add them to the Nickname list.
//------------------------------------------------------------------------

void ircsp_spawnUW (void)
{
	User *myuserp;
	Channel *channelp;
	ChanList *chanlistp;
	NickList *nicklistp;
	Server *myserverp;
	Hostname *hostp;
	NickNumeric *nnumericp;
	
	char *numeric;
	numeric = (char *)ircsp_calloc(1, 10);
	getFreeNumeric(numeric);
	nnumericp = addNickNumeric(numeric);

	LOG (MAINLOG, "[%s:%d:%s()]:  Spawning service bot [%s / %s]\n",
		__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname, numeric);

	myserverp = findServer(config->server);

	ssprintf(s, "%s N %s 1 0 %s %s %s AAAAAA %s :%s\n",
		myserverp->numeric, config->uworld_nickname, 
		config->uworld_username, config->uworld_hostname, 
		config->uworld_ircmodes, numeric,
		config->uworld_realname);

	LOG (MAINLOG, "[%s:%d:%s()]:  Nickname[%s] Numeric[%s] Online - Adding Nickname\n",
		__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname, numeric);

	hostp = findHostname (config->uworld_hostname);
	if (!hostp)
	{
		// add our nickname to the Nickname list
		LOG (MAINLOG, "[%s:%d:%s()]:  Adding hostname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_hostname);

		hostp = addHostname (config->uworld_hostname);
	}
	else
	{
		// Hostname found, increasing clone count.

		LOG (MAINLOG, "[%s:%d:%s()]:  Increasing numHosts\n",
			__FILE__, __LINE__, __FUNCTION__);

		hostp->numHosts++;
	}

	myuserp = addUser (config->uworld_nickname, config->uworld_username, 
				hostp, numeric, config->server_numeric, 
				0);

	status->numUsers++;

	LOG (MAINLOG, "[%s:%d:%s()]:  Setting flags, joining channels, setting modes\n",
		__FILE__, __LINE__, __FUNCTION__);

	// set OPER & SERVICE flags.

	SetOperator(myuserp);
	SetService (myuserp);

	ssprintf(s, "%s M %s :%s\n", myuserp->numeric, 
		config->uworld_nickname, 
		config->uworld_ircmodes);

	channelp = findChannel(config->operchannel_name);
	if (!channelp)
		channelp = addChannel(config->operchannel_name, 0, time(NULL), NULL, 0);

	nicklistp = addChannelNick(channelp, myuserp);
	chanlistp = addNickChannel(myuserp, channelp);

	chanlistp->flags |= IS_CHANOP;
	nicklistp->flags |= IS_CHANOP;

	channelp = findChannel(config->debugchannel_name);
	if (!channelp)
		channelp = addChannel(config->debugchannel_name, 0, time(NULL), NULL, 0);

	chanlistp = addNickChannel(myuserp, channelp);
	nicklistp = addChannelNick(channelp, myuserp);

	nicklistp->flags |= IS_CHANOP;
	chanlistp->flags |= IS_CHANOP;


	ssprintf(s, "%s J %s\n", 
		myuserp->numeric, config->operchannel_name);
	ssprintf(s, "%s M %s %s %s\n", myserverp->numeric,
		config->operchannel_name, config->operchannel_modes); 

	ssprintf(s, 
		"%s M %s +o %s\n",
		myserverp->numeric, config->operchannel_name,
		myuserp->numeric);

	// Join Debug channel and set modes.

	ssprintf(s, "%s J %s\n", 
		myuserp->numeric, config->debugchannel_name);
	ssprintf (s,
		"%s M %s +o %s\n",
		myserverp->numeric, config->debugchannel_name,
		myuserp->numeric);

	ssprintf(s, 
		"%s M %s %s\n", 
		myserverp->numeric, config->debugchannel_modes,
		config->debugchannel_name);
		
	ircsp_free(numeric);
}


//------------------------------------------------------------------------
//  ircsp_spawn ()
//
//  Spawn service nicks, and add them to the Nickname list.
//------------------------------------------------------------------------

void ircsp_spawnCS (void)
{
	User *myuserp;
	Server *myserverp;
	Channel *channelp;
	NickList *nicklistp;
	ChanList *chanlistp;
	Hostname *hostp;
	NickNumeric *nnumericp;

	char *numeric;

	numeric = (char *)ircsp_calloc(1, 10);
	getFreeNumeric(numeric);
	nnumericp = addNickNumeric(numeric);
	
	LOG (MAINLOG, "[%s:%d:%s()]:  Spawning service bot [%s / %s]\n",
		__FILE__, __LINE__, __FUNCTION__, config->cserve_nickname,
		numeric);

	myserverp = findServer(config->server);

	ssprintf(s, "%s N %s 1 0 %s %s %s %s :%s\n",
		myserverp->numeric, config->cserve_nickname, 
		config->cserve_username, config->cserve_hostname, 
		config->cserve_ircmodes, numeric,
		config->cserve_realname);

	LOG (MAINLOG, "[%s:%d:%s()]:  Nickname[%s] Numeric[%s] Online - Adding nickname\n",
		__FILE__, __LINE__, __FUNCTION__, config->cserve_nickname,
		numeric);

	// add our nickname to the Nickname list

	hostp = findHostname (config->cserve_hostname);
	if (!hostp)
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  Adding hostname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->cserve_hostname);

		hostp = addHostname (config->cserve_hostname);
	}
	else
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  Increasing numHosts\n",
			__FILE__, __LINE__, __FUNCTION__);

		hostp->numHosts++;
	}

	myuserp = addUser (config->cserve_nickname, config->cserve_username, 
				hostp, numeric, config->server_numeric, 0);

	status->numUsers++;

	LOG (MAINLOG, "[%s:%d:%s()]:  Setting flags, joining channels, setting modes\n",
		__FILE__, __LINE__, __FUNCTION__);

	// set OPER & SERVICE flags.

	SetOperator (myuserp);
	SetService  (myuserp);

	ssprintf(s, "%s M %s :%s\n", myuserp->numeric, 
		config->cserve_nickname, 
		config->cserve_ircmodes);

	channelp = findChannel(config->operchannel_name);
	if (!channelp)
		addChannel(config->operchannel_name, 0, time(NULL), NULL, 0);

	nicklistp = addChannelNick(channelp, myuserp);
	chanlistp = addNickChannel(myuserp, channelp);

	nicklistp->flags |= IS_CHANOP;
	chanlistp->flags |= IS_CHANOP;

	channelp = findChannel(config->debugchannel_name);
	if (!channelp)
		addChannel(config->debugchannel_name, 0, time(NULL), NULL, 0);

	nicklistp = addChannelNick(channelp, myuserp);
	chanlistp = addNickChannel(myuserp, channelp);

	nicklistp->flags |= IS_CHANOP;
	chanlistp->flags |= IS_CHANOP;

	ssprintf(s, "%s J %s\n", 
		myuserp->numeric, config->operchannel_name);
	ssprintf(s, "%s M %s +som %s\n", myserverp->numeric,
		config->operchannel_name, myuserp->numeric); 
	// Join Debug channel and set modes.

	ssprintf(s, "%s J %s\n", 
		myuserp->numeric, config->debugchannel_name);

	ssprintf (s,
		"%s M %s +o %s\n",
		myserverp->numeric, config->debugchannel_name,
		myuserp->numeric);

	ssprintf(s, "%s M %s +som\n", myserverp->numeric,
		config->debugchannel_name,
		config->debugchannel_modes );
		
	ircsp_free (numeric);
}

//------------------------------------------------------------------------
//  ircsp_spawn ()
//
//  Spawn service nicks, and add them to the Nickname list.
//------------------------------------------------------------------------

void ircsp_spawnNS (void)
{
	User *myuserp;
	Server *myserverp;
	Channel *channelp;
	NickList *nicklistp;
	ChanList *chanlistp;
	Hostname *hostp;
	NickNumeric *nnumericp;

	char *numeric;
	numeric = (char *)ircsp_calloc(1, 10);
	
	getFreeNumeric(numeric);
	nnumericp = addNickNumeric(numeric);
	
	LOG (MAINLOG, "[%s:%d:%s()]:  Spawning service bot [%s / %s]\n",
		__FILE__, __LINE__, __FUNCTION__, config->nserve_nickname,
		numeric);

	myserverp = findServer(config->server);

	ssprintf(s, "%s N %s 1 0 %s %s %s %s :%s\n",
		myserverp->numeric, config->nserve_nickname, 
		config->nserve_username, config->nserve_hostname, 
		config->nserve_ircmodes, numeric,
		config->nserve_realname);

	LOG (MAINLOG, "[%s:%d:%s()]:  Nickname[%s] Numeric[%s] Online - Adding nickname\n",
		__FILE__, __LINE__, __FUNCTION__, config->nserve_nickname,
		numeric);

	hostp = findHostname (config->nserve_hostname);
	if (!hostp)
	{
		// add our nickname to the Nickname list
		LOG (MAINLOG, "[%s:%d:%s()]:  Adding hostname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->nserve_hostname);

		hostp = addHostname (config->nserve_hostname);
	}
	else
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  Increasing numHosts\n",
			__FILE__, __LINE__, __FUNCTION__);

		hostp->numHosts++;
	}

	myuserp = addUser (config->nserve_nickname, config->nserve_username, 
				hostp, numeric, config->server_numeric, 0);

	status->numUsers++;

	LOG (MAINLOG, "[%s:%d:%s()]:  Setting flags, joining channels, setting modes\n",
		__FILE__, __LINE__, __FUNCTION__);

	// set OPER & SERVICE flags.

	SetOperator (myuserp);
	SetService  (myuserp);

	ssprintf(s, "%s M %s :%s\n", myuserp->numeric, 
		config->nserve_nickname, 
		config->nserve_ircmodes);

	channelp = findChannel(config->operchannel_name);
	if (!channelp)
		addChannel(config->operchannel_name, 0, time(NULL), NULL, 0);

	nicklistp = addChannelNick(channelp, myuserp);
	chanlistp = addNickChannel(myuserp, channelp);

	nicklistp->flags |= IS_CHANOP;
	chanlistp->flags |= IS_CHANOP;

	channelp = findChannel(config->debugchannel_name);
	if (!channelp)
		addChannel(config->debugchannel_name, 0, time(NULL), NULL, 0);

	nicklistp = addChannelNick(channelp, myuserp);
	chanlistp = addNickChannel(myuserp, channelp);

	nicklistp->flags |= IS_CHANOP;
	chanlistp->flags |= IS_CHANOP;

	ssprintf(s, "%s J %s\n", 
		myuserp->numeric, config->operchannel_name);
	ssprintf(s, "%s M %s +som %s\n", myserverp->numeric,
		config->operchannel_name, myuserp->numeric); 
	// Join Debug channel and set modes.

	ssprintf(s, "%s J %s\n", 
		myuserp->numeric, config->debugchannel_name);

	ssprintf (s,
		"%s M %s +o %s\n",
		myserverp->numeric, config->debugchannel_name,
		myuserp->numeric);

	ssprintf(s, "%s M %s %s\n", myserverp->numeric,
		config->debugchannel_name, config->debugchannel_modes);

	ircsp_free(numeric);
}
