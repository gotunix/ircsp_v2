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
// $Revision: 1.132 $
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
#include "channels.h"
#include "servers.h"
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
#include "version.h"


//------------------------------------------------------------------------
// UWorld Command Structure
//
//  Command Name
//  Command Function
//  Default Level
//  Min Arguments
//  Disabled
//------------------------------------------------------------------------

struct uworld_comtab UWorldCommand[] = {
	// Level 0000 Commands (User)

	{	"version",		&uw_version,		0,		3,	0,	NULL	},
	{	"help",			&uw_help,		0,		3,	0,	NULL	},
	{	"operlist",		&uw_operlist,		0,		3,	0,	NULL	},
	{	"verify",		&uw_verify,		0,		4,	0,	NULL	},

	// Level 0001 Commands (Oper)

	{	"uptime",		&uw_uptime,		1,		3,	0,	NULL	},
	{	"servlist",		&uw_servlist,		1,		3,	0,	NULL	},
	{	"nicklist",		&uw_nicklist,		1,		3,	0,	NULL	},
	{	"hostlist",		&uw_hostlist,		1,		3,	0,	NULL	},
	{	"adminlist",		&uw_adminlist,		1,		3,	0,	NULL	},
	{	"glinelist",		&uw_glinelist,		1,		3,	0,	NULL	},
	{	"chanlist",		&uw_chanlist,		1,		3,	0,	NULL	},
	{	"auth",			&uw_auth,		1,		5,	0,	NULL	},
	{	"deauth",		&uw_deauth,		1,		3,	0,	NULL	},
	{	"gline",		&uw_gline,		1,		5,	0,	NULL	},
	{	"remgline",		&uw_remgline,		1,		4,	0,	NULL	},
	{	"whoison",		&uw_whoison,		1,		4,	0,	NULL	},
	{	"banlist",		&uw_banlist,		1,		4,	0,	NULL	},
	{	"xlate",		&uw_xlate,		1,		4,	0,	NULL	},
	{	"whois",		&uw_whois,		1,		4,	0,	NULL	},

	// Level 0100 Comamnds

	{	"clearbans",		&uw_clearbans,		100,		4,	0,	NULL	},
	{	"opermsg",		&uw_opermsg,		100,		4,	0,	NULL	},
	{	"clearops",		&uw_clearops,		100,		4,	0,	NULL	},
	{	"opcom",		&uw_opcom,		100,		6,	0,	NULL	},
	{	"clearmodes",		&uw_clearmodes,		100,		4,	0,	NULL	},
	{	"scan",			&uw_scan,		100,		4,	0,	NULL	},
	{	"mode",			&uw_mode,		100,		5,	0,	NULL	},

	// Level 0900 Commands

	{	"masskill",		&uw_masskill,		900,		5,	0,	NULL	},

	// Level 1000 Commands

	{	"save",			&uw_save,		1000,		3,	0,	NULL	},
	{	"die",			&uw_die,		1000,		3,	0,	NULL	},
	{	"cmdlist",		&uw_cmdlist,		1000,		3,	0,	NULL	},
	{	"disablecmd",		&uw_disablecmd,		1000,		4,	0,	NULL	},
	{	"enablecmd",		&uw_enablecmd,		1000,		4,	0,	NULL	},
	{	"restart",		&uw_restart,		1000,		4,	0,	NULL	},
	{	"rehash",		&uw_rehash,		1000,		3,	0,	NULL	},
	{	"chlevel",		&uw_chlevel,		1000,		5,	0,	NULL	},
};






//------------------------------------------------------------------------
// uw_loadCommand ()
//
//  This function will load UWorlds Command Database which will 
//  consist of, the command name, the command level, and
//  weather or not the command is disabled allowing admins
//  to change access of commands without having to recompile.
//------------------------------------------------------------------------

int uw_loadCommand (void)
{
	FILE *f;
	int c, level, disabled;
	char read[512], *command;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  Loading UWorld command database\n",
		__FILE__, __LINE__, __FUNCTION__);

	f = fopen (config->uw_cmddb, "r");
	if (!f)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to load UWorld command database\n",
			__FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	else
	{
		while (!feof(f))
		{
			if (fgets(read, 255, f))
			{
				if (isComment(read))
				{
					ircsp_debug (
						"@(#) - (%s:%d) %s():  Ignoring Comments\n",
						__FILE__, __LINE__, __FUNCTION__);
					continue;
				}

				if (isBlank(read))
				{
					ircsp_debug (
						"@(#) - (%s:%d) %s():  Ignoring Empty Space\n",
						__FILE__, __LINE__, __FUNCTION__);
					continue;
				}

				command = strtok(read, ":");
				level = atoi(strtok(NULL, ":"));
				disabled = atoi(strtok(NULL, ":"));

				for (c = 0; c <  UW_NUM_CMDS; c++)
				{
					if (!strcasecmp(command, UWorldCommand[c].command))
					{
						if (disabled)
						{
							ircsp_debug (
								"@(#) - (%s:%d) %s():  UWorld Command [%s] DISABLED\n",
								__FILE__, __LINE__, __FUNCTION__,
								UWorldCommand[c].command);

							UWorldCommand[c].disabled = disabled;
						}

						if (level)
						{
							ircsp_debug (
								"@(#) - (%s:%d) %s():  UWorld Command [%s] Level [%d]\n",
								__FILE__, __LINE__, __FUNCTION__,
								UWorldCommand[c].command, UWorldCommand[c].minlevel);

							UWorldCommand[c].minlevel = level;
						}
					}
				}
			}
		}
	}

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld Command Database loaded\n",
		__FILE__, __LINE__, __FUNCTION__);
}


//------------------------------------------------------------------------
// uw_saveCommand ()
//
//  Save UWorlds command database to file.
//------------------------------------------------------------------------

int uw_saveCommand (void)
{
	User *myuserp;

	FILE *f;
	int cmd;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  Saving UWorld Command database\n",
		__FILE__, __LINE__, __FUNCTION__);

	myuserp = findUser (config->uworld_nickname);
	if (myuserp)
	{
		ircsp_notice (
			"@(#) - (%s:%d) %s():  Saving UWorld command database\n",
			__FILE__, __LINE__, __FUNCTION__);
	}

	if (!(f = fopen(config->uw_cmddb, "w")))
		return 0;

	fprintf(f, 
		"; UWorld Command Level DB [%s]\n", 
		config->uw_cmddb);
	fprintf(f, 
		"; Last Saved on: %lu\n", 
		time(NULL));

	for (cmd = 0; cmd < UW_NUM_CMDS; cmd++)
		fprintf(f, 
			"%s:%d:%d:\n", 
			UWorldCommand[cmd].command, 
			UWorldCommand[cmd].minlevel, 
			UWorldCommand[cmd].disabled);

	fclose(f);

	ircsp_debug (
		"@(#) - (%s:%d) %s():  Completed\n",
		__FILE__, __LINE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////
// GLINE Linked List
///////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------
// addGline ()
//
//------------------------------------------------------------------------


Gline *addGline (char *mask, time_t setwhen, time_t expires, char *reason, char *setby)
{
	Gline *glinep;

	ircsp_debug ("@(#) - (%s:%d) %s():  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, mask);

	glinep = (Gline *)ircsp_calloc(1, sizeof(Gline));
	if (!glinep)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to allocate memory for glineHEAD\n",
			__FILE__, __LINE__, __FUNCTION__);
		return NULL;
	}
	else
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Memory allocated for glineHEAD\n",
			__FILE__, __LINE__, __FUNCTION__);
	}

	glinep->mask = (char *)ircsp_calloc(1, strlen(mask) + 5);
	if (!glinep->mask)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to allocate memory for MASK\n",
			__FILE__, __LINE__, __FUNCTION__);
		return NULL;
	}
	else
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Memory allocated for MASK at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, glinep->mask);
	}

	glinep->reason = (char *)ircsp_calloc(1, strlen(reason) + 5);
	if (!glinep->reason)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to allocate memory for REASON\n",
			__FILE__, __LINE__, __FUNCTION__);
		return NULL;
	}
	else
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Memory allocated for REASON at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, glinep->reason);
	}

	glinep->setby = (char *)ircsp_calloc(1, strlen(setby) + 5);
	if (!glinep->setby)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to allocate memory for SETBY\n",
			__FILE__, __LINE__, __FUNCTION__);
		return NULL;
	}
	else
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Memory allocated for SETBY at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, glinep->setby);
	}


	strncpy(glinep->mask, mask, strlen(mask) + 5);
	strncpy(glinep->reason, reason, strlen(reason) + 5);
	strncpy(glinep->setby, setby, strlen(setby) + 5);

	glinep->setwhen = setwhen;
	glinep->expires = expires;


	ircsp_debug ("@(#) - (%s:%d) %s():  Adding GLine [%s] (%s))\n",
		__FILE__, __LINE__, __FUNCTION__, glinep->mask, glinep->reason);

	if (!glineHEAD)
	{
		glineHEAD = glinep;
		glinep->next = NULL;
	}
	else
	{
		glinep->next = glineHEAD;
		glineHEAD = glinep;
	}

	ircsp_debug ("@(#) - (%s:%d) %s():  Finished running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, mask);

	status->numGlines++;

	return glinep;
}


//------------------------------------------------------------------------
// findGline ()
//
//------------------------------------------------------------------------


