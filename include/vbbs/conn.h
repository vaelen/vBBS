#ifndef VBBS_CONN_H
#define VBBS_CONN_H

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
#include <vbbs/buffer.h>
#include <vbbs/user.h>
#include <vbbs/terminal.h>

typedef enum
{
    CONSOLE,
    SERIAL,
    MODEM,
    TELNET
} ConnectionType;

typedef enum
{
    DISCONNECTED,
    CONNECTED,
    AUTHENTICATED
} ConnectionStatus;

typedef struct
{
    unsigned int connectionID;
    unsigned int connectionStatus;
    unsigned int connectionType;
    unsigned int connectionSpeed;
    char location[100];
    char address[100];
    FILE *inputStream;
    FILE *outputStream;
    void *data;
    Terminal terminal;
    InputBuffer *inputBuffer;
    Buffer *outputBuffer;
    bool inEscape;
    bool inCSI;
} Connection;

/* typedef void (*DisconnectFunction)(Connection *conn);*/

void InitConnection(Connection *conn);
bool AuthenticateConnection(Connection *conn, const char *username, 
    const char *password);
void WriteToConnection(Connection *conn, const char *format, ...);
int WriteBufferToConnection(Connection *conn);
void Disconnect(Connection *conn);
void DestroyConnection(Connection *conn);

#endif
