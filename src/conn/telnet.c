/*
Copyright (c) 2025, Andrew Young

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 
1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <vbbs/types.h>

#include <stdio.h>
#include <vbbs/log.h>
#include <vbbs/conn.h>
#include <vbbs/conn/telnet.h>

/***** UNIX Implementation Using Berkeley Sockets *****/

#ifdef _POSIX_VERSION

#include "telnet/unix.c"

#else
/***** Default Null Implementation *****/

TelnetListener* NewTelnetListener(int port)
{
    Warn("TelnetListener: Not implemented on this platform.");
    return NULL;
}

Connection* TelnetListenerAccept(TelnetListener *listener)
{
    Warn("TelnetListenerAccept: Not implemented on this platform.");
    return NULL;
}

void CloseTelnetListener(TelnetListener *listener) 
{
    Warn("CloseTelnetListener: Not implemented on this platform.");
}

void DisconnectTelnet(Connection *conn, bool closeImmediately)
{
    Warn("DisconnectTelnet: Not implemented on this platform.");
}

void DestroyTelnetConnection(Connection *conn)
{
    Warn("DestroyTelnetConnection: Not implemented on this platform.");
}

const char* TelnetRemoteAddress(Connection *conn)
{
    Warn("TelnetRemoteAddress: Not implemented on this platform.");
    return "Unknown";
}

int TelnetRemotePort(Connection *conn)
{
    Warn("TelnetRemotePort: Not implemented on this platform.");
    return -1;
}

#endif