Gline *findGline (char *mask)
{
	Gline *glinep;

	ircsp_debug ("@(#) - (%s:%d) %s():  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, mask);

	glinep = glineHEAD;
	while (glinep)
	{
		if (!strcasecmp(glinep->mask, mask))
		{
			ircsp_debug ("@(#) - (%s:%d) %s():  Gline [%s] Found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, glinep->mask,
				glinep->mask);

			return glinep;
		}
		glinep = glinep->next;
	}

	ircsp_debug ("@(#) - (%s:%d) %s():  Gline [%s] was not found\n",
		__FILE__, __LINE__, __FUNCTION__, mask);

	return glinep;
}


//------------------------------------------------------------------------
// delGline ()
//
//------------------------------------------------------------------------


void delGline (char *mask)
{
	Gline *glinep;
	Gline *prev;
	
	
	prev = NULL;
	
	ircsp_debug (
		"[%s:%d:%s()]:  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, mask);

	for (glinep = glineHEAD; glinep; glinep = glinep->next)
	{
		if (!strcasecmp(glinep->mask, mask))
		{
			ircsp_debug (
				"[%s:%d:%s()]:  Deleting GLINE [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, mask, glinep->mask);
				
			if (!prev)
			{
				glineHEAD = glinep->next;
			}
			else
			{
				prev->next = glinep->next;
			}
			
			ircsp_free (glinep->mask);
			ircsp_free (glinep->reason);
			ircsp_free (glinep->setby);
			ircsp_free (glinep);
		}
		prev = glinep;
	}


/*	Gline *glinep, *next, *delete, *check;

	ircsp_debug ("@(#) - (%s:%d) %s():  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, mask);

	for (glinep = glineHEAD; glinep; glinep = glinep->next)
	{
		next = glinep->next;
		if (!strcasecmp(glinep->mask, mask))
		{
			ircsp_debug ("@(#) - (%s:%d) %s():  Gline [%s] found, Deleting.\n",
				__FILE__, __LINE__, __FUNCTION__, mask);

			if (glineHEAD == glinep)
			{
				glineHEAD = glinep->next;
			}
			else
			{
				for (delete = glinep; (delete->next != glinep) && delete; delete = delete->next);
				delete->next = glinep->next;
			}

			ircsp_free (glinep->mask);
			ircsp_free (glinep->reason);
			ircsp_free (glinep->setby);

			ircsp_free (glinep);

			check = findGline (mask);
			if (!check)
			{
				ircsp_debug ("@(#) - (%s:%d) %s():  Gline [%s] Successfully deleted.\n",
					__FILE__, __LINE__, __FUNCTION__, mask);

				status->numGlines--;
			}
			else
			{
				ircsp_debug ("@(#) - (%s:%d) %s():  Gline [%s] Was not deleted\n",
					__FILE__, __LINE__, __FUNCTION__, mask);
			}

			return ;
		}

		ircsp_debug ("@(#) - (%s:%d) %s():  Gline [%s] was not found\n",
			__FILE__, __LINE__, __FUNCTION__, mask);
	} */
}


///////////////////////////////////////////////////////////////////////////
// END Gline Linked List Code
///////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////
// GLINE Database Code
///////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------
// uw_loadGlineDB ()
//
//------------------------------------------------------------------------


int uw_loadGlineDB (void)
{
	Gline *glinep;

	FILE *f;
	char inBuf[256], *argv[9];


	f = fopen(config->gline_db, "r");
	if (!f)
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to load GLINE DB\n",
			__FILE__, __LINE__, __FUNCTION__);
		ircsp_panic ("Failed to load GLINE DB");
	}
	else
	{
		while (!feof(f))
		{
			if (fgets(inBuf, 256, f))
			{
				if ((inBuf[0] == ';') || (inBuf[0] <= 32))
					continue;

				argv[0] = strtok(inBuf, ":"); // Mask
				argv[1] = strtok(NULL, ":"); // Set When
				argv[2] = strtok(NULL, ":"); // Expires
				argv[3] = strtok(NULL, ":"); // Set by
				argv[4] = strtok(NULL, "\n"); // Reason

				glinep = addGline(argv[0], atol(argv[1]), atol(argv[2]), argv[4], argv[3]);
			}
		}
	}
	fclose(f);
	return 1;
}


//------------------------------------------------------------------------
// uw_saveGlineDB ()
//
//------------------------------------------------------------------------


int uw_saveGlineDB (void)
{
	FILE *f;
	Gline *glinep;

	ircsp_debug ("@(#) - (%s:%d) %s():  Saving GLine Database\n",
		__FILE__, __LINE__, __FUNCTION__);

	if (!(f = fopen(config->gline_db, "w")))
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to write Gline DB\n",
			__FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	fprintf(f, "; GLINE DB file\n");
	fprintf(f, "; Last Saved: %lu\n", time(NULL));

	for (glinep = glineHEAD; glinep; glinep = glinep->next)
	{
		fprintf(f, "%s:%lu:%lu:%s:%s\n",
			glinep->mask, glinep->setwhen, glinep->expires,
			glinep->setby, glinep->reason);
	}

	ircsp_debug ("@(#) - (%s:%d) %s():  GLine Database Saved!\n",
		__FILE__, __LINE__, __FUNCTION__);

	fclose(f);
	return 1;
}


///////////////////////////////////////////////////////////////////////////
//  End GLINE Database Code
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
// NickJupe Linked list and Database Code
///////////////////////////////////////////////////////////////////////////



//------------------------------------------------------------------------
// addNickJupe ()
//
//------------------------------------------------------------------------


NickJupe *addNickJupe (char *nick, char *reason, int flags)
{
	NickJupe *njupep;
	
	ircsp_debug (
		"[%s:%d:%s()]:  Runnin on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, nick);
		
	njupep = (NickJupe *)ircsp_calloc(1, sizeof(NickJupe));
	if (!njupep)
	{
		ircsp_debug (
			"[%s:%d:%s()]:  Failed to allocate memory for nickjupeHEAD\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		ircsp_panic ("Failed to allocate memory");
	}
	else
	{
		ircsp_debug (
			"[%s:%d:%s()]:  Memory allocated for nickjupeHEAD at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, njupep);
	}
	
	
	njupep->nickname = (char *)ircsp_calloc(1, strlen(nick) + 5);
	if (!njupep->nickname)
	{
		ircsp_debug (
			"[%s:%d:%s()]:  Failed to allocate memory for NICKNAME\n",
			__FILE__, __LINE__, __FUNCTION__);
		
		ircsp_panic ("Failed to allocate memory");
	}
	else
	{
		ircsp_debug (
			"[%s:%d:%s()]:  Memory allocated for NICKNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, njupep->nickname);
	}
	
	
	njupep->reason = (char *)ircsp_calloc(1, strlen(reason) + 5);
	if (!njupep->reason)
	{
		ircsp_debug (
			"[%s:%d:%s()]:  Failed to allocate memory for REASON\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		ircsp_panic ("Failed to allocate memory");
	}
	else
	{
		ircsp_debug (
			"[%s:%d:%s()]:  Memory allocated for REASON at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, njupep->reason);
	}
	
	
	ircsp_debug (
		"[%s:%d:%s()]:  Nick Jupe [%s] added\n",
		__FILE__, __LINE__, __FUNCTION__, nick);
	
	
	strncpy (njupep->nickname, nick, strlen(nick) + 5);
	strncpy (njupep->reason, reason, strlen(reason) + 5);
	njupep->flags = flags;
	
	
	if (!nickjupeHEAD)
	{
		nickjupeHEAD = njupep;
		njupep->next = NULL;
	}
	else
	{
		njupep->next = nickjupeHEAD;
		nickjupeHEAD = njupep;
	}
	
	return njupep;
}


//------------------------------------------------------------------------
// findNickJupe ()
//
//------------------------------------------------------------------------


NickJupe *findNickJupe (char *nick)
{
	NickJupe *njupep;
	
	ircsp_debug (
		"[%s:%d:%s(): Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, nick);
		
	njupep = nickjupeHEAD;
	while (njupep)
	{
		if (!strcasecmp(njupep->nickname, nick))
		{
			ircsp_debug (
				"[%s:%d:%s()]:  Nick Jupe [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, njupep->nickname, 
				njupep->nickname);
				
			return njupep;
		}
		njupep = njupep->next;
	}
	
	
	ircsp_debug (
		"[%s:%d:%s()]:  Nick Jupe not found\n",
		__FILE__, __LINE__,  __FUNCTION__);
		
	return NULL;
}


//------------------------------------------------------------------------
// delNickJupe ()
//
//------------------------------------------------------------------------


void delNickJupe (char *nick)
{
	NickJupe *njupep;
	NickJupe *prev;	
	
	prev = NULL;
	
	ircsp_debug (
		"[%s:%d:%s()]:  Running on [%s]\n",
		__FILE__, __LINE__, __FUNCTION__, nick);

	for (njupep = nickjupeHEAD; njupep; njupep = njupep->next)
	{
		if (!strcasecmp(njupep->nickname, nick))
		{
			ircsp_debug (
				"[%s:%d:%s()]:  Deleting NJUPE [%s] found at [%p]\n",
				__FILE__, __LINE__, __FUNCTION__, nick, njupep->nickname);
				
			if (!prev)
			{
				nickjupeHEAD = njupep->next;
			}
			else
			{
				prev->next = njupep->next;
			}
			
			njupep->flags = 0;
			ircsp_free (njupep->nickname);
			ircsp_free (njupep->reason);
			ircsp_free (njupep);
		}
		prev = njupep;
	}
}
		

//------------------------------------------------------------------------
// uw_loadNickJupe ()
//
//------------------------------------------------------------------------


int uw_loadNickJupe (void)
{
	FILE *f;
	char read[256], *argv[4];
	
	f = fopen(config->jupe_database, "r");
	if (!f)
	{
		ircsp_debug (
			"[%s:%d:%s()]:  Unable to load NICK JUPE databas\n",
			__FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	else
	{
		while (!feof(f))
		{
			if (fgets(read, 255, f))
			{
				if (isComment (read))
				{
					ircsp_debug (
						"[%s:%d:%s()]:  Ignoring comments\n",
						__FILE__, __LINE__, __FUNCTION__);
						
					continue;
				}
				
				if (isBlank (read))
				{
					ircsp_debug (
						"[%s:%d:%s()]:  Ignoring empty space\n",
						__FILE__, __LINE__, __FUNCTION__);
						
					continue;
				}
				
				argv[0] = strtok(read, ":");
				argv[1] = strtok(NULL, ":");
				argv[2] = strtok(NULL, ":");
				
				addNickJupe(argv[0], argv[1], atoi(argv[2]));
			}
		}
	}
	fclose(f);
	return 1;
}


//------------------------------------------------------------------------
// uw_saveNickJupe ()
//
//------------------------------------------------------------------------


int uw_saveNickJupe (void)
{
	FILE *f;
	NickJupe *njupep;

	ircsp_debug ("@(#) - (%s:%d) %s():  Saving NJUPE Database\n",
		__FILE__, __LINE__, __FUNCTION__);

	if (!(f = fopen(config->jupe_database, "w")))
	{
		ircsp_debug ("@(#) - (%s:%d) %s():  Failed to write NJUPE DB\n",
			__FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	fprintf(f, "; NJUPE DB file\n");
	fprintf(f, "; Last Saved: %lu\n", time(NULL));

	for (njupep = nickjupeHEAD; njupep; njupep = njupep->next)
	{
		fprintf(f, "%s:%s:%d:\n",
			njupep->nickname, njupep->reason, njupep->flags);
	}

	ircsp_debug ("@(#) - (%s:%d) %s():  NJUPE Database Saved!\n",
		__FILE__, __LINE__, __FUNCTION__);

	fclose(f);
	return 1;
}

				 
///////////////////////////////////////////////////////////////////////////
// End Nick Jupe Code
///////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------
// do_uworld ()
//
//  UWorld command parser.
//------------------------------------------------------------------------

void do_uworld (char *sBuf)
{
	User *myuserp;
	User *userp;

	char *parse[5], *argv[256], *temp;
	int c, argc = 1, cmd = 0, level = 0;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  Starting UWorld command parser\n",
		__FILE__, __LINE__, __FUNCTION__);


	parse[0] = strtok(sBuf, " "); // nickname
	strtok(NULL, " "); // skip
	parse[1] = strtok(NULL, " "); // Dest nickname
	temp = strtok(NULL, "\r"); // Command and args

	parse[2] = strtok(temp, " "); // Command
	parse[2]++;
	parse[3] = strtok(NULL, "\r"); // Args


	myuserp= findUser(config->uworld_nickname);
	if (!myuserp)
	{
		// UWorld nickname was not found - 
		// This should never happen, but just in case
		// we have a memory leak, lets error insted
		// of causing a SEGV.

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Couldn't find my own nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);
		ircsp_warning (
			"@(#) - (%s:%d) %s():  Couldn't find my own nickname [%s]!\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return ;
	}

	userp = findUserByNumeric(parse[0]);
	if (!userp)
	{
		// Sender nickname was not found -
		// This should never happen, but just in case
		// it does, lets throw up an error
		// insted of causing a SEGV/Panic.

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Couldn't find sender nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, parse[0]);
		ircsp_warning (
			"@(#) - (%s:%d) %s():  Couldn't find sender nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, parse[0]);
		return ;
	}

	for (c = 0; c <= UW_NUM_CMDS; c++)
	{
		if (!strcasecmp(parse[2], UWorldCommand[c].command))
		{
			// Command was found.

			cmd = 1;
			argv[0] = parse[0]; // Nickname
			argv[1] = parse[1]; // Dest Nickname
			argv[2] = parse[2]; // Command
			argc = 3;

			if (parse[3])
			{
				argv[3] = strtok(parse[3], " ");

				while (argv[argc])
				{
					argv[++argc] = strtok(NULL, " ");
				}
			}

			if (userp->adminp)
				level = userp->adminp->level;
			else if (IsOperator(userp))
				level = 1;
			else
				level = 0;

			
			// make sure that the sender has enough access
			// to issue the command.

			if (level >= UWorldCommand[c].minlevel)
			{
				// Make sure that the command is not disabled.

				if (UWorldCommand[c].disabled)
				{
					ssprintf(s, 
						"%s O %s :Command is \002DISABLED\002.\n",
						myuserp->numeric, parse[0]);
					return ;
				}
	
				// Make sure that the syntax is correct for the 
				// issued command.

				if (argc < UWorldCommand[c].minargs)
				{
					ssprintf(s, 
						"%s O %s :Incorrect Syntex. try /msg %s HELP %s\n",
						myuserp->numeric, parse[0], config->uworld_nickname,
						UWorldCommand[c].command);
					return ;
				}

				// Push out the command

				if (UWorldCommand[c].func(argc, argv))
				{
					// Eventually we will add command logging here.
					// for now we do nothing.
				}
			}

			// User did not meet the level requirements to issue the command.

			else
			{
				ircsp_notice (
					"@(#) - (%s:%d) %s():  Denied access to %s from %s\n",
					__FILE__, __LINE__, __FUNCTION__, UWorldCommand[c].command,
					userp->nickname);
					
				ssprintf(s, 
					"%s O %s :\002%s\002\n",
					myuserp->numeric, parse[0], config->deny_message);
				return ;
			}
		}
	}

	// Command does not exist.

	if (parse[2][0] == '\001')
	{
		parse[2]++;
		if (!strncasecmp(parse[2], "VERSION", 7))
		{
			ssprintf (s, 
				"%s O %s :\001VERSION IRCSP %s - Copyright (C) 1998-2009 GOTUNIX Networks. \001\n",
				myuserp->numeric, parse[0], 
				version_number);
			return ;
		}
	}

	if (!cmd)
	{
		ssprintf(s, 
			"%s O %s :Unknown Command.\n",
			myuserp->numeric, parse[0]);
		return ;
	}
	ircsp_debug (
		"@(#) - (%s:%d) %s():  Ending UWorld command parser.\n",
		__FILE__, __LINE__, __FUNCTION__);
}



///////////////////////////////////////////////////////////////////////////
//  UWorld Level 0000 Commands (USER)
///////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------
// uw_version ()
//
//  This function will return the current service and  ircsp 
//  version information back to the sender.
//
// argv[0] = Sender Numeric
//------------------------------------------------------------------------

int uw_version (int argc, char *argv[])
{
	User *userp;
	User *myuserp;
	int i;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command VERSION\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		// Sender nickname was not found in our linked list -
		// This really should never happen unless something
		// went very wrong.

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser(config->uworld_nickname);
	if (!myuserp)
	{
		// UWorld nickname was not found in our linked list -
		// This shouldn't happen unless something is really
		// wrong.  for now we will send out an error
		// and keep running.
		//
		//
		// In the future we could run a 'ircsp_restart' to
		// try have IRCSP fix itself.

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);
		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002VERSION\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s, 
		"%s O %s :---------------------------------------------------\n",
		myuserp->numeric, userp->numeric);
		
	for (i = 0; info_text[i]; i++)
	{
		ssprintf(s,
			"%s O %s :%s\n", 
			myuserp->numeric, userp->numeric, info_text[i]);
	}

	ssprintf(s, 
		"%s O %s :---------------------------------------------------\n",
		myuserp->numeric, userp->numeric);

	ssprintf(s,
		"%s O %s :IRCSP Version Information:\n",
		myuserp->numeric, userp->numeric);
		
	ssprintf(s,
		"%s O %s :IRCSP Codename: %s\n",
		myuserp->numeric, userp->numeric, code_name);
		
	ssprintf(s,
		"%s O %s :IRCSP Version: %s\n",
		myuserp->numeric, userp->numeric, version_number);
		
	ssprintf(s, 
		"%s O %s :IRCSP BUILD: %s\n",
		myuserp->numeric, userp->numeric, version_build);
		
	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command VERSION\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


//------------------------------------------------------------------------
// uw_help ()
//
//  This command will display the help file for UWorld as well as any
//  help topic.
//
// argv[0] = Sender Numeric
//------------------------------------------------------------------------

int uw_help (int argc, char *argv[])
{
	User *userp;
	User *myuserp;
	FILE *fp;


	char buf[256], newbuf[512], helpfile[128];
	int level, show = 1;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command HELP\n",
		__FILE__, __LINE__, __FUNCTION__);

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		// UWorld nickname was not found in our linked list -
		// THis shouldn't happen, but it did so.. report it

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}
	
	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}


	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002HELP\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	if (argc == 3)
	{
		sprintf(helpfile, "helpfiles/uworld/index");
	}
	else
	{
		sprintf(helpfile, "helpfiles/uworld/%s", argv[3]);
	}

	fp = fopen(helpfile, "r");
	if (!fp)
	{
		ssprintf(s, 
			"%s O %s :No help available on \002%s\002.\n",
			myuserp->numeric, argv[0], argv[3]);
		return 0;
	}

	while (!feof(fp))
	{
		if (fgets(buf, 255, fp))
		{
			if (buf[0] == '@')
			{
				buf[0] = '0';
				level = atoi(buf);


				if ((IsAdministrator(userp)) && (userp->adminp->level >= level))
				{
					ssprintf(s, 
						"%s O %s :Level [\002%d\002]\n",
						myuserp->numeric, argv[0], level);

					show = 1;
				}
				else if ( (IsOperator(userp)) && (level <= 1) )
				{
					ssprintf(s, 
						"%s O %s :Level [\002%d\002]\n",
						myuserp->numeric, argv[0], level);

					show = 1;
				}
				else if ((!IsOperator(userp)) && (level ==0))
				{
					ssprintf(s, 
						"%s O %s :Level [\002%d\002]\n",
						myuserp->numeric, argv[0], level);

					show = 1;
				}

				else
				{
					show = 0;
				}
				continue;
			}
			if (show)
				ssprintf(s, 
					"%s O %s :%s\n",
					myuserp->numeric, argv[0], translate(buf, newbuf));
		}
	}

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command HELP\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


//------------------------------------------------------------------------
// uw_operlist ()
//
//  This command will list all GLOBAL opers on the network
//
// argv[0] = Sender Numeric
//------------------------------------------------------------------------

int uw_operlist (int argc, char *argv[])
{
	User *myuserp;
	User *userp;

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		// Failed to find SENDER nickname in our linked list

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		// Failed to find UWORLD nickname in our linked list

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002OPERLIST\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	for (userp = userHEAD; userp; userp = userp->next)
	{
		if ((IsOperator(userp)) && (!IsService(userp)))
			ssprintf(s, 
				"%s O %s :Nickname [%s] is an \002OPER\002\n",
				myuserp->numeric, argv[0], userp->nickname);
	}

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command OPERLIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


//------------------------------------------------------------------------
// uw_verify ()
//
//  This function will allow users to verify that someone is infact
//  a UWorld administrator.
//
// argv[0] = sender nickname
// argv[3] = nickname to verify
//------------------------------------------------------------------------

int uw_verify (int argc, char *argv[])
{
	User *myuserp;	// UWorld Nickname
	User *userp;	// Sender Nickname
	User *vuserp;	// Nickname to Verify

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002VERIFY\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	vuserp = findUser (argv[3]);
	if (!vuserp)
	{
		ssprintf(s, 
			"%s O %s :[%s] does not exist on the network\n",
			myuserp->numeric, userp->numeric, argv[3]);
		return 0;
	}
	else
	{
		if (IsAdministrator(vuserp))
		{
			ssprintf(s, 
				"%s O %s :[%s] is a authorized Level[\002%d\002] Administrator on %s\n",
				myuserp->numeric, userp->numeric, vuserp->nickname, vuserp->adminp->level, 
				config->uworld_nickname);

			return 1;
		}
		else
		{
			ssprintf(s,
				"%s O %s :[%s] is not an authorized Administrator\n",
				myuserp->numeric, userp->numeric, argv[3]);
			return 0;
		}
	}
}



///////////////////////////////////////////////////////////////////////////
// UWorld Level 0001 Commands
///////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------
// uw_servlist ()
//
//   This function will return a list of servers that are currently
//   on the network, as well as their uplink information back to
//   the sender.
//
// argv[0] = Sender Numeric
//------------------------------------------------------------------------

int uw_servlist (int argc, char *argv[])
{
	User *userp;
	User *myuserp;
	Server *serverp;
	int numServers = 0;

	ircsp_debug(
		"@(#) - (%s:%d) %s():  UWorld command SERVLIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		// SENDER nickname was not found in linked list -
		// This should never happen but just in case it does.
		// Issue warning, possibly restart IRCSP to correct 
		// itself.

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		// UWORLD nickname was not found in linked list -
		// This should never happen but just incase, issue a warning, 
		// and possibly restart IRCSP.

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002SERVLIST\002 issued by %s\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s, 
		"%s O %s :Global \002SERVER\002 list\n",
		myuserp->numeric, argv[0]);

	for (serverp = serverHEAD; serverp; serverp = serverp->next)
	{

		ssprintf(s, 
			"%s O %s :Server [\002%s\002] Numeric: [\002%s\002]\n",
			myuserp->numeric, argv[0], serverp->name, serverp->numeric);

		if (serverp->flags & SERVER_IS_MY_UPLINK)
		{
			// Server is IRCSP's uplink.

			ssprintf(s, 
				"%s O %s :|-  IRCSP UPLINK\n",
				myuserp->numeric, argv[0]);
		}
		else
		{
			Server *userverp;

			userverp = findServerByNumeric(serverp->uplink);
			if (userverp)
				ssprintf(s, 
					"%s O %s :|-  Linked to [\002%s\002] Numeric: [\002%s\002]\n",
					myuserp->numeric, argv[0], userverp->name, 
					userverp->numeric);
		}

		ssprintf(s, 
			"%s O %s :|-  Current Users [\002%d\002]\n", 
			myuserp->numeric, argv[0], serverp->numUsers);

		ssprintf(s, 
			"%s O %s :|-  Current Operators [\002%d\002]\n", 
			myuserp->numeric, argv[0], serverp->numOpers);


		numServers++;
	}

	ssprintf(s, 
		"%s O %s :[\002%d\002] Servers listed.\n",
		myuserp->numeric, argv[0], numServers);


	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command SERVLIST\n",
		__FILE__, __LINE__, __FUNCTION__);
	return 0;
}


//------------------------------------------------------------------------
// uw_nicklist ()
//
//  This function will display a list of all nicknames currently online
//  as well as their status (is oper, is service, is admin, etc)
//
// argv[0] = Sender numeric
//------------------------------------------------------------------------

int uw_nicklist (int argc, char *argv[])
{
	User *myuserp;
	User *userp;
	Server *serverp;
	int numNicks = 0, numOpers = 0, numService = 0, numJupes = 0;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command NICKLIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		// SENDER nickname was not found in linked list -
		// This should never happen, but just in case it does
		// lets throw up a warning and possibly restart 
		// IRCSP

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		// UWORLD nickname was not found in linked list -
		// This should never happen, but just in case it does,
		// lets throw up a warning and possibly restart IRCSP.

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002NICKLIST\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);


	ssprintf(s, 
		"%s O %s :Global \002NICK\002 list\n",
		myuserp->numeric, argv[0]);

	for (userp = userHEAD; userp; userp = userp->next)
	{
		serverp = findServerByNumeric (userp->server);
		if (!serverp)
		{
			ircsp_debug (
				"@(#) - (%s:%d) %s(): Failed to find server [%s]\n",
				userp->server);
			return 0;
		}

		if (IsService(userp))
		{
			ssprintf(s, 
				"%s O %s :Nickname [\002%s\002] (%s@%s) on %s (\002SERVICE BOT\002)\n",
				myuserp->numeric, argv[0], userp->nickname, userp->username,
				userp->hostp->hostname, serverp->name);

			numService++;
		}
			

		else if ((IsOperator(userp)) && (!IsService(userp)))
		{
			ssprintf(s, 
				"%s O %s :Nickname [\002%s\002] (%s@%s) on %s (\002OPER\002)\n",
				myuserp->numeric, argv[0], userp->nickname, userp->username,
				userp->hostp->hostname, serverp->name);
			numOpers++;
		}
		else if (IsNickJupe(userp))
		{
			ssprintf(s, 
				"%s O %s :Nickname [\002%s\002] (%s@%s) on %s (\002JUPE\002)\n",
				myuserp->numeric, argv[0], userp->nickname, userp->username, 
				userp->hostp->hostname, serverp->name);
			numJupes++;
		}

		else if ((!IsOperator(userp)) && (!IsService(userp)))
		{
			ssprintf(s, 
				"%s O %s :Nickname [\002%s\002] (%s@%s) on %s\n",
				myuserp->numeric, argv[0], userp->nickname, userp->username,
				userp->hostp->hostname, serverp->name);
		}
		numNicks++;
	}

	ssprintf(s, 
		"%s O %s :[\002%d\002] Nicknames [\002%d\002] Operators  and [\002%d\002] Service Bots and [\002%d\002] Nick Jupes listed\n",
		myuserp->numeric, argv[0], numNicks, numOpers, numService, numJupes);

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command NICKLIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


//------------------------------------------------------------------------
// uw_uptime ()
//
//  Display the current uptime of IRCSP.
//
// argv[0] = Sender Numeric
//------------------------------------------------------------------------

int uw_uptime (int argc, char *argv[])
{
	User *userp;
	User *myuserp;

	int days, hours, mins, secs;
	time_t curtime, uptime;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command UPTIME\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		// Failed to find SENDER nickname in linked list -
		// This shouldn't happen but just in case ...

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}


	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	ircsp_notice ( 
		"@(#) - (%s:%d) %s():  UWorld command \002UPTIME\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	curtime = time(NULL);
	uptime = curtime - service->uptime;

	days = uptime/86400;
	hours = (uptime/3600)%24;
	mins = (uptime/60)%60;
	secs = uptime%60;

	ssprintf(s, 
		"%s O %s :\002%s\002 has been up for \002%d\002 days \002%d\002 hours \002%d\002 minutes \002%d\002 seconds\n",
		myuserp->numeric, argv[0], config->server, days, hours, mins, 
		secs);

	ircsp_debug ("@(#) - (%s:%d) %s():  End UWorld command UPTIME\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


//------------------------------------------------------------------------
// uw_hostlist ()
//
//  Display all HOSTNAMES currently online, including a count of how
//  many clients connected on the host. (Clones)
//
// argv[0] = Sender Numeric
//------------------------------------------------------------------------



int uw_hostlist (int argc, char *argv[])
{
	User *userp;
	User *myuserp;
	Hostname *hostp;
	int numHosts = 0;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command HOSTLIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		// Failed to find SENDER nickname in linked list.
		// send error, possibly restart.

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}


	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		// Failed to find UWORLD nickname in linked list.
		// send error, possibly restart.

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002HOSTLIST\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);


	ssprintf(s, 
		"%s O %s :Global \002HOST\002 list\n",
		myuserp->numeric, argv[0]);

	for (hostp = hostHEAD; hostp; hostp = hostp->next)
	{
		ssprintf(s, 
			"%s O %s :Hostname [\002%s\002]  Host Count [\002%d\002]\n",
			myuserp->numeric, argv[0], hostp->hostname, hostp->numHosts);
		numHosts++;
	}

	ssprintf(s, 
		"%s O %s :[\002%d\002] Hostnames listed.\n",
		myuserp->numeric, argv[0], numHosts);

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command HOSTLIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


//------------------------------------------------------------------------
// uw_adminlist ()
//
//  This function will display a list of all administrators and their
//  status (logged in, logged in from, level, hostmask, etc)
//
// argv[0] = Sender Numeric
//------------------------------------------------------------------------


int uw_adminlist (int argc, char *argv[])
{
	User *userp;
	User *myuserp;
	Admin *adminp;
	int numAdmin = 0;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command ADMINLIST\n",
		__FILE__, __LINE__, __FUNCTION__);


	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		// Failed to find SENDER nickname in linked list.

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find my own nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002ADMINLIST\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s, 
		"%s O %s :IRCSP Administrator LIST\n",
		myuserp->numeric, argv[0]);

	for (adminp = adminHEAD; adminp; adminp = adminp->next)
	{
		ssprintf(s, 
			"%s O %s :[\002%s\002]\n",
			myuserp->numeric, argv[0], adminp->nickname);

		ssprintf(s, 
			"%s O %s :|- Level   :  %d\n",
			myuserp->numeric, argv[0], adminp->level);

		ssprintf(s, 
			"%s O %s :|- Hostmask:  %s\n",
			myuserp->numeric, argv[0], adminp->hostmask);

		if (adminp->userp)
			ssprintf(s, 
				"%s O %s :|- \002IN-USE\002 %s (%s@%s)\n",
				myuserp->numeric, argv[0], adminp->userp->nickname,
				adminp->userp->username, adminp->userp->hostp->hostname);

		if (IsAdminSuspended(adminp))
		{
			ssprintf(s, 
				"%s O %s :|- %s is Suspended!\n",
				myuserp->numeric, argv[0], adminp->nickname);
		}

		numAdmin++;
	}

	ssprintf(s, 
		"%s O %s :[\002%d\002] Admins listed\n",
		myuserp->numeric, argv[0], numAdmin);

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command ADMINLIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}

//------------------------------------------------------------------------
// uw_auth ()
//
//  Authorize yourself as a IRCSP administrator.
//
//  argv[0] = Sender Numeric
//  argv[3] = Account Nickname
//  argv[4] = Password
//------------------------------------------------------------------------

int uw_auth (int argc, char *argv[])
{
	User *myuserp;
	User *userp;
	Admin *adminp;
	char userhost[2096];
	int hostMatch = 0;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command AUTH\n",
		__FILE__, __LINE__, __FUNCTION__);


	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		// Failed to locate UWORLD nickname in linked list.

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s] \n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}


	userp = findUserByNumeric(argv[0]);
	if (!userp)
	{
		// Failed to locate SENDER nickname in linked list.

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	
	if (IsAdministrator(userp))
	{
		ssprintf(s, 
			"%s O %s :You are already authorized.\n",
			myuserp->numeric, argv[0]);
		return 0;
	}

	adminp = findAdmin (argv[3]);
	if (!adminp)
	{
		ssprintf(s, 
			"%s O %s :Authentication failed! [1]\n",
			myuserp->numeric, argv[0]);

		ircsp_notice (
			"@(#) - (%s:%d) %s():  \002AUTH\002 - %s!%s@%s FAILED [Invalid Username]\n",
			__FILE__, __LINE__, __FUNCTION__, userp->nickname, userp->username,
			userp->hostp->hostname);

		return 0;
	}


	if (IsAdminLogin(adminp))
	{
		ssprintf(s, 
			"%s O %s :Authentication failed! (Account already in use) [\002%s\002]\n",
			myuserp->numeric, argv[0], adminp->userp->nickname);

		ircsp_notice (
			"@(#) - (%s:%d) %s():  \002AUTH\002 - %s!%s@%s FAILED [Account in USE]\n",
			__FILE__, __LINE__, __FUNCTION__,
			userp->nickname, userp->username, userp->hostp->hostname);

		return 0;
	}

	sprintf(userhost, "%s@%s", userp->username, userp->hostp->hostname);

	if (!match(adminp->hostmask, userhost))
	{
		hostMatch = 1;
	}

	if (!hostMatch)
	{
		ssprintf(s, 
			"%s O %s :Authentication failed! [2]\n",
			myuserp->numeric, argv[0]);

		ircsp_notice (
			"@(#) - (%s:%d) %s():  \002AUTH\002 - %s!%s@%s FAILED [Host mismatch]\n",
			__FILE__, __LINE__, __FUNCTION__, userp->nickname, userp->username,
			userp->hostp->hostname);

		return 0;
	}

	if (!strcmp(adminp->password, argv[4]))
	{
		ssprintf(s, 
			"%s O %s :Authentication successful!\n",
			myuserp->numeric, argv[0]);

		if (IsAdminSuspended(adminp))
		{
			ssprintf(s, 
				"%s O %s :... But you are suspended!\n",
				myuserp->numeric, argv[0]);

			ircsp_notice (
				"@(#) - (%s:%d) %s():  \002AUTH\002 - %s!%s@%s FAILED [Suspended]\n",
				__FILE__, __LINE__, __FUNCTION__, userp->nickname, 
				userp->username, userp->hostp->hostname);

			return 0;
		}

		SetAdminLogin(adminp);
		SetAdministrator(userp);

		userp->adminp = adminp;
		adminp->userp = userp;

		ircsp_notice (
			"@(#) - (%s:%d) %s():  \002AUTH\002 - %s!%s@%s SUCCESS [Account: %s]\n",
			__FILE__, __LINE__, __FUNCTION__, userp->nickname,
			userp->username, userp->hostp->hostname,
			userp->adminp->nickname);

		return 1;
	}
	else
	{
		ssprintf(s, 
			"%s O %s :Authentication failed! [3]\n",
			myuserp->numeric, argv[0]);

		ircsp_notice (
			"@(#) - (%s:%d) %s():  \002AUTH\002 - %s!%s@%s FAILED [Invalid Password]\n",
			__FILE__, __LINE__, __FUNCTION__, userp->nickname,
			userp->username, userp->hostp->hostname);

		return 0;
	}

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command AUTH\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}

//------------------------------------------------------------------------
// uw_deauth ()
//
//  Deauthorize yourself as a IRCSP administrator.
// 
//  argv[0] = Sender Numeric
//------------------------------------------------------------------------


int uw_deauth (int argc, char *argv[])
{
	User *myuserp;
	User *userp;
	Admin *adminp;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command DEAUTH\n",
		__FILE__, __LINE__, __FUNCTION__);

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		// Failed to find UWORLD nickname in linked list

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	userp = findUserByNumeric(argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	if (IsAdministrator(userp))
	{
		adminp = findAdmin(userp->adminp->nickname);
		if (!adminp)
		{
			ssprintf(s, 
				"%s O %s :You're not logged in.\n",
				myuserp->numeric, argv[0]);

			return 0;
		}

		if (IsAdminLogin(adminp))
		{
			DelAdminLogin(adminp);
			DelAdministrator(userp);
			userp->adminp = NULL;
			adminp->userp = NULL;

			ssprintf(s, 
				"%s O %s :Deauthorization successfull!\n",
				myuserp->numeric, argv[0]);

			return 1;
		}
		else
		{
			ssprintf(s, 
				"%s O %s :You're not logged in.\n",
				myuserp->numeric, argv[0]);
			return 0;
		}
	}
	else
	{
		ssprintf(s, 
			"%s O %s :You're not logged in.\n",
			myuserp->numeric, argv[0]);
		return 0;
	}

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command DEAUTH\n",
		__FILE__, __LINE__, __FUNCTION__);
}


//------------------------------------------------------------------------
// uw_glinelist ()
//
//  Display a current GLOBAL gline list.
//
// argv[0] = Sender Numeric
//------------------------------------------------------------------------

int uw_glinelist (int argc, char *argv[])
{
	User *userp;
	User *myuserp;
	Gline *glinep;
	int numGlines = 0;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command GLINELIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	ssprintf(s, 
		"%s O %s :Global \002GLINE\002 list\n",
		myuserp->numeric, argv[0]);

	for (glinep = glineHEAD; glinep; glinep = glinep->next)
	{
		ssprintf(s, 
			"%s O %s :Gline [%s]\n",
			myuserp->numeric, argv[0], glinep->mask);
		ssprintf(s, 
			"%s O %s :|- Added on  : [%s]\n",
			myuserp->numeric, argv[0], chop(ctime(&glinep->setwhen)));
		ssprintf(s, 
			"%s O %s :|- Expires on: [%s]\n",
			myuserp->numeric, argv[0], chop(ctime(&glinep->expires)));
		ssprintf(s, 
			"%s O %s :|- Set By    : [%s]\n",
			myuserp->numeric, argv[0], glinep->setby);
		ssprintf(s, 
			"%s O %s :|- Reason    : [%s]\n",
			myuserp->numeric, argv[0], glinep->reason);
		numGlines++;
	}

	ssprintf(s, 
		"%s O %s :[\002%d\002] GLINES listed.\n",
		myuserp->numeric, argv[0], numGlines);

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command GLINELIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 1;
}


//------------------------------------------------------------------------
// uw_chanlist ()
//
//  This function will display a list of all channels on the network
//
// argv[0] = Sender Numeric
//------------------------------------------------------------------------

int uw_chanlist (int argc, char *argv[])
{
	User *userp;
	User *myuserp;
	Channel *channelp;
	int numChannels = 0;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command CHANLIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		// Failed to locate SENDER nickname in our linked list.

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		// Failed to find UWORLD nickname in our linked list.

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002CHANLIST\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s, 
		"%s O %s :Global \002CHANNEL\002 list\n",
		myuserp->numeric, argv[0]);

	for (channelp = channelHEAD; channelp; channelp = channelp->next)
	{
		ssprintf(s, 
			"%s O %s :Channel [%s]\n",
			myuserp->numeric, argv[0], channelp->name);
		numChannels++;
	}

	ssprintf(s, 
		"%s O %s :[\002%d\002] CHANNELS listed.\n",
		myuserp->numeric, argv[0], numChannels);


	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command CHANLIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 1;
}


//------------------------------------------------------------------------
// uw_gline ()
//
//  This function will add a gline to our database, and globally to the
//  network.
//
// argv[0] = Sender Numeric
// argv[3] = Mask
// argv[4] = Duration
// argv[5] .. argv[parc] = reason
//------------------------------------------------------------------------


int uw_gline (int argc, char *argv[]) 
{
	User *myuserp;
	User *userp;
	Gline *glinep;
	int duration, curarg = 3, is_bad_gline = 0, numdots, curdot;
	int eduration;
	char reason[ 512 ], *maskp, *uidp, mask[ 256 ];


	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command GLINE\n",
		__FILE__, __LINE__, __FUNCTION__);


	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		// Failed to find SENDER nickname in linked list

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		// Failed to find UWORLD nickname in linked list

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}


	strncpy(mask, argv[3], 255);

	// First, we need to check to see if this G:Line is a *bit*
	// excessive.  The following are considered Bad:
	//
	// 1) Bans that are equivalent to *@*
	// 2) Bans of entire TLD's
	

	// First lets check for *@* 

	if (!match(argv[3], "*@*"))
	{
		is_bad_gline = 1;
	}

	// Next we'll check for TLD bans. EVERY character up to the
	// next to last token (by period) must be ? or * and the
	// last must not be (it should be caught by above anyway).


	if (!is_bad_gline)
	{
		uidp = strtok(mask, "@");
		if (!match(uidp, "*"))      /* userid = * or equivalent */
		{
			maskp = strtok(NULL, " ");
			numdots = tokenize(maskp, '.');

			if (!numdots)
			{
				is_bad_gline = 1;
			}
			else
			{
				is_bad_gline = 1;
				for(curdot = 1; curdot < numdots; curdot++)
					if(match(token(maskp, curdot), "*") != 0)
						is_bad_gline = 0;
			} 
		}
	}

	if (is_bad_gline)
	{
		ssprintf(s, 
			"%s O %s :Excessive GLINE - please narrow the mask.\n",
			myuserp->numeric, userp->numeric);
		return 0;
	}

	if (isdigit(argv[4][0]))
	{
		duration = atoi(argv[4]);
		if (argc < 6)
		{
			ssprintf(s, 
				"%s O %s :\002GLINE\002 requires a REASON.\n",
				myuserp->numeric, userp->numeric);
			return 0;
		}

		curarg = 5;
	}
	else
	{
		duration = config->gline_auto_len;
		curarg = 4;
	}

	eduration = duration * 8;

	glinep = glineHEAD;
	while (glinep)
	{
		// Do we already have an exact gline for this host?

		if (!strcasecmp(glinep->mask, argv[3]))
		{
			// Gline exists - check the expiration versus our new 
			// time -- if it's greater, update the expiration
			// else ignore it.

			if ((time(NULL) + duration) > glinep->expires)
			{
				glinep->expires += eduration;
				glinep->setwhen = time(NULL);

				ssprintf(s, 
					"%s O %s :\002GLINE\002 Expiration reset.\n",
					myuserp->numeric, userp->numeric);

				ssprintf(s, 
					"%s GL * +%s %lu %lu :%s\n",
					config->server_numeric, glinep->mask, 
					eduration, time(NULL), glinep->reason);

				ssprintf(s, 
					"%s WA :[%s] reset \002GLINE\002 Expiration on [%s]\n",
					config->server_numeric, userp->nickname, glinep->mask);
				return 1;
			}

			ssprintf(s, 
				"%s O %s :\002GLINE\002 for [\002%s\002] already exists.\n",
				myuserp->numeric, userp->numeric, glinep->mask);
			return 0;
		}

		else if (!match(glinep->mask, argv[3]))
		{
			ssprintf(s, 
				"%s O %s :\002GLINE\002 covering [\002%s\002] already exists [\002%\002]\n",
				myuserp->numeric, userp->numeric, argv[3], glinep->mask);
			return 1;
		}

		else if (!match(argv[3], glinep->mask))
		{
			ssprintf(s, 
				"%s O %s :Removing lower precedence \002GLINE\002 for [\002%s\002]\n",
				myuserp->numeric, userp->numeric, glinep->mask);

			ssprintf(s, 
				"%s GL * -%s %lu %lu\n",
				config->server_numeric, glinep->mask, (time(NULL) - 60), 
				time(NULL));
			delGline(glinep->mask);
		}
		glinep = glinep->next;
	}

	// Create our reason

	strcpy(reason, argv[curarg]);

	argc--;
	while (++curarg <= argc)
	{
		strcat(reason, " ");
		strcat(reason, argv[curarg]);
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002GLINE\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	glinep = addGline (argv[3], time(NULL), time(NULL) + duration, reason, userp->nickname);
	ssprintf(s, 
		"%s GL * +%s %lu %lu :(Expires: %s (%s))\n",
		config->server_numeric, argv[3], time(NULL) + eduration, 
		time(NULL), chop(ctime(&glinep->expires)), reason);

	ssprintf(s, 
		"%s WA :[\002%s\002] Adding \002GLINE\002 for %s (Expires: %s (%s))\n",
		config->server_numeric, userp->nickname, argv[3], 
		chop(ctime(&glinep->expires)), reason);

	ssprintf(s, 
		"%s O %s :Adding \002GLINE\002 for %s (Expires %s (%s))\n",
		myuserp->numeric, argv[0], glinep->mask, 
		chop(ctime(&glinep->expires)), reason);


	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command GLINE\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 1;
}


//------------------------------------------------------------------------
// uw_remgline ()
//
//  This function will remove a gline form our database and from the
//  network.
//
// argv[0] = Sender numeric
// argv[3] = Hostmask
//------------------------------------------------------------------------

int uw_remgline (int argc, char *argv[])
{
	User *myuserp;
	User *userp;
	Gline *glinep;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command REMGLINE\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		// Failed to find SENDER nickname in our linked list

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@()# - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		// Failed to find UWORLD nickname in our linked list

		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	glinep = findGline (argv[3]);
	if (!glinep)
	{
		ssprintf(s, 
			"%s O %s :No such \002GLINE\002\n",
			myuserp->numeric, userp->numeric);
		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002REMGLINE\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s, 
		"%s GL * -%s %lu %lu\n",
		config->server_numeric, glinep->mask, (time(NULL) - 172800), 
		time(NULL));

	ssprintf(s, 
		"%s O %s :\002GLINE\002 [%s] Removed.\n",
		myuserp->numeric, userp->numeric, glinep->mask);

	ssprintf(s, 
		"%s WA :[%s] has removed \002GLINE\002 for %s\n",
		config->server_numeric, userp->nickname, glinep->mask);

	delGline (glinep->mask);

	return 0;
}


//------------------------------------------------------------------------
// uw_whoison ()
//
//  This function will list all users and their status (chanop, chan voice)
//  on a specified channel.
//
// argv[0] nick numeric
// argv[3] = channel
//------------------------------------------------------------------------

int uw_whoison (int argc, char *argv[])
{
	User *myuserp;
	User *userp;
	Channel *channelp;
	NickList *nicklistp;

	int numUsers = 0;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command WHOISON\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric(argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	channelp = findChannel (argv[3]);
	if (!channelp)
	{
		ssprintf(s, 
			"%s O %s :Channel [%s] doesn't exist.\n",
			myuserp->numeric, argv[0], argv[3]);
		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002WHOISON\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s, 
		"%s O %s :\002WHOISON\002 Channel %s\n",
		myuserp->numeric, argv[0], argv[3]);


	nicklistp = channelp->nickhead;
	while (nicklistp)
	{
		ssprintf(s, 
			"%s O %s :[%s] is on [%s]\n",
			myuserp->numeric, argv[0], nicklistp->userp->nickname,
			channelp->name);
		if (IsChannelOp(nicklistp))
			ssprintf(s, 
				"%s O %s :|- IS A CHANNEL OP\n",
				myuserp->numeric, argv[0]);
		if (IsChannelVoice(nicklistp))
			ssprintf(s, 
				"%s O %s :|- IS A CHANNEL VOICE\n",
				myuserp->numeric, argv[0]);

		nicklistp = nicklistp->next;
	}

	ssprintf(s, 
		"%s O %s :End of WHOISON LIST\n",
		myuserp->numeric, argv[0]);


	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command WHOISON\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


//------------------------------------------------------------------------
// uw_banlist ()
//
//  This command will display a list of BANLIST on a specified channel
//
// argv[0] nick numeric
// argv[3] = channel
//------------------------------------------------------------------------

int uw_banlist (int argc, char *argv[])
{
	User *userp;
	User *myuserp;
	Channel *channelp;
	Ban	*banp;

	int numBans = 0;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command BANLIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	channelp = findChannel (argv[3]);
	if (!channelp)
	{
		ssprintf(s, 
			"%s O %s :Channel [%s] doesn't exist.\n",
			myuserp->numeric, argv[0], argv[3]);
		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002BANLIST\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s, 
		"%s O %s :\002BANLIST\002 Channel %s\n",
		myuserp->numeric, argv[0], argv[3]);

	banp = channelp->banhead;
	while (banp)
	{
		if (!strcasecmp(channelp->name, argv[3]))
		{
			ssprintf(s, 
				"%s O %s :[%s] is \002BANNED\002 on [%s]\n",
				myuserp->numeric, argv[0], banp->mask,
				channelp->name);

			numBans++;
		}
		banp = banp->next;
	}

	ssprintf(s, 
		"%s O %s :End of BANLIST LIST\n",
		myuserp->numeric, argv[0]);

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command BANLIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


//------------------------------------------------------------------------
// uw_xlate ()
//
//  This function will lookup the given numeric, and return the
//  nickname attached to it.
//
// argv[0] = sender numeric
// argv[3] = xlate numeric
//------------------------------------------------------------------------

int uw_xlate (int argc, char *argv[])
{
	User *myuserp;	// UWorld nickname
	User *userp;	// Sender nickname
	User *xuserp;	// xlate nickname

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command XLATE\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002XLATE\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s,
		"%s WA :\002XLATE\002 - [%s] used me to lookup %s\n",
		config->server_numeric, userp->nickname, argv[3]);

	xuserp = findUserByNumeric (argv[3]);
	if (!xuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find XLATE nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[3]);

		ssprintf(s, 
			"%s O %s :No such numeric\n",
			myuserp->numeric, userp->numeric);

		return 0;
	}
	else
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  XLATE nickname [%s] found\n",
			__FILE__, __LINE__, __FUNCTION__, argv[3]);

		ssprintf(s, 
			"%s O %s :Numeric \002%s\002 belongs to nickname \002%s\002.\n",
			myuserp->numeric, userp->numeric, xuserp->numeric, xuserp->nickname);

		return 1;
	}

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command XLATE\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


//------------------------------------------------------------------------
// uw_whois ()
//
//  This function will return extended whois information on the given
//  user.
//
// argv[0] = sender numeric
// argv[0] = whois nickname
//------------------------------------------------------------------------

int uw_whois (int argc, char *argv[])
{
	User *myuserp;	// UWorld nickname
	User *userp;	// Sender nickname
	User *wuserp;	// Whois nickname
	Server *serverp;	// Whois nickname's server
	ChanList *chanlistp;	// Whois nickname's channel list

	char channels[512];
	int chanlen, chanlistlen = 0;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command WHOIS\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002WHOIS\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	wuserp = findUser (argv[3]);
	if (!wuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  No such nickname\n",
			__FILE__, __LINE__, __FUNCTION__);

		ssprintf(s,
			"%s O %s :No such nickname \002%s\002\n",
			myuserp->numeric, userp->numeric, argv[3]);
		return 0;
	}

	serverp = findServerByNumeric (wuserp->server);
	if (!serverp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find USERS server\n",
			__FILE__, __LINE__, __FUNCTION__);

		return 0;
	}

	ssprintf(s,
		"%s O %s :Whois information on \002%s\002\n",
		myuserp->numeric, userp->numeric, wuserp->nickname);
	ssprintf(s,
		"%s O %s :User: %s (%s@%s)\n",
		myuserp->numeric, userp->numeric, wuserp->nickname,
		wuserp->username, wuserp->hostp->hostname);
	ssprintf(s,
		"%s O %s :Numeric: %s\n",
		myuserp->numeric, userp->numeric, wuserp->numeric);
	ssprintf(s,
		"%s O %s :Server: %s\n",
		myuserp->numeric, userp->numeric, serverp->name);

	if (wuserp->adminp)
	{
		if (IsAdminLogin(wuserp->adminp))
			ssprintf(s,
				"%s O %s :%s is a level \002%d\002 %s administrator\n",
				myuserp->numeric, userp->numeric, 
				wuserp->nickname, wuserp->adminp->level, 
				myuserp->nickname);
	}

	if (IsOperator(wuserp))
	{
		ssprintf(s,
			"%s O %s :%s is an IRC Operator\n",
			myuserp->numeric, userp->numeric, wuserp->nickname);
	}

	if (IsService(wuserp))
	{
		ssprintf (s,
			"%s O %s :%s is a IRC Service\n",
			myuserp->numeric, userp->numeric, 
			wuserp->nickname);
	}

	if (!IsService(wuserp))
	{
		chanlistp = wuserp->chanhead;
		if (chanlistp)
		{
			chanlistlen += strlen(chanlistp->channelp->name);
			if (chanlistlen < 512)
				strncpy(channels, chanlistp->channelp->name, 511);

			chanlistp = chanlistp->next;
			while (chanlistp)
			{
				strncat(channels, " ", 1);
				if (chanlistp->flags & IS_CHANOP)
				{
					strncat(channels, "@", 1);
					chanlistlen++;
				}
				else if (chanlistp->flags & IS_CHANVOICE)
				{
					strncat(channels, "+", 1);
					chanlistlen++;
				}

				chanlistlen += strlen(chanlistp->channelp->name) + 1;
				if (chanlistlen < 512)
				{
					strncat(channels, chanlistp->channelp->name, 511);
					chanlistp = chanlistp->next;
				}
				else
				{
					ssprintf(s, 
						"%s O %s :Channels: %s\n",
						myuserp->numeric, userp->numeric,
						channels);
					chanlistlen = 0;
					channels[0] = '\0';
				}
			}
			ssprintf(s, 
				"%s O %s :Channels: %s\n",
				myuserp->numeric, userp->numeric,
				channels);
		}
	}
			


	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command WHOIS\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 1;
}


///////////////////////////////////////////////////////////////////////////
// UWorld Level 0100 Commands
///////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------
// uw_clearbans ()
//
//  This command will remove all bans on a specified channel
//
// argv[0] = Sender nickname
// argv[3] = Channel
//------------------------------------------------------------------------

int uw_clearbans (int argc, char *argv[])
{
	User *myuserp;
	User *userp;
	Channel *channelp;
	Ban *banp;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command CLEARBANS\n",
		__FILE__, __LINE__, __FUNCTION__);


	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	channelp = findChannel(argv[3]);
	if (!channelp)
	{
		ssprintf(s, 
			"%s O %s :Channel [%s] doesn't exist.\n",
			myuserp->numeric, userp->numeric, argv[3]);
		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002CLEARBANS\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	banp = channelp->banhead;
	while (banp)
	{
		ssprintf(s, 
			"%s M %s -b %s\n",
			config->server_numeric, channelp->name, banp->mask);
		delChannelBan(channelp, banp->mask); 
		banp = banp->next;
	}

	ssprintf(s, 
		"%s WA :[%s] used me to \002CLEARBANS\002 on %s\n",
		config->server_numeric, userp->nickname, channelp->name);

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command CLEARBANS\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 1;
}

//------------------------------------------------------------------------
// uw_opermsg ()
//
//  This function will send a NOTICE out to all OPERS
//
// argv[0] = Sender numeric
// argv[3..parc] = Message
//------------------------------------------------------------------------

int uw_opermsg (int argc, char *argv[])
{
	User *myuserp;	// UWorld Nickname
	User *userp;	// Sender Nickname
	User *ouserp;	// Oper Nickname

	char message[512];
	int curarg = 3;
	argc--;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command OPERMSG\n",
		__FILE__, __LINE__, __FUNCTION__);


	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	strcpy(message, argv[3]);
	while (++curarg <= argc)
	{
		strcat(message, " ");
		strcat(message, argv[curarg]);
	}	

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002OPERMSG\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s,
		"%s WA :[%s] just issued a \002OPERMSG\002\n",
		config->server_numeric, userp->nickname);

	for (ouserp = userHEAD; ouserp; ouserp = ouserp->next)
	{
		if (IsOperator(ouserp))
		{
			ssprintf(s,
				"%s O %s :[\002OPERMSG\002] (%s) %s\n",
				myuserp->numeric, ouserp->numeric, 
				userp->nickname, message);
		}
	}

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command OPERMSG\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
	
}


//------------------------------------------------------------------------
// uw_clearops ()
//
//  This function will clear all ops on the given channel.
//
// argv[0] = sender numeric
// argv[3] = channel name
//------------------------------------------------------------------------


int uw_clearops (int argc, char *argv[])
{
	User *myuserp;	// Uworld nickname
	User *userp;	// Sender nickname
	Channel *channelp;	// Channel
	NickList *nicklistp;	// Users on channel

	int numDeops = 0;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command CLEAROPS\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	channelp = findChannel (argv[3]);
	if (!channelp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find requested channel [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[3]);

		ssprintf (s,
			"%s O %s :Channel [%s] doesn't exist\n",
			myuserp->numeric, userp->numeric, argv[3]);

		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002CLEAROPS\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	
	nicklistp = channelp->nickhead;
	while (nicklistp)
	{
		if ((IsChannelOp(nicklistp)) && (!IsService(nicklistp->userp)))
		{
			ssprintf(s, 
				"%s M %s -o %s\n",
				config->server_numeric, channelp->name, 
				nicklistp->userp->numeric);

			DelChannelOp(nicklistp);
		}

		if (IsChannelVoice(nicklistp))
		{
			ssprintf (s,
				"%s M %s -v %s\n",
				config->server_numeric, channelp->name,
				nicklistp->userp->numeric);

			DelChannelVoice(nicklistp);
		}

		nicklistp = nicklistp->next;
	}

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command CLEAROPS\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 1;
}


//------------------------------------------------------------------------
// uw_opcom ()
//
//  This function will allow opers/admins to op/deop voice/devoice
//  any user on any channel.
//
// argv[0] = sender nickname
// argv[3] = channel
// argv[4] = modes
// argv[5.argc] = users
//------------------------------------------------------------------------


int uw_opcom (int argc, char *argv[])
{
	User *myuserp;	// UWorld nickname
	User *userp;	// Sender nickname
	User *muserp;	// User modes were set on
	Channel *channelp;	// Channel
	ChanList *chanlistp;
	NickList *nicklistp;

	char nicks[512], numerics[512];
	int pos = -1, addModes = 0, curarg = 4, numSet = 0;
	argc--;

	strcpy(nicks, " ");
	strcpy(numerics, " ");

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command OPCOM\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	channelp = findChannel (argv[3]);
	if (!channelp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find requested channel [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[3]);

		ssprintf (s,
			"%s O %s :Channel [%s] doesn't exist\n",
			myuserp->numeric, userp->numeric, argv[3]);

		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002OPCOM\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);


	while (argv[4][++pos])
	{
		if (argv[4][pos] == '+')
		{
			addModes = 1;
			continue;
		}

		if (argv[4][pos] == '-')
		{
			addModes = 0;
			continue;
		}

		if (argv[4][pos] == 'o')
		{
			++curarg;

			if (curarg > argc)
			{
				goto dumpEarly;
			}


			muserp = findUser(argv[curarg]);
			if (!muserp)
				continue;

			chanlistp = findNickChannel (muserp, channelp);
			if (!chanlistp)
				continue;

			nicklistp = findChannelNick (channelp, muserp);
			if (!nicklistp)
				return;

			if (addModes)
			{
				strcat (nicks, muserp->nickname);
				strcat (nicks, " ");
				strcat (numerics, muserp->numeric);
				strcat (numerics, " ");

				SetChannelOp (nicklistp);
				SetChannelOp (chanlistp);
			}
			else
			{
				strcat (nicks, muserp->nickname);
				strcat (nicks, " ");
				strcat (numerics, muserp->numeric);
				strcat (numerics, " ");

				DelChannelOp (nicklistp);
				DelChannelOp (chanlistp);
			}
		}

		if (argv[4][pos] == 'v')
		{
			++curarg;

			if (curarg > argc)
			{
				goto dumpEarly;
			}


			muserp = findUser(argv[curarg]);
			if (!muserp)
				continue;

			chanlistp = findNickChannel (muserp, channelp);
			if (!chanlistp)
				continue;

			nicklistp = findChannelNick (channelp, muserp);
			if (!nicklistp)
				return;

			if (addModes)
			{
				strcat (nicks, muserp->nickname);
				strcat (nicks, " ");
				strcat (numerics, muserp->numeric);
				strcat (numerics, " ");

				SetChannelVoice (nicklistp);
				SetChannelVoice (chanlistp);
			}
			else
			{
				strcat (nicks, muserp->nickname);
				strcat (nicks, " ");
				strcat (numerics, muserp->numeric);
				strcat (numerics, " ");

				DelChannelVoice (nicklistp);
				DelChannelVoice (chanlistp);
			}
		}

	}

	ssprintf(s, 
		"%s WA :%s is using me to: MODE %s %s%s\n",
		config->server_numeric, userp->nickname, argv[3], argv[4], nicks);

dumpEarly:
	ssprintf (s,
		"%s M %s %s %s\n",
		config->server_numeric, argv[3], argv[4], numerics);

	return 1;
}


//------------------------------------------------------------------------
// uw_clearmodes ()
//
//  This function will clear ALL modes from the given channel.
//
// argv[0] = sender numeric
// argv[3] = channel
//------------------------------------------------------------------------

int uw_clearmodes (int argc, char *argv[])
{
	User *myuserp;
	User *userp;
	Channel *channelp;

	char undo[64];

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	channelp = findChannel (argv[3]);
	if (!channelp)
	{
		ssprintf (s,
			"%s O %s :Channel [%s] doesn't exist.\n",
			myuserp->numeric, userp->numeric, argv[3]);
		return 0;
	}

	strncpy(undo, "-ntismpkl ", 10);

	if (channelp->flags & CHANNEL_HAS_KEY)
		strcat(undo, channelp->key);

	ssprintf (s,
		"%s M %s %s\n",
		config->server_numeric, channelp->name, undo);

	channelp->flags &= ~CHANNEL_HAS_KEY;
	channelp->flags &= ~CHANNEL_HAS_LIMIT;

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002CLEARMODES\002 issued by \002%s\002 on \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname, channelp->name);

	return 1;
}

//------------------------------------------------------------------------
// uw_scan ()
//
//  This function will scan a hostmask, and give you a list of users
//  matching the hostmask.
//
// argv[0] = sender numeric
// argv[3] = hostmask
//------------------------------------------------------------------------


int uw_scan (int argc, char *argv[])
{
	User *myuserp;
	User *userp;
	User *suserp;
	int numFound = 0;
	
	char userhost[1024];

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command SCAN\n",
		__FILE__, __LINE__, __FUNCTION__);
		
	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		return 0;
	}
	
	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);
			
		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);
			
		return 0;
	}
	
	
	for (suserp = userHEAD; suserp; suserp = suserp->next)
	{
		Server *serverp;
		
		sprintf(userhost, "%s@%s", suserp->username, suserp->hostp->hostname);
		
		if (!match(argv[3], userhost))
		{
			serverp = findServerByNumeric (userp->server);
			ssprintf(s, 
				"%s O %s :%s (%s) %s\n",
				myuserp->numeric, userp->numeric, suserp->nickname,
				userhost, serverp->name);
			numFound++;
		}
	}
	
	if (numFound)
	{
		ssprintf (s,
			"%s O %s :End of list - %d matches found\n",
			myuserp->numeric, userp->numeric, numFound);
		return 1;
	}
	else
	{
		ssprintf (s,
			"%s O %s :No matches found!\n",
			myuserp->numeric, userp->numeric);
		return 0;
	}
}


