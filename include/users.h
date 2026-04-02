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
// $Revision: 1.12 $
// $Author: jovens $
// $Date: 2009/04/11 06:40:50 $
//------------------------------------------------------------------------






#if !defined (__USERS_H__)
#define __USERS_H__

typedef struct _user
{
	char		*nickname;
	char		*username;
	struct	_hostname		*hostp;
	char		*server;
	char		*numeric;
	int		 flags;
	time_t	 stamp;
	time_t	 lastsent;
	int		 numChans;
	int		 flood;

	int	nservice_kill;
	struct _chanlist *chanhead;
	struct _user *next;
	struct _admin *adminp;
} User;


User *userHEAD;


typedef struct _chanlist
{
	struct  _channel *channelp;
	int	flags;
	struct _chanlist *next;
} ChanList;


typedef struct _hostname
{
	char *hostname;
	int  numHosts;

	struct _hostname *next;
} Hostname;

Hostname *hostHEAD;


// Structures

typedef struct _admin
{
	char		*nickname;
	char		*hostmask;
	char		*password;
	int		 level;
	int		 flags;
	time_t	 lastseen;
	int		 suspended;
	int		 numHosts;

	struct	 _user *userp;
	struct	 _admin *next;
} Admin;

Admin *adminHEAD;

// Prototypes

extern	User		*addUser		(char *, char *, struct _hostname *, char *, char *, time_t);
extern	User		*findUser		(char *);
extern	User		*findUserByNumeric	(char *);
extern	void		delUser		(char *);
extern	void		delUserByServer	(char *);

extern	ChanList	*addNickChannel		(User *, struct _channel *);
extern	ChanList	*findNickChannel	(User *, struct _channel *);
extern	void		delNickChannel		(User *, struct _channel *);

extern	Hostname	*addHostname		(char *);
extern	Hostname	*findHostname		(char *);
extern	void		delHostname		(char *);

extern	int		loadAdminDB		(void);


extern	Admin		*addAdmin		(char *, int, char *, int, time_t, int, char *);
extern	Admin		*findAdmin		(char *);
extern	void		delAdmin		(char *);




#endif // __USERS_H__
