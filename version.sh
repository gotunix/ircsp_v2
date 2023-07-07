#!/bin/sh
#
# Build the version.c file which contains all the version related info and
# needs to be updated on a per-build basis.

# $PROGRAM is the string returned as the first part of a /VERSION reply,
# and must not contain spaces.  It is not used anywhere else.
PROGRAM=IRCSP
CODENAME=Pheonix
VERSION=1.0.02

# Increment Services build number
if [ -f source/version.c ]; then
    BUILD=`fgrep '#define BUILD' source/version.c | sed 's/^#define BUILD.*"\([0-9]*\)".*$/\1/'`
    BUILD=`expr $BUILD + 1 2>/dev/null`
else
    BUILD=1
fi
if [ ! "$BUILD" ]; then
    BUILD=1
fi

DATE=`date`
if [ $? -ne "0" ]; then
    DATE="\" __DATE__ \" \" __TIME__ \""
fi

cat >source/version.c <<EOF
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




#include "version.h"

#define BUILD   "$BUILD"

const char program_name[] = "$PROGRAM";
const char code_name[] = "$CODENAME";
const char version_number[] = "$VERSION";
const char version_build[] = "build #" BUILD ", compiled $DATE";


/* Look folks, please leave this INFO reply intact and unchanged.  If you do
 * have the urge to mention yourself, please simply add your name to the list.
 * The other people listed below have just as much right, if not more, to be
 * mentioned.  Leave everything else untouched.  Thanks.
 */

const char *info_text[] = {
	"IRCSP [Internet Relay Chat Service Package]",
	"Copyright (C) 1998 - 2009 GOTUNIX Networks",
	"           All rights reserved.",
	" ",
	"IRCSP is developed by Justin Ovens <jovens@gotunix.net>.",
	"Parts written by Anthony Soriano and others.",
	"IRCSP may be freely redistributed under the GNU",
	"General Public License, version 2 or later.",
	" ",
	"Many people have contributed to the ongoing development of",
	"IRCSP Particularly noteworthy contributors include:",
	"    Matt Holmes",
	"    Jim Johnson",
	"    Chris Olsen",
	"    Carsten Haitzler",
	"A full list of contributors and their contributions can be",
	"found in the AUTHORS file included in the IRCSP",
	"distribution archive.  Many thanks to all of them!",
	" ",
	"For more information and a list of distribution sites,",
	"please visit: http://justinovens.info/?Project=IRCSP",
	0
};
EOF