//------------------------------------------------------------------------
// uw_mode ()
//
//  This function will allow admins to change channel modes without
//  having to be opped.
//
// argv[0] = sender numeric
// argv[3] = channel name
// argv[4] = mode string
// argv[5] = (optional) key | limit
// argv[6] = (optional) key | limit
//------------------------------------------------------------------------


int uw_mode (int argc, char *argv[])
{
	User *myuserp;
	User *userp;
	Channel *channelp;
	int addModes, pos = -1, curarg = 5;
	char mode_string[512], limit[6];
	
	
	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);
			
		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);
			
		return 0;
	}
	
	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);
			
		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);
			
		return 0;
	}
	
	
	channelp = findChannel (argv[3]);
	if (!channelp)
	{
		ssprintf (s,
			"%s O %s :No such channel.\n",
			myuserp->numeric, userp->numeric);
		return 0;
	}
	
	strncpy (mode_string, argv[4], 32);
	while (argv[4][++pos] != '\0')
	{
		if (argv[4][pos] == '+')
		{
			addModes = 1;
		}
		
		else if (argv[4][pos] == '-')
		{
			addModes = 0;
		}
		
		else if (argv[4][pos] == 'l')
		{
			if (addModes)
			{
				if (argc >= curarg)
				{
					sprintf(limit, "%d", atoi(argv[curarg]));
					strcat(mode_string, " ");
					strcat(mode_string, limit);
					curarg++;
				}
			}
		}
		
		else if (argv[4][pos] == 'k')
		{
			if (addModes)
			{
				if (argc >= curarg)
				{
					strcat(mode_string, " ");
					strcat(mode_string, argv[curarg]);
					curarg++;
					channelp->flags |= CHANNEL_HAS_KEY;
				}
			}
		
			else if ((channelp->flags & CHANNEL_HAS_KEY) && !addModes)
			{
				strcat (mode_string, " ");
				strcat (mode_string, channelp->key);
			}
		}
	}
	
	ssprintf (s,
		"%s M %s %s\n",
		config->server_numeric, channelp->name, mode_string);
		
	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002MODE\002 issued by \002%s\002 on channel \002%s\002 with modes \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname,
		channelp->name, mode_string);
		
}
		


