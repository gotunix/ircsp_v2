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
// $Revision: 1.39 $
// $Author: jovens $
// $Date: 2009/04/11 06:40:36 $
//------------------------------------------------------------------------






#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <time.h>


#include "conf.h"
#include "memory.h"
#include "numeric.h"
#include "debug.h"
#include "ircsp.h"
#include "misc.h"
#include "strings.h"
#include "log.h"
#include "flags.h"
#include "macros.h"

//------------------------------------------------------------------------
// ircsp_loadConfig ()
//
//
//  Load IRCSP config file and push it to the parser.
//------------------------------------------------------------------------

int ircsp_loadConfig (void)
{
	FILE *f;
	char read[256], *argv[2];
	
	LOG (MAINLOG, "[%s:%d:%s()]:  Starting\n",
		__FILE__, __LINE__, __FUNCTION__);

	f = fopen (__CONFIG_FILE__, "r");
	if (!f)
	{
		ERROR (998, "Failed to load IRCSP config file\n", TRUE);
	}
	else
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  Loading config file [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, __CONFIG_FILE__);
			

		LOG (MAINLOG, "[%s:%d:%s()]:  Allocating memory for configHEAD\n",
			__FILE__, __LINE__, __FUNCTION__);

		config = (Config *)ircsp_calloc(1, sizeof(Config));
		if (!config)
		{
			ERROR (150, "Failed to allocate memory for configHEAD\n", TRUE);
		}
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for configHEAD at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config);
			
	
		while (!feof(f))
		{
			if (fgets(read, 256, f))
			{
				if (isComment(read))
				{
					LOG (MAINLOG, "[%s:%d:%s()]:  Ignoring comments\n",
						__FILE__, __LINE__, __FUNCTION__);
						
					continue;
				}

				if (isBlank (read))
				{
					LOG (MAINLOG, "[%s:%d:%s()]:  Ignoring empty space\n",
						__FILE__, __LINE__, __FUNCTION__);

					continue;
				}

				argv[0] = strtok(read, " ");
				argv[1] = strtok(NULL, "\n");

				ircsp_parseConfig(2, argv);
			}
		}

		LOG (MAINLOG, "[%s:%d:%s()]:  Config file loaded\n",
			__FILE__, __LINE__, __FUNCTION__);
		return 1;
	}
	return 0;
}



//------------------------------------------------------------------------
// ircsp_parseConfig ()
//
//  Parse up our config file.
//------------------------------------------------------------------------


