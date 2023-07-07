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
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "log.h"

void	LOG	(FILE *, char *, ...);
void	ERROR	(int, char *, int);


//------------------------------------------------------------------------
// LOG ()
//
//------------------------------------------------------------------------


void LOG (FILE *logfile, char *format, ...)
{
	char buf[256], entry[256];
	va_list msg;
	time_t logtime;

	va_start (msg, format);
	vsprintf (buf, format, msg);

	time (&logtime);
	sprintf (entry, "[%s] %s", lrange(ctime(&logtime), 1), buf);

	fprintf (logfile, "%s", entry);
	fflush (logfile);
}


//------------------------------------------------------------------------
// ERROR ()
//
//------------------------------------------------------------------------

void ERROR (int errNum, char *errMsg, int fatal)
{
	if (fatal)
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  FATAL ERROR(%d): %s", 
			__FILE__, __LINE__, __FUNCTION__, errNum, errMsg);
		ircsp_panic (errMsg);
	}
	else
	{
		LOG (MAINLOG, "[%s:%d:%s()]:  ERROR(%d): %s\n",
			__FILE__, __LINE__, __FUNCTION__, errNum, errMsg);
	}
}
