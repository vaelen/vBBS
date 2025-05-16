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

#include <vbbs.h>

#include <signal.h>
#include <errno.h>

#ifdef __unix__
#include <sys/select.h>
#endif

bool running = TRUE;

void SignalHandler(int signum)
{
    if (signum == SIGINT)
    {
        Info("Received SIGINT, shutting down.");
        running = FALSE;
    }
}

void SessionDestructor(void *item)
{
    Session *session = (Session *)item;
    if (session != NULL)
    {
        Info("Destroying session.");
        Disconnect(session->conn);
        DestroySession(session);
    }
}

int main(int argc, char *argv[])
{
    Session *session;
    Connection *conn;
    TelnetListener *telnetListener;
    int port = 23;

    ArrayList *sessions = NewArrayList(10, SessionDestructor);

#ifdef __unix__
    int fd, max_fd, i;
    fd_set read_fds, write_fds;
    /* struct timeval timeout; */
#endif

    if (argc > 1)
    {
        port = atoi(argv[1]);
    }

    Info("Starting %s", VBBS_VERSION_STRING);

    signal(SIGINT, SignalHandler);

    sessions = NewArrayList(32, SessionDestructor);

    telnetListener = NewTelnetListener(port);
    if (telnetListener == NULL)
    {
        Error("Failed to create Telnet listener on port %d.", port);
        return EXIT_FAILURE;
    }
    
    /** Event Loop */
    while (running)
    {
#ifdef __unix__
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        FD_SET(telnetListener->socket, &read_fds);
        max_fd = telnetListener->socket;

        for (i = 0; i < sessions->size; i++)
        {
            session = (Session *)GetFromArrayList(sessions, i);
            if (session == NULL)
            {
                Warning("Session is NULL, skipping %d.", i);
                continue;
            }

            if (session->conn == NULL)
            {
                Warning("Session connection is NULL, skipping %d.", i);
                continue;
            }

            if (session->conn->connectionStatus == DISCONNECTED)
            {
                RemoveFromArrayList(sessions, i);
                i--;
                continue;
            }

            if (!IsBufferFull(session->conn->inputBuffer->buffer))
            {
                fd = fileno(session->conn->inputStream);
                max_fd = MAX(max_fd, fd);
                FD_SET(fd, &read_fds);
            }
    
            if (!IsBufferEmpty(session->conn->outputBuffer))
            {
                    fd = fileno(session->conn->outputStream);
                    fd = MAX(max_fd, fd);
                    FD_SET(fd, &write_fds);
            }
        } /* End for(sessions) */

        /*
        timeout.tv_sec = 5;  // Set timeout to 5 seconds
        timeout.tv_usec = 0;
        */

        if(select(max_fd + 1, &read_fds, &write_fds, NULL, NULL) < 0)
        {
            if (errno == EINTR)
            {
                /* Interrupted by signal, continue. */
                continue;
            }
            else
            {
                Error("Error in select(): %s", strerror(errno));
                break;
            }
        }

        /** Check for new connections */
        if(FD_ISSET(telnetListener->socket, &read_fds))
        {
            conn = TelnetListenerAccept(telnetListener);
            if (conn != NULL)
            {
                Info("New connection accepted.");
                session = NewSession(conn);
                if (session == NULL)
                {
                    Error("Failed to create session.");
                    Disconnect(session->conn);
                    break;
                }
                AddToArrayList(sessions, session);
                session->eventHandler = Connected;
                Connected(session);
            }
        }

        /** Check for data on existing connections */
        for (i = 0; i < sessions->size; i++)
        {
            session = (Session *)GetFromArrayList(sessions, i);

            fd = fileno(session->conn->inputStream);

            /* Check for data to read from the input stream */
            if (FD_ISSET(fd, &read_fds))
            {
                ReadDataFromStream(session->conn->inputBuffer, 
                    session->conn->inputStream);
                if(feof(session->conn->inputStream))
                {
                    Debug("End of file reached on input stream.");
                    Disconnect(session->conn);
                    break;
                }
                else if (ferror(session->conn->inputStream))
                {
                    switch (errno)
                    {
                        case EINTR:
                            /* Non-blocking read interrupted, continue. */
                            break;
                        case EAGAIN:
                            /* Non-blocking read blocked, continue. */
                            break;
                        default:
                            Error("Error reading from input stream: %s", 
                                strerror(errno));
                            Disconnect(session->conn);
                            break;
                    }
                }
                if (IsNextLineReady(session->conn->inputBuffer))
                {
                    session->eventHandler(session);
                }
            } /* End if(FD_ISSET(in_fd, &read_fds)) */
            
            fd = fileno(session->conn->outputStream);

            /** Check for data to write to the output stream */
            if (FD_ISSET(fd, &write_fds))
            {
                WriteBufferToConnection(session->conn);
                if(feof(session->conn->outputStream))
                {
                    Debug("End of file reached on input stream.");
                    Disconnect(session->conn);
                    break;
                }
                else if (ferror(session->conn->outputStream))
                {
                    switch (errno)
                    {
                        case EINTR:
                            /* Non-blocking write interrupted, continue. */
                            break;
                        case EAGAIN:
                            /* Non-blocking write blocked, continue. */
                            break;
                        default:
                            Error("Error writing to output stream: %s", 
                                strerror(errno));
                            Disconnect(session->conn);
                            break;
                    }
                }
            } /* End if(FD_ISSET(out_fd, &write_fds)) */
        } /* End for(sessions) */
#else
perror("select() is not supported on this platform.");
        break;
#endif

        /** TODO: Other things should be processed here. */

    } /* End of Event Loop */

    DestroyTelnetListener(telnetListener);

    DestroyArrayList(sessions);

    Info("Shutting down %s", VBBS_VERSION_STRING);

    return EXIT_SUCCESS;
}
