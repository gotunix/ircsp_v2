/* ---------------------------------------------------------------------------
 * Copyright (c) GOTUNIX Networks
 * Copyright (c) GOTUNIXCODE
 * Copyright (c) Justin Ovens
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ---------------------------------------------------------------------------
 */



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




void synch (void)
{
	Gline *glinep;
	NickJupe *njupep;

	ircsp_debug ("@(#) - (%s:%d) %s():  Adding GLINES\n",
		__FILE__, __LINE__, __FUNCTION__);

	ircsp_notice(
		"Synching GLINES!\n");
		
	for (glinep = glineHEAD; glinep; glinep = glinep->next)
	{
		ircsp_debug("@(#) - (%s:%d) %s():  Adding GLine [%s]\n",
			__FILE__, __LINE__, __FUNCTION__, glinep->mask);

		ssprintf (s, 
			"%s GL * +%s %ld %ld :%s\n",
			config->server_numeric, glinep->mask, 
			glinep->expires, time(NULL),
			glinep->reason);
	}
	
	
	ircsp_notice (
		"Loading NICK Jupes\n");
		
	for (njupep = nickjupeHEAD; njupep; njupep = njupep->next)
	{
		User *userp;
		Hostname *hostp;
		NickNumeric *nnumericp;

		char *numeric;
		numeric = (char *)ircsp_calloc(1, 10);
		getFreeNumeric(numeric);
		nnumericp = addNickNumeric(numeric);
		
		ssprintf(s, 
			"%s N %s 1 0 %s %s %s AAAAAA %s :%s\n",
			config->server_numeric, njupep->nickname,
			config->jupe_username, config->jupe_hostname,
			config->jupe_ircmodes, numeric, config->jupe_realname);


		hostp = findHostname(config->jupe_hostname);
		if (!hostp)
		{
			hostp = addHostname (config->jupe_hostname);
		}
		else
		{
			hostp->numHosts++;
		}

		userp = addUser(njupep->nickname, config->jupe_username,
			hostp, numeric, config->server_numeric, njupep->flags);
		
		SetNickJupe(userp);			
		ircsp_free (numeric);
	}
	
	
	
/*
	if (config->nserve_enable)
	{
		ssprintf(s, "%s WA :Loading Registered nickname DB\n",
			config->server_numeric);
		 
		loadNServiceDB ();
	} */ 
	
}
