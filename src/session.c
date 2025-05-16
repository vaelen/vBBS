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


Session* NewSession(Connection *conn)
{
    Session *session = (Session *)malloc(sizeof(Session));
    if (session == NULL)
    {
        Error("Failed to allocate memory for session.");
        return NULL;
    }
    session->conn = conn;
    InitUser(&session->user);
    session->eventHandler = NULL;
    session->loginAttempts = 0;
    return session;
}

void DestroySession(Session *session)
{
    if (session == NULL)
    {
        return;
    }

    if (session->eventHandler != NULL)
    {
        session->eventHandler = NULL;
    }

    if (session->conn != NULL)
    {
        Disconnect(session->conn);
        DestroyConnection(session->conn);
        session->conn = NULL;
    }

    free(session);
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
    session->eventHandler = PromptUserName;
    PromptUserName(session);
}

void PromptUserName(Session *session)
{
    Connection *conn;
    
    if (session == NULL || session->conn == NULL)
    {
        Error("Session is NULL or connection is NULL.");
        return;
    }
    conn = session->conn;

    WriteToConnection(conn, RESET_MODES);
    WriteToConnection(conn, "Login => ");
    session->eventHandler = PromptPassword;
}

void PromptPassword(Session *session)
{
    Connection *conn;
    int maxLength;
    
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    maxLength = sizeof(session->user.username) - 1;

    if (IsNextLineReady(conn->inputBuffer))
    {
        strncpy(session->user.username, conn->inputBuffer->nextLine, maxLength);
        session->user.username[maxLength] = '\0';
        ClearNextLine(conn->inputBuffer);;
        WriteToConnection(conn, SET_CONCEAL_OFF);
        WriteToConnection(conn, "Password => ");
        WriteToConnection(conn, SET_CONCEAL);
        session->eventHandler = CheckPassword;
    }
}

void CheckPassword(Session *session)
{
    Connection *conn;
    User user;
    
    InitUser(&user);

    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    if (IsNextLineReady(conn->inputBuffer))
    {

        InitUser(&user);
        /** TODO: Look up user by username in user database. */

        if (!AuthenticateUser(&user, session->user.username, 
            conn->inputBuffer->nextLine))
        {
            Info("[%d] Authentication failure for user %s.", 
                conn->connectionID, session->user.username);
            WriteToConnection(conn, "Authentication failed.\n");
            ClearNextLine(conn->inputBuffer);
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
                PromptUserName(session);
            }
        }
        else
        {
            conn->connectionStatus = AUTHENTICATED;
            Info("[%d] User %s logged in successfully.", 
                conn->connectionID, session->user.username);
            ClearNextLine(conn->inputBuffer);
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
