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
// $Date: 2009/04/11 06:40:50 $
//------------------------------------------------------------------------





#if !defined (__CONFIG_H__)
#define __CONFIG_H__

#define __CONFIG_FILE__	"conf/ircsp.conf"
#define __CONFIG_PATH__ "conf"

typedef struct _Config
{
	int		protocol;
	char		*server;
	char		*server_comment;
	char		*server_numeric;

	char		*uplink;
	int		 uplink_port;
	char		*uplink_pass;

	char		*uworld_nickname;
	char		*uworld_username;
	char		*uworld_hostname;
	char		*uworld_realname;
	char		*uworld_ircmodes;
	int		uworld_enable;

	char		*cserve_nickname;
	char		*cserve_username;
	char		*cserve_hostname;
	char		*cserve_realname;
	char		*cserve_ircmodes;
	int		cserve_enable;

	char		*nserve_nickname;
	char		*nserve_username;
	char		*nserve_hostname;
	char		*nserve_realname;
	char		*nserve_ircmodes;
	int		nserve_enable;

	char		*operchannel_name;
	char		*operchannel_modes;
	int		operchannel_notice;

	char		*debugchannel_name;
	char		*debugchannel_modes;

	char		*deny_message;
	
	
	char		*jupe_username;
	char		*jupe_hostname;
	char		*jupe_realname;
	char		*jupe_ircmodes;
	char		*jupe_database;


	int		fork;
	int		xPos;
	int		yPos;
	int		yyPos;

	int		 clone_warn;
	int		 clone_ban;

	char		*admin_db;	// Services Admin DB
	char		*gline_db;	// GLine DB
	char		*uw_cmddb;	// UWorld Command Levels

	int		 save_cycle;
	int		 gline_cycle;
	int		nick_cycle;

	int		 gline_auto_len;
} Config;

Config *config;


// Prototypes

extern int ircsp_loadConfig (void);
extern int ircsp_parseConfig (int, char *[]);

// Defines

#define CHANLEN 1024
#define NICKLEN 15
#define USERLEN 10
#define HOSTLEN 256
#define SERVLEN 1024
#define KEYLEN 28
#define PASSLEN 20
#define COMMLEN 2096

#endif // __CONFIG_H__

