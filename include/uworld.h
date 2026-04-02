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
// $Revision: 1.46 $
// $Author: jovens $
// $Date: 2009/04/11 06:40:50 $
//------------------------------------------------------------------------






#if !defined (__UWORLD_H__)
#define __UWORLD_H__

#define UWORLD_VERSION	"1.1.3 - PRERELEASE"

#define UW_NUM_CMDS 35


struct uworld_comtab
{
	char	command[20];
	int (*func)(int, char *[]);
	int minlevel;
	int minargs;
	int disabled;
	char *reason;
};


typedef struct _gline
{
	char		*mask;
	char		*reason;
	char		*setby;
	time_t	 setwhen;
	time_t	 expires;

	struct	_gline *next;
} Gline;

Gline *glineHEAD;

typedef struct _nickjupe
{
	char	*nickname;
	char	*reason;
	
	int	flags;
	
	struct _nickjupe *next;
} NickJupe;

NickJupe *nickjupeHEAD;


// Prototypes

extern	Gline		*addGline		(char *, time_t, time_t, char *, char *);
extern	Gline		*findGline		(char *);
extern	void		delGline		(char *);

extern	NickJupe	*addNickJupe		(char *, char *, int);
extern	NickJupe	*findNickJupe		(char *);
extern	void		delNickJupe		(char *);


extern	void		do_uworld		(char *);
extern	int		loadUWCmd		(void);
extern	int		writeUWCmd		(void);

extern	int		uw_loadCommand		(void);
extern	int		uw_saveCommand		(void);
extern	int		uw_loadGlineDB		(void);
extern	int		uw_saveGlineDB		(void);
extern	int		uw_loadNickJupe		(void);
extern	int		uw_saveNickJupe		(void);

// Level 0000 Commands

extern	int		uw_version		(int, char *[]);
extern	int		uw_help			(int, char *[]);
extern	int		uw_operlist		(int, char *[]);
extern	int		uw_verify		(int, char *[]);

// Level 0001 Commands

extern	int		uw_servlist		(int, char *[]);
extern	int		uw_nicklist		(int, char *[]);
extern	int		uw_hostlist		(int, char *[]);
extern	int		uw_adminlist		(int, char *[]);
extern	int		uw_glinelist		(int, char *[]);
extern	int		uw_chanlist		(int, char *[]);
extern	int		uw_uptime		(int, char *[]);
extern	int		uw_auth			(int, char *[]);
extern	int		uw_deauth		(int, char *[]);
extern	int		uw_banlist		(int, char *[]);
extern	int		uw_gline		(int, char *[]);
extern	int		uw_remgline		(int, char *[]);
extern	int		uw_whoison		(int, char *[]);
extern	int		uw_xlate		(int, char *[]);
extern	int		uw_whois		(int, char *[]);

// Level 0100 Commands

extern	int		uw_clearbans		(int, char *[]);
extern	int		uw_opermsg		(int, char *[]);
extern	int		uw_clearops		(int, char *[]);
extern	int		uw_opcom		(int, char *[]);
extern	int		uw_clearmodes		(int, char *[]);
extern	int		uw_scan			(int, char *[]);
extern	int		uw_mode			(int, char *[]);

// Level 0900 Commands

extern	int	uw_masskill		(int, char *[]);

// Level 1000 Commands


extern	int		uw_disablecmd		(int, char *[]);
extern	int		uw_enablecmd		(int, char *[]);
extern	int		uw_cmdlist		(int, char *[]);
extern	int		uw_save			(int, char *[]);
extern	int		uw_die			(int, char *[]);
extern	int		uw_restart		(int, char *[]);
extern	int		uw_rehash		(int, char *[]);
extern	int		uw_chlevel		(int, char *[]);

#endif // __UWORLD_H__
