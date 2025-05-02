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

#include <vBBS/Types.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <vBBS/Log.h>
#include <vBBS/Connection.h>
#include <vBBS/ConsoleConnection.h>
#include <vBBS/SerialConnection.h>
#include <vBBS/ModemConnection.h>
#include <vBBS/TelnetConnection.h>

void InitConnection(Connection *conn)
{
    conn->connectionID = 0;
    conn->connectionStatus = DISCONNECTED;
    conn->connectionType = CONSOLE;
    conn->connectionSpeed = 9600;
    strcpy(conn->location, "Unknown");
    strcpy(conn->address, "Unknown");
    conn->inputStream = stdin;
    conn->outputStream = stdout;
    InitUser(&conn->user);
    InitTerminal(&conn->terminal);
}

int _WriteToConnection(Connection *conn, const char *format, va_list args)
{
    int bytesWritten;
    char message[1024];

    if (conn->connectionStatus == DISCONNECTED)
    {
        return EOF;
    }
    
    /* TODO: Using vsnprintf would prevent possible buffer overflow here. */
    /* 
    vsnprintf(message, sizeof(message), format, args);
    */
    vsprintf(message, format, args);

    if (conn->terminal.isANSI)
    {
        bytesWritten = fputs(message, conn->outputStream);
    }
    else
    {
        bytesWritten = StripANSI(message, conn->outputStream);
    }
    fflush(conn->outputStream);
    
    return bytesWritten;
}

int WriteToConnection(Connection *conn, const char *format, ...)
{
    int bytesWritten = 0;
    va_list args;

    if (conn->connectionStatus == DISCONNECTED)
    {
        return EOF;
    }
    
    va_start(args, format);
    _WriteToConnection(conn, format, args);
    va_end(args);

    return bytesWritten;
}


bool AuthenticateConnection(Connection *conn, const char *username, 
    const char *password)
{
    if (!AuthenticateUser(&conn->user, username, password))
    {
        return FALSE;
    }   
    conn->connectionStatus = AUTHENTICATED;
    return TRUE;
}

void Disconnect(Connection *conn)
{
    if (conn == NULL)
    {
        return;
    }

    if (conn->outputStream != NULL)
    {
        fflush(conn->outputStream);
    }

    switch(conn->connectionType)
    {
        case CONSOLE:
            DisconnectConsole(conn);
            break;
        case SERIAL:
            DisconnectSerial(conn);
            break;
        case MODEM:
            DisconnectModem(conn);
            break;
        case TELNET:
            DisconnectTelnet(conn);
            break;
        default:
            Warning("Unknown connection type: %d.\n", conn->connectionType);
            break;
    }
    conn->connectionStatus = DISCONNECTED;
}
