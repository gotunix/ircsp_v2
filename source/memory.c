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
// $Revision: 1.9 $
// $Author: jovens $
// $Date: 2009/04/11 07:09:02 $
//------------------------------------------------------------------------






#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "ircsp.h"
#include "debug.h"
#include "ircsp.h"
#include "memory.h"
#include "log.h"

long runtime = 0;
long allocated = 0;


void ircsp_initmemory (void)
{
	runtime = allocated;

	LOG (MAINLOG, "[%s:%d:%s()]:  Memory initialized\n",
		__FILE__, __LINE__, __FUNCTION__);
}


void ircsp_showleaks (void)
{
	if (runtime >= 0 && (allocated - runtime) > 0)
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  There were [%ld] bytes leaked on exit\n",
			__FILE__, __LINE__, __FUNCTION__, (allocated - runtime));
	}
	else
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  There were [0] bytes leaked on exit\n",
			__FILE__, __LINE__, __FUNCTION__);
	}
}


void *ircsp_malloc (long size)
{
	MemBlock *mb;

	if (size == 0)
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  Attempt to allocate [0 bytes]\n",
			__FILE__, __LINE__, __FUNCTION__);

		return NULL;
	}

	mb = malloc (size + sizeof(MemBlock));
	assert (mb != NULL);
	if (mb == NULL)
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  Unable to allocate [%ld] bytes\n",
			__FILE__, __LINE__, __FUNCTION__, size);

		return NULL;
	}

	mb->size = size;
	mb->sig = SIGNATURE;
	mb->data = (void *)((char *)(mb) + sizeof(MemBlock));
	allocated += size;

	LOG (MAINLOG, "[%s:%d:%s()]:  Allocated [%ld] bytes at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, mb->size, mb->data);

	return mb->data; 
}


void *ircsp_calloc (long num, long size)
{
	MemBlock *mb;

	if (num == 0 || size == 0)
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  Attempt to allocate [0 bytes]\n",
			__FILE__, __LINE__, __FUNCTION__);

		return NULL;
	}

	mb = malloc (num * size + sizeof(MemBlock));
	if (mb == NULL)
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  Unable to allocate [%ld] bytes\n",
			__FILE__, __LINE__, __FUNCTION__, size);

		return NULL;
	}

	mb->size = num * size;
	mb->sig = SIGNATURE;
	mb->data = (void *)((char *)(mb) + sizeof(MemBlock));
	memset (mb->data, 0, num * size);
	allocated += mb->size;

	LOG (MAINLOG, "[%s:%d:%s()]:  Allocated [%ld] bytes at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, mb->size, mb->data);

	return mb->data;
}


void ircsp_free (void *ptr)
{
	MemBlock *mb;

	if (ptr == NULL)
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  Attempt to free a NULL pointer\n",
			__FILE__, __LINE__, __FUNCTION__);

		return ;
	}

	mb = (MemBlock *)((char *)(ptr) - sizeof(MemBlock));
	if (mb->sig != SIGNATURE)
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  Attempt to free an invalid pointer at [%p]\n",
			__FILE__, __LINE__, __FUNCTION__, mb->data);
	}

	allocated -= mb->size;

	LOG (MAINLOG, "[%s:%d:%s()]:  Released [%ld] bytes at [%p]\n",
		__FILE__, __LINE__, __FUNCTION__, mb->size, mb->data);

	mb->sig = FREE_SIGNATURE;
	free ((mb)); 
}