int ircsp_parseConfig (int argc, char *argv[])
{
	LOG (MAINLOG, "[%s:%d:%s()]:  Starting config parser\n",
		__FILE__, __LINE__, __FUNCTION__);


	if (!strcasecmp (argv[0], "PROTOCOL"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  PROTOCOL\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		if (atoi(argv[1]) == PROTOCOL_IS_UNDERNET)
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  UNDERNET PROTOCOL\n",
				__FILE__, __LINE__, __FUNCTION__);
				
			SetProtocolUndernet (config);
		}
		
		else if (atoi(argv[1]) == PROTOCOL_IS_DALNET)
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  DALNET PROTOCOL\n",
				__FILE__, __LINE__, __FUNCTION__);
				
			SetProtocolDalnet (config);
		}
		
		else if (atoi(argv[1]) == PROTOCOL_IS_EFNET)
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  EFNET PROTOCOL\n",
				__FILE__, __LINE__, __FUNCTION__);
				
			SetProtocolEfnet (config);
		}
	}

	else if (!strcasecmp (argv[0], "SERVER"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  SERVER\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->server = (char *)ircsp_calloc(1, strlen(argv[1])+5);
		if (!config->server)
		{
			ERROR (150, "Failed to allocate memory for SERVER\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for SERVER at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->server);

		strncpy (config->server, argv[1], strlen(argv[1])+5);
	}

	else if ( !strcasecmp (argv[0], "SERVER-COMMENT"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  SERVER-COMMENT\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->server_comment = (char *)ircsp_calloc(1, strlen(argv[1])+5);
		if (!config->server_comment)
		{
			ERROR (150, "Failed to allocate memory for SERVER-COMMENT\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for SERVER-COMMENT at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->server_comment);

		strncpy (config->server_comment, argv[1], strlen(argv[1])+5);
	}

	else if (!strcasecmp(argv[0], "SERVER-NUMERIC"))
	{
		char num[4], *numeric;

		LOG (MAINLOG, "[%s:%d:%s()]:  SERVER-NUMERIC\n",
			__FILE__, __LINE__, __FUNCTION__);

		numeric = intobase64(num, atoi(argv[1]), 2);

		config->server_numeric = (char *)ircsp_calloc(1, strlen(numeric) + 5);
		if (!config->server_numeric)
		{
			ERROR (150, "Failed to allocate memory for NUMERIC\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for NUMERIC at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->server_numeric);
			
		strncpy(config->server_numeric, numeric, 2);
	}

	else if (!strcasecmp(argv[0], "UPLINK"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  UPLINK\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->uplink = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->uplink)
		{
			ERROR (150, "Failed to allocate memory for UPLINK\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for UPLINK at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uplink);

		strncpy(config->uplink, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "UPLINK-PORT"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  UPLINK-PORT\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->uplink_port = atoi (argv[1]);
	}

	else if (!strcasecmp(argv[0], "UPLINK-PASS"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  UPLINK-PASS\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->uplink_pass = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->uplink_pass)
		{
			ERROR (150, "Failed to allocate memory for UPLINK-PASS\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for UPLINK-PASS at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uplink_pass);

		strncpy (config->uplink_pass, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "UWORLD-NICKNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  UWORLD-NICKNAME\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->uworld_nickname = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->uworld_nickname)
		{
			ERROR (150, "Failed to allocate memory for UWORLD-NICKNAME\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for UWORLD-NICKNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_nickname);

		strncpy(config->uworld_nickname, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "UWORLD-USERNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  UWORLD-USERNAME\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->uworld_username = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->uworld_username)
		{
			ERROR (150, "Failed to allocate memory for UWORLD-USERNAME\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for UWORLD-USERNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_username);

		strncpy(config->uworld_username, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "UWORLD-HOSTNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  UWORLD-HOSTNAME\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->uworld_hostname = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->uworld_hostname)
		{
			ERROR (150, "Failed to allocate memory for UWORLD-HOSTNAME\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for UWORLD-HOSTNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_hostname);

		strncpy(config->uworld_hostname, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "UWORLD-REALNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  UWORLD-REALNAME\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->uworld_realname = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->uworld_realname)
		{
			ERROR (150, "Failed to allocate memory for UWORLD-REALNAME\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for UWORLD-REALNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_realname);

		strncpy(config->uworld_realname, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "UWORLD-IRCMODES"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  UWORLD-IRCMODES\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->uworld_ircmodes = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->uworld_ircmodes)
		{
			ERROR (150, "Failed to allocate memory for UWORLD-IRCMODES\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for UWORLD-IRCMODES at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uworld_ircmodes);

		strncpy(config->uworld_ircmodes, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "UWORLD-ENABLE"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  UWORLD-ENABLE\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->uworld_enable = atoi (argv[1]);
	}

	else if (!strcasecmp(argv[0], "CSERVE-NICKNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  CSERVE-NICKNAME\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->cserve_nickname = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->cserve_nickname)
		{
			ERROR (150, "Failed to allocate memory for CSERVE-NICKNAME\n", TRUE);
		}

		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for CSERVE-NICKNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->cserve_nickname);

		strncpy(config->cserve_nickname, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "CSERVE-USERNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  CSERVE-USERNAME\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->cserve_username = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->cserve_username)
		{
			ERROR (150, "Failed to allocate memory for CSERVE-USERNAME\n", TRUE);
		}

		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for CSERVE-USERNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->cserve_username);

		strncpy(config->cserve_username, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "CSERVE-HOSTNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  CSERVE-HOSTNAME\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->cserve_hostname = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->cserve_hostname)
		{
			ERROR (150, "Failed to allocate memory for CSERVE-HOSTNAME\n", TRUE);
		}

		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for CSERVE-HOSTNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->cserve_hostname);

		strncpy(config->cserve_hostname, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "CSERVE-REALNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  CSERVE-REALNAME\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->cserve_realname = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->cserve_realname)
		{
			ERROR (150, "Failed to allocate memory for CSERVE-REALNAME\n", TRUE);
		}

		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for CSERVE-REALNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->cserve_realname);

		strncpy(config->cserve_realname, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "CSERVE-IRCMODES"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  CSERVE-IRCMODES\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->cserve_ircmodes = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->cserve_ircmodes)
		{
			ERROR (150, "Failed to allocate memory for CSERVE-IRCMODES\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for CSERVE-IRCMODES at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->cserve_ircmodes);

		strncpy(config->cserve_ircmodes, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "CSERVE-ENABLE"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  CSERVE-ENABLE\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->cserve_enable = atoi(argv[1]);
	}

	else if (!strcasecmp(argv[0], "NSERVE-NICKNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  NSERVE-NICKNAME\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->nserve_nickname = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->nserve_nickname)
		{
			ERROR (150, "Failed to allocate memory for NSERVE-NICKNAME\n", TRUE);
		}

		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for NSERVE-NICKNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->nserve_nickname);

		strncpy(config->nserve_nickname, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "NSERVE-USERNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  NSERVE-USERNAME\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->nserve_username = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->nserve_username)
		{
			ERROR (150, "Failed to allocate memory for NSERVE-USERNAME\n", TRUE);
		}

		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for NSERVE-USERNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->nserve_username);
			
		strncpy(config->nserve_username, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "NSERVE-HOSTNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  NSERVE-HOSTNAME\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->nserve_hostname = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->nserve_hostname)
		{
			ERROR (150, "Failed to allocate memory for NSERVE-HOSTNAME\n", TRUE);
		}

		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for NSERVE-HOSTNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->nserve_hostname);

		strncpy(config->nserve_hostname, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "NSERVE-REALNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  NSERVE-REALNAME\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->nserve_realname = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->nserve_username)
		{
			ERROR (150, "Failed to allocate memory for NSERVE-REALNAME\n", TRUE);
		}

		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for NSERVE-REALNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->nserve_realname);

		strncpy(config->nserve_realname, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "NSERVE-IRCMODES"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  NSERVE-IRCMODES\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->nserve_ircmodes = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->nserve_ircmodes)
		{
			ERROR (150, "Failed to allocate memory for NSERVE-IRCMODES\n", TRUE);
		}

		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for NSERVE-IRCMODES at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->nserve_ircmodes);

		strncpy(config->nserve_ircmodes, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "NSERVE-ENABLE"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  NSERVE-ENABLE\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->nserve_enable = atoi (argv[1]);
	}

	else if (!strcasecmp(argv[0], "OPERCHANNEL-NAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  OPERCHANNEL-NAME\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->operchannel_name = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->operchannel_name)
		{
			ERROR (150, "Failed to allocate memory for OPERCHANNEL-NAME\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for OPERCHANNEL-NAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->operchannel_name);
			
		strncpy(config->operchannel_name, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "OPERCHANNEL-MODES"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  OPERCHANNEL-MODES\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->operchannel_modes = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->operchannel_modes)
		{
			ERROR (150, "Failed to allocate memory for OPERCHANNEL-MODES\n", TRUE);
		}

		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for OPERCHANNEL-MODES at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->operchannel_modes);

		strncpy (config->operchannel_modes, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "OPERCHANNEL-NOTICE"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  OPERCHANNEL-NOTICE\n", 
			__FILE__, __LINE__, __FUNCTION__);
			
		config->operchannel_notice = atoi (argv[1]);
	}

	else if (!strcasecmp(argv[0], "DEBUGCHANNEL-NAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  DEBUGCHANNEL-NAME\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->debugchannel_name = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->debugchannel_name)
		{
			ERROR (150, "Failed to allocate memory for DEBUGCHANNEL-NAME\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for DEBUGCHANNEL-NAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->debugchannel_name);

		strncpy(config->debugchannel_name, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "DEBUGCHANNEL-MODES"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  DEBUGCHANNEL-MODES\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->debugchannel_modes = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->debugchannel_modes)
		{
			ERROR (150, "Failed to allocate memory for DEBUGCHANNEL-MODES\n", TRUE);
		}

		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for DEBUGCHANNEL-MODES at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->debugchannel_modes);

		strncpy (config->debugchannel_modes, argv[1], strlen(argv[1]) + 5);
	}
	
	else if (!strcasecmp(argv[0], "DENY-MESSAGE"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  DENY-MESSAGE\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->deny_message = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->deny_message)
		{
			ERROR (150, "Failed to allocate memory for DENY-MESSAGE\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for DENY-MESSAGE at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->deny_message);
			
		strncpy (config->deny_message, argv[1], strlen(argv[1]) + 5);
	}
	
	else if (!strcasecmp(argv[0], "JUPE-USERNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  JUPE-USERNAME\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->jupe_username = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->jupe_username)
		{
			ERROR (150, "Failed to allocate memory for JUPE-USERNAME\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for JUPE-USERNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->jupe_username);
			
		strncpy(config->jupe_username, argv[1], strlen(argv[1]) + 5);
	}
	
	else if (!strcasecmp(argv[0], "JUPE-HOSTNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  JUPE-HOSTNAME\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->jupe_hostname = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->jupe_hostname)
		{
			ERROR (150, "Failed to allocate memory for JUPE-HOSTNAME\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for JUPE-HOSTNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->jupe_hostname);
			
		strncpy(config->jupe_hostname, argv[1], strlen(argv[1]) + 5);
	}
	
	else if (!strcasecmp(argv[0], "JUPE-REALNAME"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  JUPE-REALNAME\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->jupe_realname = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->jupe_realname)
		{
			ERROR (150, "Failed to allocate memory for JUPE-REALNAME\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for JUPE-REALNAME at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->jupe_realname);

		strncpy (config->jupe_realname, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "JUPE-IRCMODES"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  JUPE-IRCMODES\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->jupe_ircmodes = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->jupe_ircmodes)
		{
			ERROR (150, "Failed to allocate memory for JUPE-IRCMODES\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for JUPE-IRCMODES at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->jupe_ircmodes);
			
		strncpy(config->jupe_ircmodes, argv[1], strlen(argv[1]) + 5);
	}
	
	else if (!strcasecmp(argv[0], "JUPE-DB"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  JUPE-DATABASE\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->jupe_database = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->jupe_database)
		{
			ERROR (150, "Failed to allocate memory for JUPE-DATABASE\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for JUPE-DATABASE at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->jupe_database);

		strncpy(config->jupe_database, argv[1], strlen(argv[1]) + 5);
	}
			

	else if (!strcasecmp(argv[0], "FORK"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  FORK\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->fork = atoi (argv[1]);
	}
	
	else if (!strcasecmp(argv[0], "NUMERIC-XPOS"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  NUMERIC-XPOS\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->xPos = atoi (argv[1]);
	}
	
	else if (!strcasecmp(argv[0], "NUMERIC-YPOS"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  NUMERIC-YPOS\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->yPos = atoi (argv[1]);
	}
	
	else if (!strcasecmp(argv[0], "NUMERIC-YYPOS"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  NUMERIC-YYPOS\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->yyPos = atoi (argv[1]);
	}

	else if (!strcasecmp(argv[0], "CLONE-WARN"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  CLONE-WARN\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->clone_warn = atoi (argv[1]);
	}

	else if (!strcasecmp(argv[0], "CLONE-BAN"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  CLONE-BAN\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->clone_ban = atoi (argv[1]);
	}

	else if (!strcasecmp(argv[0], "ADMIN-DB"))
	{
		LOG (MAINLOG, "[%s:%d:%s():  ADMIN-DB\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->admin_db = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->admin_db)
		{
			ERROR (150, "Failed to allocate memory for ADMIN-DB\n", TRUE);
		}

		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for ADMIN-DB at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->admin_db);
			
		strncpy(config->admin_db, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "GLINE-DB"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  GLINE-DB\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->gline_db = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->gline_db)
		{
			ERROR (150, "Failed to allocate memory for GLINE-DB\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for GLINE-DB at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->gline_db);
			
		strncpy(config->gline_db, argv[1], strlen(argv[1]) + 5);
	}

	else if (!strcasecmp(argv[0], "UWORLD-CMD-DB"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  UWORLD-CMD-DB\n",
			__FILE__, __LINE__, __FUNCTION__);

		config->uw_cmddb = (char *)ircsp_calloc(1, strlen(argv[1]) + 5);
		if (!config->uw_cmddb)
		{
			ERROR (150, "Failed to allocate memory for UWORLD-CMD-DB\n", TRUE);
		}
		
		LOG (MAINLOG, "[%s:%d:%s()]:  Memory allocated for UWORLD-CMD-DB at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, config->uw_cmddb);
			
		strncpy(config->uw_cmddb, argv[1], strlen(argv[1]) + 5);

	}

	else if (!strcasecmp(argv[0], "SAVE-CYCLE"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  SAVE-CYCLE\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->save_cycle = atoi(argv[1]);
	}

	else if (!strcasecmp(argv[0], "GLINE-CYCLE"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  GLINE-CYCLE\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->gline_cycle = atoi(argv[1]);
	}
	
	else if (!strcasecmp(argv[0], "NICK-CYCLE"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  NICK-CYCLE\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->nick_cycle = atoi(argv[1]);
	}

	else if (!strcasecmp(argv[0], "GLINE-AUTO-LEN"))
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  GLINE-AUTO-LEN\n",
			__FILE__, __LINE__, __FUNCTION__);
			
		config->gline_auto_len = atoi(argv[1]);
	}

	else
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  Unknown config option [IGNORED] [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, argv[0]);
	}
		
	LOG (MAINLOG, "[%s:%d:%s()]:  Finished\n",
		__FILE__, __LINE__, __FUNCTION__);

	return 0;
}
