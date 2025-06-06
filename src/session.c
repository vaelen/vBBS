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

#include <vbbs/session.h>
#include <vbbs/log.h>
#include <vbbs/conn.h>
#include <vbbs/user.h>
#include <vbbs/terminal.h>
#include <vbbs/time.h>
#include <vbbs/db.h>
#include <vbbs/db/user.h>

#include <vbbs/conn/telnet.h>
#include <vbbs/conn/console.h>
#include <vbbs/conn/serial.h>
#include <vbbs/conn/modem.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LOGIN_ATTEMPTS 3

static uint32_t sessionIDCounter = 0;

/** Input handlers */
void IdentifyTerminal(Session *session);
void CheckTerminalIdentity(Session *session);
void PromptUserName(Session *session);
void PromptPassword(Session *session);
void CheckPassword(Session *session);
void LoggedIn(Session *session);
void Logout(Session *session);
void NewUserPromptUserName(Session *session);
void NewUserCheckUserName(Session *session);
void NewUserPromptPassword(Session *session);
void NewUserPromptPasswordConfirm(Session *session);
void NewUserCheckPassword(Session *session);
void NewUserPromptEmail(Session *session);
void NewUserSubmit(Session *session);
void ListUsers(Session *session);
void ShowMainMenu(Session *session);
void MainMenuSelection(Session *session);

Session* NewSession(Connection *conn)
{
    Session *session = (Session *)malloc(sizeof(Session));
    if (session == NULL)
    {
        Error("Failed to allocate memory for session.");
        return NULL;
    }
    session->sessionID = ++sessionIDCounter;
    session->conn = conn;
    session->user = NULL;
    session->eventHandler = NULL;
    session->loginAttempts = 0;
    session->isNewUser = FALSE;
    memset(session->tempBuffer, 0, sizeof(session->tempBuffer));

    if (conn != NULL)
    {
        switch(conn->connectionType)
        {
            case CONSOLE:
                session->eventHandler = PromptUserName;
                Info("[%d] New console connection", 
                    session->sessionID);
                break;
            case SERIAL:
                session->eventHandler = PromptUserName;
                Info("[%d] New serial connection from %s at %dbps", 
                    session->sessionID,
                    conn->location,
                    conn->connectionSpeed);
                break;
            case MODEM:
                session->eventHandler = PromptUserName;
                Info("[%d] New modem connection from %s at %dbps", 
                    session->sessionID,
                    conn->location,
                    conn->connectionSpeed);
                break;
            case TELNET:
                Info("[%d] New telnet connection from %s:%d", 
                    session->sessionID,
                    TelnetRemoteAddress(session->conn),
                    TelnetRemotePort(session->conn));
                break;
        }
    }

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
        switch(session->conn->connectionType)
        {
            case CONSOLE:
                session->eventHandler = PromptUserName;
                Info("[%d] Console connection closed", 
                    session->sessionID);
                break;
            case SERIAL:
                session->eventHandler = PromptUserName;
                Info("[%d] Serial connection closed", 
                    session->sessionID);
                break;
            case MODEM:
                session->eventHandler = PromptUserName;
                Info("[%d] Modem connection closed", 
                    session->sessionID);
                break;
            case TELNET:
                Info("[%d] Telnet connection from %s:%d closed", 
                    session->sessionID,
                    TelnetRemoteAddress(session->conn),
                    TelnetRemotePort(session->conn));
                break;
        }
        DestroyConnection(session->conn);
        session->conn = NULL;
    }

    if (session->isNewUser && session->user != NULL)
    {
        /* We have to clean up this memory, because it hasn't been
            added to ther UserDB yet and thus we own it. */
        DestroyUser(session->user);
        session->user = NULL;
    }

    free(session);
}

void CheckTerminalIdentity(Session *session)
{
    Connection *conn;

    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    CheckIdentifyResponse(conn->outputBuffer, 
        conn->inputBuffer->nextLine, &conn->terminal);

    ClearNextLine(conn->inputBuffer);

    session->eventHandler = PromptUserName;
    PromptUserName(session);
}

