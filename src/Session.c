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

#include <vBBS/Session.h>

#define MAX_TRIES 3

bool Login(Connection *conn)
{
    char username[51];
    char password[51];
    int i;

    WriteToConnection(conn, RESET_MODES);
    WriteToConnection(conn, "Login => ");
    i = fscanf(conn->inputStream, "%50s", username);
    if (i == EOF)
    {
        Info("[%d] Connection closed while reading username.", 
            conn->connectionID);
        return FALSE;
    }
    else if (i == 0)
    {
        WriteToConnection(conn, "Invalid username.\n");
        Info("[%d] Invalid username: %s", conn->connectionID, username);
        return FALSE;
    }

    WriteToConnection(conn, "Password => ");
    WriteToConnection(conn, SET_CONSEAL);
    fflush(conn->outputStream);
    i = fscanf(conn->inputStream, "%50s", password);
    WriteToConnection(conn, SET_CONSEAL_OFF);
    fflush(conn->outputStream);
    if (i == EOF)
    {
        Info("[%d] Connection closed while reading password.", 
            conn->connectionID);
        return FALSE;
    }
    else if (i == 0)
    {
        WriteToConnection(conn, "Invalid username.\n");
        Info("[%d] Invalid password: %s", conn->connectionID, username);
        return FALSE;
    }

    if (!AuthenticateConnection(conn, username, password))
    {
        Info("[%d] Authentication failure for user %s.", 
            conn->connectionID, username);
        WriteToConnection(conn, "Authentication failed.\n");
        return FALSE;
    }

    Info("[%d] User %s logged in successfully.", 
        conn->connectionID, conn->user.username);
    return TRUE;
}

void Connected(Connection *conn)
{
    bool b = FALSE;
    int tries = 0;

    conn->connectionStatus = CONNECTED;
    WriteToConnection(conn, "Connected to vBBS.\n");
    NegotiateTerminal(conn->inputStream, conn->outputStream, &conn->terminal);
    while(tries < MAX_TRIES)
    {
        b = Login(conn);
        if (b)
        {
            conn->connectionStatus = AUTHENTICATED;
            WriteToConnection(conn, "Welcome, %s!\n", conn->user.username);
            break;
        }
        tries++;
        if (tries == MAX_TRIES)
        {
            WriteToConnection(conn, "Too many failed attempts. Disconnecting...\n");
            Info("[%d] Too many failed attempts. Disconnecting...", conn->connectionID);
            Disconnect(conn);
            return;
        }
    }
}

void NeoFetch(Connection *conn)
{
    char buffer[256];
#ifdef _POSIX_VERSION
    FILE *pipe;

    pipe = popen("neofetch", "r");
    if (pipe == NULL)
    {
        WriteToConnection(conn, "Failed to fetch system information.\n");
        return;
    }

    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        WriteToConnection(conn, "%s", buffer);
    }
    pclose(pipe);
    WriteToConnection(conn, "\n");
    WriteToConnection(conn, "Press Enter to continue...\n");
    fgets(buffer, sizeof(buffer), conn->inputStream);
    WriteToConnection(conn, "\n");
#else
    WriteToConnection(conn, "NeoFetch is not supported on this platform.\n");
    WriteToConnection(conn, "Press Enter to continue...\n");
    fgets(buffer, sizeof(buffer), conn->inputStream);
    WriteToConnection(conn, "\n");
#endif
}