///////////////////////////////////////////////////////////////////////////
//  UWorld Level 0900 Commands
///////////////////////////////////////////////////////////////////////////



//------------------------------------------------------------------------
// uw_masskill ()
//
//  This command will kill anyone that matchs the specified hostmask
//
//  argv[0] = sender
//  argv[3] = hostmask
//  argv[4] = reason
//------------------------------------------------------------------------


int uw_masskill (int argc, char *argv[])
{
	User *userp;
	User *myuserp;
	User *kuserp;

	int curarg = 4;
	char reason[512], hostmask[512];

	ircsp_debug(
		"@(#) - (%s:%d) %s():  UWorld command MASSKILL\n",
		__FILE__, __LINE__, __FUNCTION__);


	userp = findUserByNumeric(argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	argc--;
	strcpy(reason, argv[curarg]);
	while (++curarg <= argc)
	{
		strcat(reason, " ");
		strcat(reason, argv[curarg]);
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002MASSKILL\002 issued on \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	for (kuserp = userHEAD; kuserp; kuserp = kuserp->next)
	{
		sprintf(hostmask, "%s@%s", kuserp->username, kuserp->hostp->hostname);

		if (!match(argv[3], hostmask))
		{
			ssprintf(s, 
				"%s D %s :%s!%s (MASSKILL (%s)\n",
				config->server_numeric, kuserp->numeric, myuserp->hostp->hostname, myuserp->nickname, reason);

			if (kuserp->hostp->numHosts > 1)
				kuserp->hostp->numHosts--;
			else
			{
				delHostname(kuserp->hostp->hostname);
				kuserp->hostp = NULL;
			}			
			delUser(kuserp->nickname);
		}
	}


	ssprintf(s, 
		"%s WA :[\002%s\002] Complete \002MASSKILL\002 on host [%s]\n", 
		config->server_numeric, userp->nickname, argv[3]);
		

	ircsp_debug(
		"@(#) - (%s:%d) %s():  End UWorld command MASSKILL\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;

}


///////////////////////////////////////////////////////////////////////////
//  UWorld Level 1000 Commands
///////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------
// uw_save ()
//
//  This command will save all our databases.
//
// argv[0] = Sender Numeric
//------------------------------------------------------------------------

int uw_save (int argc, char *argv[])
{
	User *myuserp;
	User *userp;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command SAVE\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002SAVE\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s, 
		"%s O %s :Saving Databases...\n",
		myuserp->numeric, userp->numeric);


	ssprintf(s, 
		"%s WA :\002SAVE\002 Requsted by [\002%s\002]\n",
		config->server_numeric, userp->nickname);

	writeAdminDB();
	uw_saveGlineDB ();
	uw_saveCommand ();
	
	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command SAVE\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 1;
}


//------------------------------------------------------------------------
// uw_die ()
//
//  Save our databases and exit.
//
// argv[0] = Sender Numeric.
//------------------------------------------------------------------------


int uw_die (int argc, char *argv[])
{
	User *myuserp;
	User *userp;



	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	if (!userp->adminp)
	{
		ssprintf(s, 
			"%s O %s :\002ACCESS DENIED\002\n",
			myuserp->numeric, userp->numeric);
		return 0;
	}


	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002DIE\002 issued \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s, 
		"%s O %s :Saving databases, and shutting down.\n",
		myuserp->numeric, userp->numeric);

	writeAdminDB ();
	uw_saveGlineDB ();
	uw_saveCommand ();


	ircsp_exit (0, userp->nickname, "Shutdown Requested");

	return 1;
}


//------------------------------------------------------------------------
// uw_cmdlist ()
//
//  This command will give a list of all UWorld commands, their levels
//  and their status.
//
// argv[0] = Sender Numeric
//------------------------------------------------------------------------


int uw_cmdlist (int argc, char *argv[])
{
	User *userp;
	User *myuserp;
	int cmd;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command CMDLIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002CMDLIST\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s, 
		"%s O %s :\002CMDLIST\002 \n",
		myuserp->numeric, argv[0]);


	for (cmd = 0; cmd < UW_NUM_CMDS; cmd++)
	{
		ssprintf(s, 
			"%s O %s :\002%s\002 - Level: \002%d\002\n",
			myuserp->numeric, argv[0], UWorldCommand[cmd].command,
			UWorldCommand[cmd].minlevel);

		if (UWorldCommand[cmd].disabled)
			ssprintf(s, 
				"%s O %s :|-  \002COMMAND DISABLED\002\n",
				myuserp->numeric, argv[0]);
	}

	ssprintf(s, 
		"%s O %s :End of \002CMDLIST\002\n",
		myuserp->numeric, argv[0]);

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command CMDLIST\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}