void IdentifyTerminal(Session *session)
{ 
    Connection *conn;

    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    conn->inputBuffer->buffer->userData = session;
    conn->inputBuffer->buffer->windowSize = SetSessionWindowSize;
    conn->inputBuffer->buffer->terminalType = SetSessionTerminalType;
    conn->inputBuffer->buffer->connectionSpeed = SetSessionConnectionSpeed;
    conn->inputBuffer->buffer->echo = EchoCharToSession;

    Identify(conn->outputBuffer);
    session->eventHandler = CheckTerminalIdentity;
}

void SetSessionWindowSize(void *userData, int width, int height)
{
    Session *session = (Session *)userData;
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    session->conn->terminal.width = width;
    session->conn->terminal.height = height;
    Info("[%d] Terminal window size set to %dx%d", 
        session->sessionID, width, height);
}

void SetSessionTerminalType(void *userData, const char *type)
{
    Session *session = (Session *)userData;
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    strncpy(session->conn->terminal.type, type, 
        sizeof(session->conn->terminal.type));
    Info("[%d] Terminal type set to %s", 
        session->sessionID, session->conn->terminal.type);
}

void SetSessionConnectionSpeed(void *userData, const char *speed)
{
    Session *session = (Session *)userData;
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    session->conn->connectionSpeed = (unsigned int)atoi(speed);
    Info("[%d] Connection speed set to %d bps: %s", 
        session->sessionID, session->conn->connectionSpeed, speed);
}

void EchoCharToSession(void *userData, const char c)
{
    Session *session = (Session *)userData;
    if (session == NULL || session->conn == NULL)
    {
        return;
    }

    if (c == '\r')
    {
        WriteCharToConnection(session->conn, '\r');
        WriteCharToConnection(session->conn, '\n');
    }
    else if (c == '\n')
    {
        /* NOOP */
    }
    else
    {
        WriteCharToConnection(session->conn, c);
    }
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
    WriteToConnection(conn, "Connected to %s.\n", VBBS_VERSION_STRING);

    session->eventHandler = IdentifyTerminal;
    IdentifyTerminal(session);
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
    conn->inputBuffer->buffer->echoMode = ECHO_ON;
    WriteToConnection(conn, "Username (or 'new' to sign up) => ");
    session->eventHandler = PromptPassword;
}

void PromptPassword(Session *session)
{
    Connection *conn;
    int maxLength;
    User *user;
    
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    maxLength = sizeof(user->username) - 1;

    if (IsNextLineReady(conn->inputBuffer))
    {
        /* CleanString(conn->inputBuffer->nextLine); */
        if (strlen(conn->inputBuffer->nextLine) == 0)
        {
            ClearNextLine(conn->inputBuffer);
            PromptUserName(session);
            return;
        }
        if (strcmp(conn->inputBuffer->nextLine, "new") == 0)
        {
            WriteToConnection(conn, "New user registration.\n");
            ClearNextLine(conn->inputBuffer);
            session->user = NewUser();
            if (session->user == NULL)
            {
                Error("Failed to create new user.");
                WriteToConnection(conn, "System error.\n");
                Disconnect(conn, FALSE);
                return;
            }
            session->isNewUser = TRUE;
            NewUserPromptUserName(session);
            return;
        }
        strncpy(session->tempBuffer, conn->inputBuffer->nextLine, maxLength);
        session->tempBuffer[maxLength] = '\0';
        ClearNextLine(conn->inputBuffer);;
        conn->inputBuffer->buffer->echoMode = ECHO_ON;
        WriteToConnection(conn, "Password => ");
        conn->inputBuffer->buffer->echoMode = ECHO_PASSWORD;
        session->eventHandler = CheckPassword;
    }
}

