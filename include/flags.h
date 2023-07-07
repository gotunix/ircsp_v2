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




#if !defined (__FLAGS_H__)
#define __FLAGS_H__

// Protocol Flags

#define		PROTOCOL_IS_UNDERNET	0x0001
#define		PROTOCOL_IS_DALNET	0x0002
#define		PROTOCOL_IS_EFNET	0x0004

// Nickname FLAGS

#define		NICK_IS_JUPE		0x0000000000001
#define		NICK_IS_OPER		0x0000000000002
#define		NICK_IS_ADMIN		0x0000000000004
#define		NICK_IS_SERVICE		0x0000000000008


// Server FLAGS

#define		SERVER_IN_BURST		0x0000000000001
#define		SERVER_IS_HUB		0x0000000000002
#define		SERVER_IS_MY_UPLINK	0x0000000000004
#define		SERVER_IS_JUPED		0x0000000000008


// Admin FLAGS

#define		ADMIN_LOGIN		0x0000000000001
#define		ADMIN_INMEM		0x0000000000002
#define		ADMIN_SUSPENDED		0x0000000000004


// Channel FLAGS

#define		CHANNEL_HAS_LIMIT	0x0000000000001
#define		CHANNEL_HAS_KEY		0x0000000000002
#define		CHANNEL_LOCKED		0x0000000000004


// NickList & ChanList FLAGS

#define		IS_CHANOP		0x0000000000001
#define		IS_CHANVOICE		0x0000000000002


// NickJUPE FLAGS

#define		JUPE_IS_CS		0x0000000000001
#define		JUPE_IS_NS		0x0000000000002
#define		JUPE_IS_UW		0x0000000000004

#endif // __FLAGS_H__