//------------------------------------------------------------------------
// uw_disablecmd ()
//
//  This command will allow admins to disable commands.
//
// argv[0] = sender numeric
// argv[3] = command
//------------------------------------------------------------------------

int uw_disablecmd (int argc, char *argv[])
{
	User *userp;
	User *myuserp;
	int cmd, exists = 0;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command DISABLECMD\n",
		__FILE__, __LINE__, __FUNCTION__);


	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	for (cmd = 0; cmd <= UW_NUM_CMDS; cmd++)
	{
		if ((!strcasecmp(argv[3], "AUTH")) || 
			(!strcasecmp(argv[3], "DISABLECMD")) ||
			(!strcasecmp(argv[3], "ENABLECMD")))
		{
			ssprintf(s, 
				"%s O %s :We cannot disable \002%s\002\n",
				myuserp->numeric, argv[0], argv[3]);
			return 0;
		}
		else if (!strcasecmp(UWorldCommand[cmd].command, argv[3]))
		{
			exists = 1;
			if (UWorldCommand[cmd].disabled)
			{
				ssprintf(s, 
					"%s O %s :\002%s\002 already \002DISABLED\002\n",
					myuserp->numeric, argv[0], UWorldCommand[cmd].command);
				return 0;
			}
			else
			{
				ssprintf(s, 
					"%s O %s :\002%s\002 is now \002DISABLED\002\n",
					myuserp->numeric, argv[0], UWorldCommand[cmd].command);

				UWorldCommand[cmd].disabled = 1;
				uw_saveCommand ();
				return 1;
			}
		}
	}

	if (!exists)
	{
		ssprintf(s, 
			"%s O %s :Command [%s] not found.\n",
			myuserp->numeric, argv[0], argv[3]);
		return 0;
	}

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command DISABLECMD\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}

