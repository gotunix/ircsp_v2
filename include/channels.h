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
// $Revision: 1.15 $
// $Author: jovens $
// $Date: 2009/04/11 06:40:50 $
//------------------------------------------------------------------------






#if !defined (CHANNELS_H__)
#define __CHANNELS_H__

// Structures

typedef struct _channel
{
	char		*name;
	char		*key;

	int		 flags;
	int		 limit;
	time_t	 created;

	int		 numBans;
	int		 numUsers;

	struct _ban *banhead;
	struct _nicklist *nickhead;
	struct _channel *next;
} Channel;



typedef struct _nicklist
{
	struct 	_user *userp;
	int	flags;
	struct _nicklist *next;
} NickList;


typedef struct _ban
{
		char		*mask;
		struct	_ban	*next;
} Ban;

Channel *channelHEAD;


// Prototypes

extern	Channel		*addChannel		(char *, int, time_t, char *, int);
extern	Channel		*findChannel		(char *);
extern	void		delChannel		(char *);


extern	NickList *addChannelNick (struct _channel *channelp, struct _user *userp);
extern	NickList *findChannelNick (struct _channel *channelp, struct _user *userp);
extern	void delChannelNick (struct _channel *channelp, struct _user *userp);



extern	Ban		*addChannelBan		(Channel *, char *);
extern	Ban		*findChannelBan		(Channel *, char *);
extern	void		delChannelBan		(Channel *, char *);


#endif // __CHANNELS_H__
