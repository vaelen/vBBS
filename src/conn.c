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
#include <stdarg.h>
#include <stdlib.h>
#include <vbbs/rb.h>
#include <vbbs/log.h>
#include <vbbs/conn.h>
#include <vbbs/conn/console.h>
#include <vbbs/conn/serial.h>
#include <vbbs/conn/modem.h>
#include <vbbs/conn/telnet.h>

#define CONNECTION_BUFFER_SIZE 1024

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
    conn->data = NULL;
    InitTerminal(&conn->terminal);
    conn->inputBuffer = NewRingBuffer(CONNECTION_BUFFER_SIZE);
    conn->outputBuffer = NewRingBuffer(CONNECTION_BUFFER_SIZE);
    if (conn->inputBuffer == NULL || conn->outputBuffer == NULL)
    {
        Error("Failed to create input/output buffers for connection.\n");
        exit(EXIT_FAILURE);
    }
    conn->inEscape = FALSE;
    conn->inCSI = FALSE;
}

void DestroyConnection(Connection *conn)
{
    switch(conn->connectionType)
    {
        case TELNET:
            DestroyTelnetConnection(conn);
            break;
        case CONSOLE:
        case SERIAL:
        case MODEM:
        default:
            free(conn);
            break;
    }
}

void WriteToConnection(Connection *conn, const char *format, ...)
{
    va_list args;
    char message[1024];

    if (conn->connectionStatus == DISCONNECTED)
    {
        return;
    }
    
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    WriteStringToRingBuffer(conn->outputBuffer, message);
    va_end(args);
    WriteBufferToConnection(conn);
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
            DisconnectTelnetConnection(conn);
            break;
        default:
            Warning("Unknown connection type: %d.\n", conn->connectionType);
            break;
    }
    conn->connectionStatus = DISCONNECTED;

    if (conn->inputBuffer != NULL)
    {
        DestroyRingBuffer(conn->inputBuffer);
        conn->inputBuffer = NULL;
    }
    if (conn->outputBuffer != NULL)
    {
        DestroyRingBuffer(conn->outputBuffer);
        conn->outputBuffer = NULL;
    }
}

/** 
 * This is a non-blocking function that writes the contents of the output 
 * buffer to the output stream. It returns the number of bytes written.
 * It will also remove ANSI escape codes if the terminal does not support ANSI.
 */
int WriteBufferToConnection(Connection *conn)
{
    int bytesWritten = 0, n = 0;
    char c;

    if (conn == NULL || conn->outputBuffer == NULL || 
        IsRingBufferEmpty(conn->outputBuffer))
    {
        return 0;
    }

    do
    {
        c = PeekRingBuffer(conn->outputBuffer);

        /** Output a character, removing escape codes if needed. */
        if (conn->terminal.isANSI)
        {
            n = fputc(c, conn->outputStream);
            if (n == EOF)
            {
                /* Can't write anymore, return. */
                break;
            }
            bytesWritten++;
        }
        else
        {
            /* If the terminal does not support ANSI, we need to strip the 
             * escape codes. */

            if (conn->inEscape)
            {
                if (conn->inCSI)
                {
                    if ((c >= 'A' && c <= 'Z') || 
                    (c >= 'a' && c <= 'z'))
                    {
                        /* end of CSI */
                        conn->inCSI = FALSE;
                        conn->inEscape = FALSE;
                    }
                    /* skip all characters in the CSI */
                }
                else if (c == ANSI_CSI_CHAR)
                {
                    /** start of CSI */
                    conn->inCSI = TRUE;
                }
                else {
                    /* single character escape */
                    conn->inEscape = FALSE;
                }
            }
            else if (c == ANSI_ESCAPE_CHAR)
            {
                /** start of escape */
                conn->inEscape = TRUE;
            }
            else
            {
                n = fputc(c, conn->outputStream);
                if (n == EOF)
                {
                    /* Can't write anymore, return. */
                    break;
                }
                bytesWritten++;
            }

        } /* end ANSI support check */

        /** Remove the character we wrote earlier. */
        PopRingBuffer(conn->outputBuffer);
    } while (n > 0);
 
    fflush(conn->outputStream);

    return bytesWritten;
}
