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
#include <string.h>
#include <vbbs/session.h>
#include <vbbs/log.h>
#include <vbbs/conn.h>
#include <vbbs/user.h>
#include <vbbs/terminal.h>

#define MAX_LOGIN_ATTEMPTS 3

/** Input handlers */
static void PromptUserName(Session *session);
static void PromptPassword(Session *session);
static void CheckPassword(Session *session);
static void LoggedIn(Session *session);

void InitSession(Session *session)
{
    session->conn = NULL;
    InitUser(&session->user);
    session->eventHandler = NULL;
    session->characterInputMode = FALSE;
    session->nextCharacter = 0;
    session->nextLine[0] = '\0';
    session->loginAttempts = 0;
}

static bool isNextLineReady(Session *session)
{
    return session->nextLine[0] != '\0';
}

void Connected(Session *session)
{
    Connection *conn;
    
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    session->loginAttempts = 0;
    conn->connectionStatus = CONNECTED;
    WriteToConnection(conn, "Connected to vBBS.\n");
    PromptUserName(session);
}

static void PromptUserName(Session *session)
{
    Connection *conn;
    
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    WriteToConnection(conn, RESET_MODES);
    WriteToConnection(conn, "Login => ");
    session->eventHandler = PromptPassword;
}

static void PromptPassword(Session *session)
{
    Connection *conn;
    int maxLength;
    
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    maxLength = sizeof(session->user.username) - 1;

    if (isNextLineReady(session))
    {
        strncpy(session->user.username, session->nextLine, maxLength);
        session->user.username[maxLength] = '\0';
        session->nextLine[0] = '\0';
        WriteToConnection(conn, SET_CONSEAL_OFF);
        WriteToConnection(conn, "Password => ");
        WriteToConnection(conn, SET_CONSEAL);
        session->eventHandler = CheckPassword;
    }
}

static void CheckPassword(Session *session)
{
    Connection *conn;
    User user;
    
    InitUser(&user);

    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    if (isNextLineReady(session))
    {

        InitUser(&user);
        /** TODO: Look up user by username in user database. */

        if (!AuthenticateUser(&user, session->user.username, session->nextLine))
        {
            Info("[%d] Authentication failure for user %s.", 
                conn->connectionID, session->user.username);
            WriteToConnection(conn, "Authentication failed.\n");
            session->nextLine[0] = '\0';
            session->loginAttempts++;
            if (session->loginAttempts >= MAX_LOGIN_ATTEMPTS)
            {
                WriteToConnection(conn, 
                    "Too many failed attempts. Disconnecting...\n");
                Info("[%d] Too many failed attempts. Disconnecting...", 
                    conn->connectionID);
                Disconnect(conn);
            }
            else
            {
                session->eventHandler = PromptUserName;
            }
        }
        else
        {
            conn->connectionStatus = AUTHENTICATED;
            Info("[%d] User %s logged in successfully.", 
                conn->connectionID, session->user.username);
            session->nextLine[0] = '\0';
            LoggedIn(session);
        }
    }
}

void LoggedIn(Session *session)
{
    Connection *conn;

    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    Disconnect(conn);
}
