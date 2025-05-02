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
#include <vBBS/Log.h>
#include <vBBS/Connection.h>
#include <vBBS/ConsoleConnection.h>
#include <vBBS/TelnetConnection.h>

/***** UNIX Implementation Using Berkeley Sockets *****/

#ifdef __unix__

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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

    printf("Telnet: Server listening on port %d\n", port);

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

    Info("Telnet: New connection from %s:%d\n", 
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
    conn = (Connection *)malloc(sizeof(Connection));
    if (conn == NULL)
    {
        Error("Telnet: Memory allocation failed for connection.");
        free(telnetData);
        close(sockfd);
        return NULL;
    }
    InitConnection(conn);
    conn->connectionType = TELNET;
    conn->connectionStatus = CONNECTED;
    conn->data = telnetData;
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

void CloseTelnetListener(TelnetListener *listener) 
{
    close(listener->socket);
    Info("Telnet: Listener closed on port %d\n", listener->port);
    free(listener);
}

void DisconnectTelnet(Connection *conn)
{
    if (conn == NULL || conn->connectionType != TELNET)
    {
        Warning("DisconnectTelnet: Invalid connection type or NULL connection.");
        return;
    }

    if (conn->inputStream != NULL)
    {
        fclose(conn->inputStream);
        conn->inputStream = NULL;
    }

    if (conn->outputStream != NULL)
    {
        fclose(conn->outputStream);
        conn->outputStream = NULL;
    }

    if (conn->data != NULL)
    {
        TelnetConnectionData *telnetData = (TelnetConnectionData *)conn->data;
        close(telnetData->socket);
        Info("Telnet: Connection closed from %s:%d\n", 
            inet_ntoa(telnetData->remoteAddress.sin_addr), 
            ntohs(telnetData->remoteAddress.sin_port));
        free(telnetData);
        conn->data = NULL;
    }

    free(conn);
}

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

void DisconnectTelnet(Connection *conn)
{
    Warn("DisconnectTelnet: Not implemented on this platform.");
}

#endif