//------------------------------------------------------------------------
// uw_enablecmd ()
//
//  This command will allow admins to re-enable commands.
//
// argv[0] = Sender numeric
// argv[3] = command
//------------------------------------------------------------------------

int uw_enablecmd (int argc, char *argv[])
{
	User *userp;
	User *myuserp;
	int cmd, exists = 0;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command ENABLECMD\n",
		__FILE__, __LINE__, __FUNCTION__);

	
	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	for (cmd = 0; cmd <= UW_NUM_CMDS; cmd++)
	{
		if ((!strcasecmp(argv[3], "AUTH")) ||
			(!strcasecmp(argv[3], "DISABLECMD")) ||
			(!strcasecmp(argv[3], "ENABLECMD")))
		{
			ssprintf(s, 
				"%s O %s :\002%s\002 is \002ENABLED\002 by default.\n",
				myuserp->numeric, argv[0], argv[3]);

			return 0;
		}

		else if (!strcasecmp(UWorldCommand[cmd].command, argv[3]))
		{
			exists = 1;
			if (!UWorldCommand[cmd].disabled)
			{
				ssprintf(s, 
					"%s O %s :\002%s\002 is already \002ENABLED\002\n",
					myuserp->numeric, argv[0], argv[3]);
				return 0;
			}
			else
			{
				ssprintf(s, 
					"%s O %s :\002%s\002 is \002ENABLED\002\n",
					myuserp->numeric, argv[0], argv[3]);
				UWorldCommand[cmd].disabled = 0;
				uw_saveCommand ();
				return 1;
			}
		}
	}

	if (!exists)
	{
		ssprintf(s, 
			"%s O %s :\002%s\002 was not found.\n",
			myuserp->numeric, argv[0], argv[3]);
		return 0;
	}
	
	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command ENABLECMD\n",
		__FILE__, __LINE__, __FUNCTION__);
			
	return 0;
}