void CheckPassword(Session *session)
{
    Connection *conn;
    User *user;
    
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    if (IsNextLineReady(conn->inputBuffer))
    {

        /** TODO: Look up user by username in user database. */
        user = GetUserByUsername(session->tempBuffer);
        conn->inputBuffer->buffer->echoMode = ECHO_ON;
        if (user == NULL || !AuthenticateUser(user, session->tempBuffer, 
            conn->inputBuffer->nextLine))
        {
            Info("[%d] Authentication failure for user %s.", 
                session->sessionID, session->tempBuffer);
            WriteToConnection(conn, "Authentication failed.\n");
            ClearNextLine(conn->inputBuffer);
            session->loginAttempts++;
            if (session->loginAttempts >= MAX_LOGIN_ATTEMPTS)
            {
                WriteToConnection(conn, 
                    "Too many failed attempts. Disconnecting...\n");
                Info("[%d] Too many failed attempts. Disconnecting...", 
                    session->sessionID);
                Disconnect(conn, FALSE);
            }
            else
            {
                PromptUserName(session);
            }
        }
        else
        {
            session->user = user;
            conn->connectionStatus = AUTHENTICATED;
            Info("[%d] User %s logged in successfully.", 
                session->sessionID, session->user->username);
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

    session->user->lastSeen = time(NULL);
    SaveUserDB();

    conn->inputBuffer->buffer->echoMode = ECHO_ON;
    WriteToConnection(conn, RESET_MODES);
    WriteToConnection(conn, "Welcome %s!\n", session->user->username);
    WriteToConnection(conn, "You are now logged in.\n");

    ShowMainMenu(session);
}

void Logout(Session *session)
{
    Connection *conn;
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;
    conn->inputBuffer->buffer->echoMode = ECHO_ON;
    WriteToConnection(conn, RESET_MODES);
    WriteToConnection(conn, "Goodbye!\n");
    session->eventHandler = NULL;
    Disconnect(conn, FALSE);
}

void NewUserPromptUserName(Session *session)
{
    Connection *conn;
    
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    WriteToConnection(conn, "Enter your username: ");
    session->eventHandler = NewUserCheckUserName;
}

void NewUserCheckUserName(Session *session)
{
    Connection *conn;
    int maxLength;
    
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    maxLength = sizeof(session->tempBuffer) - 1;

    if (IsNextLineReady(conn->inputBuffer))
    {
        strncpy(session->tempBuffer, conn->inputBuffer->nextLine, maxLength);
        session->tempBuffer[maxLength] = '\0';
        ClearNextLine(conn->inputBuffer);

        if (GetUserByUsername(session->tempBuffer) != NULL)
        {
            WriteToConnection(conn, "Username already exists.\n");
            NewUserPromptUserName(session);
            return;
        }

        strncpy(session->user->username, session->tempBuffer, 
            sizeof(session->user->username));
        session->user->username[sizeof(session->user->username) - 1] = '\0';

        NewUserPromptPassword(session);
    }
}

void NewUserPromptPassword(Session *session)
{
    Connection *conn;
    
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    conn->inputBuffer->buffer->echoMode = ECHO_ON;
    WriteToConnection(conn, "Enter a password: ");
    conn->inputBuffer->buffer->echoMode = ECHO_PASSWORD;
    session->eventHandler = NewUserPromptPasswordConfirm;
}

void NewUserPromptPasswordConfirm(Session *session)
{
    Connection *conn;
    int maxLength;
    
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    maxLength = sizeof(session->tempBuffer) - 1;

    if (IsNextLineReady(conn->inputBuffer))
    {
        strncpy(session->tempBuffer, conn->inputBuffer->nextLine, maxLength);
        session->tempBuffer[maxLength] = '\0';
        ClearNextLine(conn->inputBuffer);
        conn->inputBuffer->buffer->echoMode = ECHO_ON;
        WriteToConnection(conn, "Confirm your password: ");
        conn->inputBuffer->buffer->echoMode = ECHO_PASSWORD;
        session->eventHandler = NewUserCheckPassword;
    }
}

void NewUserCheckPassword(Session *session)
{
    Connection *conn;
    
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    if (IsNextLineReady(conn->inputBuffer))
    {
        conn->inputBuffer->buffer->echoMode = ECHO_ON;
        if (strcmp(session->tempBuffer, conn->inputBuffer->nextLine) != 0)
        {
            WriteToConnection(conn, "Passwords do not match.\n");
            ClearNextLine(conn->inputBuffer);
            NewUserPromptPassword(session);
            return;
        }
        ClearNextLine(conn->inputBuffer);
        ChangePassword(session->user, session->tempBuffer);
        Debug("New user password set for %s: %s.", 
            session->user->username, session->user->pwHash);
        NewUserPromptEmail(session);
    }
}

void NewUserPromptEmail(Session *session)
{
    Connection *conn;
    
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    WriteToConnection(conn, "Enter your email address: ");
    session->eventHandler = NewUserSubmit;
}

void NewUserSubmit(Session *session)
{
    Connection *conn;
    
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    if (IsNextLineReady(conn->inputBuffer))
    {
        strncpy(session->user->email, conn->inputBuffer->nextLine, 
            sizeof(session->user->email));
        session->user->email[sizeof(session->user->email) - 1] = '\0';
        session->user->userType = REGULAR_USER;

        AddUser(session->user);
        session->isNewUser = FALSE;
        SaveUserDB();
        WriteToConnection(conn, "New user %s created successfully.\n", 
            session->user->username);

        conn->connectionStatus = AUTHENTICATED;
        
        ClearNextLine(conn->inputBuffer);
        LoggedIn(session);
    }
}

void ListUsers(Session *session)
{
    Connection *conn;
    User *user;
    int i;
    char *userListFormat;
    char lastSeen[21];
    
    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    userListFormat = "%18s %40s %20s\n";

    WriteToConnection(conn, "User List:\n");
    WriteToConnection(conn, "------------------ ");
    WriteToConnection(conn, "---------------------------------------- ");
    WriteToConnection(conn, "--------------------\n");
    WriteToConnection(conn, userListFormat, "Username", "Email", "Last Seen");
    for (i = 0; i < userDB->users->size; i++)
    {
        user = (User *)GetFromArrayList(userDB->users, i);
        if (user != NULL)
        {
            FormatTime(lastSeen, sizeof(lastSeen), user->lastSeen);
            WriteToConnection(conn, userListFormat, user->username, 
                user->email, lastSeen);
            Debug("User: %s, email: %s", 
                user->username, user->email);
        }
    }
    WriteToConnection(conn, "Press any key to continue...\n");
    SetInputMode(conn->inputBuffer, CHARACTER_INPUT_MODE);
    ClearNextLine(conn->inputBuffer);
    session->eventHandler = session->nextEventHandler;
}

void ShowMainMenu(Session *session)
{
    Connection *conn;

    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    WriteToConnection(conn, RESET_MODES);
    WriteToConnection(conn, "Main Menu:\n");
    WriteToConnection(conn, "1. List Users\n");
    WriteToConnection(conn, "2. Logout\n");
    WriteToConnection(conn, "Choose an option: ");
    
    session->eventHandler = MainMenuSelection;
    SetInputMode(conn->inputBuffer, CHARACTER_INPUT_MODE);
}

void MainMenuSelection(Session *session)
{
    Connection *conn;
    char choice[3];

    if (session == NULL || session->conn == NULL)
    {
        return;
    }
    conn = session->conn;

    if (IsNextLineReady(conn->inputBuffer))
    {
        strncpy(choice, conn->inputBuffer->nextLine, sizeof(choice) - 1);
        choice[sizeof(choice) - 1] = '\0';
        ClearNextLine(conn->inputBuffer);

        if (strcmp(choice, "1") == 0)
        {
            session->nextEventHandler = ShowMainMenu;
            ListUsers(session);
        }
        else if (strcmp(choice, "2") == 0)
        {
            Logout(session);
        }
        else
        {
            WriteToConnection(conn, "Invalid option. Please try again.\n");
            ShowMainMenu(session);
        }
    }
}
