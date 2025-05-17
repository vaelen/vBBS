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

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>

#define MAX_REQUESTS 5

typedef struct
{
    int socket;
    struct sockaddr_in remoteAddress;
} TelnetConnectionData;

TelnetListener* NewTelnetListener(int port)
{
    int sockfd;
    struct sockaddr_in serverAddress;
    TelnetListener *listener;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        Error("Telnet: Could not create socket, Error: %s", strerror(errno));
        return NULL;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        Error("Telnet: Bind failed, Error: %s", strerror(errno));
        close(sockfd);
        return NULL;
    }

    if (listen(sockfd, MAX_REQUESTS) < 0)
    {
        Error("Telnet: Listen failed, Error: %s", strerror(errno));
        close(sockfd);
        return NULL;
    }

    /* Set socket to non-blocking mode */
    fcntl(sockfd, F_SETFL, O_NONBLOCK); 

    Info("Telnet: Server listening on port %d", port);

    listener = (TelnetListener *)malloc(sizeof(TelnetListener));
    listener->socket = sockfd;
    listener->port = port;
    listener->serverAddress = serverAddress;
    return listener;
}

Connection* TelnetListenerAccept(TelnetListener *listener)
{
    TelnetConnectionData *telnetData = NULL;
    Connection *conn = NULL;
    int sockfd = 0;
    struct sockaddr_in remoteAddress;
    socklen_t addr_len = sizeof(remoteAddress);

    sockfd = accept(listener->socket, 
        (struct sockaddr *)&remoteAddress, &addr_len);
    if (sockfd < 0)
    {
        Error("Telnet: Accept failed, Error: %s", strerror(errno));
        return NULL;
    }

    Debug("Telnet: New connection from %s:%d", 
        inet_ntoa(remoteAddress.sin_addr), 
        ntohs(remoteAddress.sin_port));
    
    telnetData = (TelnetConnectionData *)malloc(sizeof(TelnetConnectionData));
    if (telnetData == NULL)
    {
        Error("Telnet: Memory allocation failed for connection data.");
        close(sockfd);
        return NULL;
    }
    telnetData->socket = sockfd;
    telnetData->remoteAddress = remoteAddress;
    conn = NewConnection();
    if (conn == NULL)
    {
        Error("Telnet: Memory allocation failed for connection.");
        free(telnetData);
        close(sockfd);
        return NULL;
    }
    conn->connectionType = TELNET;
    conn->connectionStatus = CONNECTED;
    conn->data = telnetData;
    /* enable non-blocking I/O */
    fcntl(sockfd, F_SETFL, O_NONBLOCK); 
    conn->inputStream = fdopen(sockfd, "r");
    conn->outputStream = fdopen(sockfd, "w");
    if (conn->inputStream == NULL || conn->outputStream == NULL)
    {
        Error("Telnet: Failed to open streams for connection.");
        free(telnetData);
        free(conn);
        close(sockfd);
        return NULL;
    }
    /** Disable stream buffering */
    setvbuf(conn->inputStream, NULL, _IONBF, 0);
    setvbuf(conn->outputStream, NULL, _IONBF, 0);
    return conn;
}

void DestroyTelnetListener(TelnetListener *listener) 
{
    if (listener == NULL)
    {
        return;
    }
    if (listener->socket >= 0)
    {
        close(listener->socket);
        Info("Telnet: Listener closed on port %d", listener->port);
    }
    free(listener);
}

void DisconnectTelnetConnection(Connection *conn, bool closeImmediately)
{
    if (conn == NULL || conn->connectionType != TELNET)
    {
        return;
    }

    /* Don't close the output stream or the socket unless there is no more
        data to send to the client. */
    if (closeImmediately || 
        (conn->outputBuffer != NULL && IsBufferEmpty(conn->outputBuffer)))
    {
        if (conn->data != NULL)
        {
            TelnetConnectionData *telnetData = 
                (TelnetConnectionData *)conn->data;
            if (telnetData->socket >= 0)
            {
                close(telnetData->socket);
                telnetData->socket = -1;
            }
        }
    }
}

void DestroyTelnetConnection(Connection *conn)
{
    if (conn == NULL || conn->connectionType != TELNET)
    {
        return;
    }

    if (conn->data != NULL)
    {
        TelnetConnectionData *telnetData = (TelnetConnectionData *)conn->data;
        if (telnetData->socket >= 0)
        {
            close(telnetData->socket);
            telnetData->socket = -1;
        }
        Debug("Telnet: Connection closed from %s:%d", 
            inet_ntoa(telnetData->remoteAddress.sin_addr), 
            ntohs(telnetData->remoteAddress.sin_port));
        free(conn->data);
        conn->data = NULL;
    }
}

const char* TelnetRemoteAddress(Connection *conn)
{
    TelnetConnectionData *telnetData = NULL;
    if (conn == NULL || conn->data == NULL)
    {
        return "";
    }
    telnetData = (TelnetConnectionData *)conn->data;
    return inet_ntoa(telnetData->remoteAddress.sin_addr);
}

int TelnetRemotePort(Connection *conn)
{
    TelnetConnectionData *telnetData = NULL;
    if (conn == NULL || conn->data == NULL)
    {
        return -1;
    }
    telnetData = (TelnetConnectionData *)conn->data;
    return ntohs(telnetData->remoteAddress.sin_port);
}

#endif /* _POSIX_VERSION */