//------------------------------------------------------------------------
// uw_restart ()
//
//  This function will cause IRCSP to split off, save its databases
//  and re-connect.
//
// argv[0] = Sender Numeric
// argv[3] = Reason
//------------------------------------------------------------------------


int uw_restart (int argc, char *argv[])
{
	User *myuserp;
	User *userp;

	char reason[512];
	int curarg = 3;
	argc--;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command RESTART\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	if (!config->fork)
	{
		ssprintf(s,
			"%s O %s :\002WARNING\002 - Proccess is not forked, this command is disabled\n",
			myuserp->numeric, userp->numeric);
		return 0;
	}

	// Copy our reason

	strcpy(reason, argv[3]);
	while (++curarg <= argc)
	{
		strcat(reason, " ");
		strcat(reason, argv[curarg]);
	}

	ssprintf (s,
		"%s O %s :Saving databases, and restarting.\n",
		myuserp->numeric, userp->numeric);

	writeAdminDB ();
	uw_saveGlineDB ();
	uw_saveCommand ();

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002RESTART\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	ssprintf(s, 
		"%s WA :\002RESTART\002 requested by \002%s\002 (%s)\n",
		config->server_numeric, userp->nickname, reason);


	close(s);

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command RESTART\n",
		__FILE__, __LINE__, __FUNCTION__);

	system ("source/ircsp");

	raise (SIGTERM);

	return 1;
}


