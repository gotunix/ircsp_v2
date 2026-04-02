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
// $Revision: 1.16 $
// $Author: jovens $
// $Date: 2009/04/11 06:40:50 $
//------------------------------------------------------------------------







#if !defined (__MACROS_H__)
#define __MACROS_H__

// Config Macros

#define		IsProtocolUndernet(x)		((x)->protocol & PROTOCOL_IS_UNDERNET)
#define		SetProtocolUndernet(x)		((x)->protocol |= PROTOCOL_IS_UNDERNET)

#define		IsProtocolDalnet(x)		((x)->protocol & PROTOCOL_IS_DALNET)
#define		SetProtocolDalnet(x)		((x)->protocol |= PROTOCOL_IS_DALNET)

#define		IsProtocolEfnet(x)		((x)->protocol & PROTOCOL_IS_EFNET)
#define		SetProtocolEfnet(x)		((x)->protocol |= PROTOCOL_IS_EFNET)



// Nickname

#define IsOperator(x)				((x)->flags & NICK_IS_OPER)
#define SetOperator(x)				((x)->flags |= NICK_IS_OPER)
#define DelOperator(x)				((x)->flags &= ~NICK_IS_OPER)

#define SetService(x)				((x)->flags |= NICK_IS_SERVICE)
#define IsService(x)				((x)->flags & NICK_IS_SERVICE)
#define DelService(x)				((x)->flags &= ~NICK_IS_SERVICE)

#define SetAdministrator(x)			((x)->flags |= NICK_IS_ADMIN)
#define IsAdministrator(x)			((x)->flags & NICK_IS_ADMIN)
#define DelAdministrator(x)			((x)->flags &= ~NICK_IS_ADMIN)


#define SetNickJupe(x)				((x)->flags |= NICK_IS_JUPE)
#define IsNickJupe(x)				((x)->flags & NICK_IS_JUPE)
#define DelNickJupe(x)				((x)->flags &= ~NICK_IS_JUPE)
// Admin

#define SetAdminSuspended(x)			((x)->flags |= ADMIN_SUSPENDED)
#define IsAdminSuspended(x)			((x)->flags & ADMIN_SUSPENDED)
#define DelAdminSuspended(x)			((x)->flags &= ~ADMIN_SUSPENDED)

#define SetAdminLogin(x)			((x)->flags |= ADMIN_LOGIN)
#define IsAdminLogin(x)				((x)->flags & ADMIN_LOGIN)
#define DelAdminLogin(x)			((x)->flags &= ~ADMIN_LOGIN)


// Channel

#define SetChannelOp(x)				((x)->flags |= IS_CHANOP)
#define IsChannelOp(x)				((x)->flags & IS_CHANOP)
#define DelChannelOp(x)				((x)->flags &= ~IS_CHANOP)

#define SetChannelVoice(x)			((x)->flags |= IS_CHANVOICE)
#define IsChannelVoice(x)			((x)->flags & IS_CHANVOICE)
#define DelChannelVoice(x)			((x)->flags &= ~IS_CHANVOICE)

// Nick Service

#define	SetNServAuth(x)				((x)->flags |= NICK_IS_REGISTERED)
#define IsNServAuth(x)				((x)->flags & NICK_IS_REGISTERED)
#define DelNServAuth(x)				((x)->flags &= ~NICK_IS_REGISTERED)


// Jupe Flags


#define		IsJupeCS(x)			((x)->flags & JUPE_IS_CS)
#define		IsJupeNS(x)			((x)->flags & JUPE_IS_NS)
#define		IsJupeUW(x)			((x)->flags & JUPE_IS_UW)


#endif // __MACROS_H__
