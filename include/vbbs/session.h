#ifndef VBBS_SESSION_H
#define VBBS_SESSION_H

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
#include <vbbs/user.h>
#include <vbbs/terminal.h>
#include <vbbs/conn.h>

typedef struct Session Session;

typedef void (*EventHandler)(Session *session);

struct Session {
   Connection *conn;
   uint32_t sessionID;
   User *user;
   EventHandler eventHandler;
   EventHandler nextEventHandler;
   uint8_t loginAttempts;
   char tempBuffer[256];
   bool isNewUser;
};

Session* NewSession(Connection *conn);
void DestroySession(Session *session);
void Connected(Session *session);
void SetSessionWindowSize(void *userData, int width, int height);
void SetSessionTerminalType(void *userData, const char *type);
void SetSessionConnectionSpeed(void *userData, const char *speed);
void EchoCharToSession(void *userData, const char c);

#endif
