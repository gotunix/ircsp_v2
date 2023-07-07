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
#include "log.h"


//@(#) - (s_parse.c:101) s_parse(): Read: ACAAA P AEA :hi

void m_privmsg (char *sBuf)
{
	LOG (MAINLOG, "[%s:%d:%s()]:  Starting PRIVMSG parser\n",
		__FILE__, __LINE__, __FUNCTION__);

	if (IsProtocolUndernet (config))
	{
		User *userp;
		User *myuserp;

		if (token(sBuf, 3)[0] == '#')
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Channel message ignored\n",
				__FILE__, __LINE__, __FUNCTION__);
			return ;
		}

		myuserp = findUserByNumeric(token(sBuf, 3));	
		if (!myuserp)
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Failed to find receiver by NUMERIC [%s]\n",
				__FILE__, __LINE__, __FUNCTION__, token(sBuf, 3));
			return ;
		}

		userp = findUserByNumeric(token(sBuf, 1));
		if (!userp)
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  Failed to find sender by NUMERIC [%s]\n",
				__FILE__, __LINE__, __FUNCTION__, token(sBuf, 1));
			return ;
		}

		if (IsNickJupe(myuserp))
		{
			NickJupe *njupep;
	
			njupep = findNickJupe(myuserp->nickname);
			
			if (IsJupeUW(njupep))
			{
				do_uworld(sBuf);
			}
			else
			{
				ssprintf (s,
					"%s O %s :%s\n",
					myuserp->numeric, userp->numeric, njupep->reason);
			}
		}

		if (!strcasecmp(myuserp->nickname, config->uworld_nickname))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  UWORLD PRIVMSG\n",
				__FILE__, __LINE__, __FUNCTION__);

			do_uworld(sBuf);
		}

		else if (!strcasecmp(myuserp->nickname, config->cserve_nickname))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  CSERVICE PRIVMSG\n",
				__FILE__, __LINE__, __FUNCTION__);
		} 

		else if (!strcasecmp(myuserp->nickname, config->nserve_nickname))
		{
			LOG (MAINLOG, "[%s:%d:%s()]:  NSERVICE PRIVMSG\n",
				__FILE__, __LINE__, __FUNCTION__);
		}
	}

	LOG (MAINLOG, "[%s:%d:%s()]:  Finished PRIVMSG parser\n",
		__FILE__, __LINE__, __FUNCTION__);
}