//------------------------------------------------------------------------
// uw_rehash ()
//
//  This function should remove all admins, and glines from memory 
//  and reload them from database.
//
// argv[0] = Sender Numeric
//------------------------------------------------------------------------


int uw_rehash (int argc, char *argv[0])
{
	User *myuserp;
	User *userp;
	Admin *adminp;
	Gline *glinep;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command REHASH\n",
		__FILE__, __LINE__, __FUNCTION__);

	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}


	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002REHASH\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);

	

	ssprintf (s,
		"%s WA :REHASHING [Requested: %s]\n",
		config->server_numeric, userp->nickname);

	service->rehashing = 1;


	// Remove GLINES from memory.

	for (glinep = glineHEAD; glinep; glinep = glinep->next)
	{
		delGline (glinep->mask);
	}


	// Remove Administrators from memory and send out notice

	for (adminp = adminHEAD; adminp; adminp = adminp->next)
	{
		if (IsAdminLogin(adminp))
		{
			User *uuserp;

			ssprintf(s, 
				"%s O %s :Notice IRCSP is REHASHING.  You will have to re-login.\n",
				myuserp->numeric, adminp->userp->numeric);

			uuserp = findUser (adminp->userp->nickname);

			uuserp->adminp = NULL;
			adminp->userp = NULL;

			DelAdminLogin(adminp);
			DelAdministrator (uuserp);
			
		}
		delAdmin (adminp->nickname);
	}

	// Reload ADMINS from DB

	loadAdminDB ();
			

	// Reload GLINES from the DB

	uw_loadGlineDB ();

	// Now we need to SYNC our glines.

	synch ();

	service->rehashing = 0;

	ircsp_debug (
		"@(#) - (%s:%d) %s():  End UWorld command REHASH\n",
		__FILE__, __LINE__, __FUNCTION__);

}




//------------------------------------------------------------------------
// uw_chlevel ()
//
//  This command will allow admins to change the minium level required
//  for a command.
//
// argv[0] = Sender Numeric
// argv[3] = Command
// argv[4] = New Level
//------------------------------------------------------------------------


int uw_chlevel (int argc, char *argv[])
{
	User *myuserp;
	User *userp;

	int command, found;


	ircsp_debug (
		"@(#) - (%s:%d) %s():  UWorld command CHLEVEL\n",
		__FILE__, __LINE__, __FUNCTION__);


	userp = findUserByNumeric (argv[0]);
	if (!userp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find SENDER nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__);

		return 0;
	}

	myuserp = findUser (config->uworld_nickname);
	if (!myuserp)
	{
		ircsp_debug (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		ircsp_warning (
			"@(#) - (%s:%d) %s():  Failed to find UWORLD nickname [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		return 0;
	}

	if ((!strcasecmp(argv[3], "chlevel")))
	{
		ssprintf(s,
			"%s O %s :The minimum level on \002CHLEVEL\002 cannot be changed\n",
			myuserp->numeric, userp->numeric);

		return 0;
	}

	ircsp_notice (
		"@(#) - (%s:%d) %s():  UWorld command \002CHLEVEL\002 issued by \002%s\002\n",
		__FILE__, __LINE__, __FUNCTION__, userp->nickname);


	if (atoi(argv[4]) > 1000)
	{
		ssprintf(s,
			"%s O %s :The max level that can be used is 1000\n",
			myuserp->numeric, userp->numeric);
		return 0;
	}

	for (command = 0; command < UW_NUM_CMDS; command++)
	{
		if (!strcasecmp(UWorldCommand[command].command, argv[3]))
		{
			UWorldCommand[command].minlevel = atoi(argv[4]);
			ssprintf(s,
				"%s O %s :The Minimum level for \002%s\002 has been changed to \002%d\002\n",
				myuserp->numeric, userp->numeric,
				UWorldCommand[command].command,
				UWorldCommand[command].minlevel);

			return 1;
		}
		else
		{
			found = 0;
		}
	}

	if (!found)
	{
		ssprintf(s,
			"%s O %s :Command [\002%s\002] Not found\n",
			myuserp->numeric, argv[0]);
		return 0;
	}
}

	
