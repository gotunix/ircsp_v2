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
// $Revision: 1.23 $
// $Author: jovens $
// $Date: 2009/04/11 06:40:50 $
//------------------------------------------------------------------------





#if !defined (__IRCSP_H__)
#define __IRCSP_H__

#define		TRUE	1
#define		FALSE	0


typedef struct _service
{
	int		argc;
	char**		argv;
	pid_t		pid;
	uid_t		uid;
	uid_t		euid;
	time_t		uptime;
	int		connected;
	int		initial_burst;
	time_t		last_save_cycle;
	time_t		last_gline_cycle;
	time_t		last_nick_cycle;
	int		rehashing;
} Service;

Service *service;



typedef struct _status
{
	int		numUsers;
	int		numServers;
	int		numChannels;
	int		numOpers;
	int		numAdmins;
	int		numHosts;
	int		numGlines;
	int		numNumeric;
} Status;

Status *status;


// Prototypes

extern	void	ircsp_startup		(int, char *[]);
extern	void	ircsp_exit		(int, char *, char *);
extern	void	ircsp_panic		(char *);
extern	void	ircsp_login		(void);
extern	void	ircsp_spawnUW		(void);
extern	void	ircsp_spawnCS		(void);
extern	void	ircsp_spawnNS		(void);

extern	int	ircsp_signals		(void);
extern	void	ircsp_sighup		(int);
extern	void	ircsp_sigsegv		(int);
extern	void	ircsp_sigint		(int);
extern	void	ircsp_sigterm		(int);


#endif // __IRCSP_H__
