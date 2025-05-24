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

#ifdef _POSIX_VERSION
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
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
        DestroySession(session);
    }
}

void CreateConsoleConnection(ArrayList *sessions)
{
    Connection *conn;
    Session *session;

    /** Create the console connection */
    conn = NewConnection();
    if (conn == NULL)
    {
        Error("Failed to create console connection.");
        return;
    }
    conn->inputStream = stdin;
    conn->outputStream = stdout;
    conn->connectionType = CONSOLE;
    conn->connectionStatus = CONNECTED;

    session = NewSession(conn);
    if (session == NULL)
    {
        Error("Failed to create session.");
        DestroyConnection(conn);
        return;
    }

    session->eventHandler = Connected;
    Connected(session);

    AddToArrayList(sessions, session);

    conn = NULL;
    session = NULL;
}

void AcceptTelnetConnection(TelnetListener *listener, ArrayList *sessions)
{
    Connection *conn;
    Session *session;

    conn = TelnetListenerAccept(listener);
    if (conn != NULL)
    {
        session = NewSession(conn);
        if (session == NULL)
        {
            Error("Failed to create session.");
            DestroyConnection(conn);
            return;
        }

        AddToArrayList(sessions, session);

        session->eventHandler = Connected;
        Connected(session);
    }
}

void ReadFromSession(Session *session)
{
    ReadDataFromStream(session->conn->inputBuffer, 
        session->conn->inputStream);
    if(feof(session->conn->inputStream))
    {
        if (session->conn->inputStream == stdin)
        {
            running = FALSE;
            Info("Received EOF on stdin, shutting down.");
        }
        Disconnect(session->conn, FALSE);
        return;
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
                Error("[%d] Error reading from input stream: %s", 
                    session->sessionID,
                    strerror(errno));
                Disconnect(session->conn, FALSE);
                break;
        }
    }
    if (IsNextLineReady(session->conn->inputBuffer) && 
        session->eventHandler != NULL)
    {
        session->eventHandler(session);
    }
}

void WriteToSession(Session *session)
{
    if (session == NULL || session->conn == NULL)
    {
        return;
    }

    WriteBufferToConnection(session->conn);
    if(feof(session->conn->outputStream))
    {
        Debug("End of file reached on input stream.");
        Disconnect(session->conn, TRUE);
        return;
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
                Error("[%d] Error writing to output stream: %s", 
                    session->sessionID,
                    strerror(errno));
                Disconnect(session->conn, TRUE);
                break;
        }
    }
}

void PruneSessions(ArrayList *sessions)
{
    bool done = FALSE;
    Session *session;
    int i;

    while (!done)
    {
        done = TRUE;
        for (i = 0; i < sessions->size; i++)
        {
            session = (Session *)GetFromArrayList(sessions, i);
            if (session == NULL || session->conn == NULL)
            {
                continue;
            }

            if (session->conn->connectionStatus == DISCONNECTED)
            {
                if (session->conn->inputStream != stdin &&
                    (session->conn->outputBuffer == NULL ||
                        session->conn->outputStream == NULL ||
                        IsBufferEmpty(session->conn->outputBuffer)))
                {
                    RemoveFromArrayList(sessions, i);
                    done = FALSE;
                    break;
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    Session *session = NULL;

    ArrayList *sessions = NewArrayList(10, SessionDestructor);

    TelnetListener *telnetListener = NULL;
    int telnetPort = TELNET_PORT;

#ifdef _POSIX_VERSION
    int fd, max_fd, i;
    fd_set read_fds, write_fds;
    struct timeval *timeout = malloc(sizeof(struct timeval));

    /** Set stdin and stdout to non-blocking mode */
    fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
    fcntl(fileno(stdout), F_SETFL, O_NONBLOCK);
#endif

    /** TODO: Add support for command line arguments to set the port. */
    if (argc > 1)
    {
        telnetPort = atoi(argv[1]);
    }

    InitLog("vbbs.log");

    Info("Starting %s", VBBS_VERSION_STRING);

    signal(SIGINT, SignalHandler);

    LoadUserDB() ? 
        Info("User database loaded successfully.") : 
        Error("Failed to load user database: %s", USER_DB_FILE);

    sessions = NewArrayList(32, SessionDestructor);

    telnetListener = NewTelnetListener(telnetPort);
    if (telnetListener == NULL)
    {
        Error("Failed to create Telnet listener on port %d.", telnetPort);
    }
    
    /*
    CreateConsoleConnection(sessions);
    */

    /** Event Loop */
    while (running)
    {

/********** UNIX Event Loop **********/
#ifdef _POSIX_VERSION
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        if (telnetListener != NULL && telnetListener->socket >= 0)
        {
            FD_SET(telnetListener->socket, &read_fds);
            max_fd = telnetListener->socket;
        }

        for (i = 0; i < sessions->size; i++)
        {
            session = (Session *)GetFromArrayList(sessions, i);
            if (session == NULL)
            {
                Warn("Session is NULL, skipping %d.", i);
                continue;
            }

            if (session->conn == NULL)
            {
                Warn("Session connection is NULL, skipping %d.", i);
                continue;
            }

            if (!IsBufferFull(session->conn->inputBuffer->buffer))
            {
                fd = fileno(session->conn->inputStream);
                if (fd >= 0)
                {
                    max_fd = MAX(max_fd, fd);
                    FD_SET(fd, &read_fds);
                }
            }
    
            if (!IsBufferEmpty(session->conn->outputBuffer))
            {
                    fd = fileno(session->conn->outputStream);
                    if (fd >= 0)
                    {
                        max_fd = MAX(max_fd, fd);
                        FD_SET(fd, &write_fds);
                    }
            }
        } /* End for(sessions) */

        /** Set timeout for select() to 5 seconds */
        if (timeout != NULL)
        {
            timeout->tv_sec = 1;
            timeout->tv_usec = 0;
        }   

        if(select(max_fd + 1, &read_fds, &write_fds, NULL, timeout) < 0)
        {
            if (errno == EINTR || errno == EAGAIN)
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
        if(telnetListener != NULL && 
            FD_ISSET(telnetListener->socket, &read_fds))
        {
            AcceptTelnetConnection(telnetListener, sessions);
        }

        /** Check for data on existing connections */
        for (i = 0; i < sessions->size; i++)
        {
            session = (Session *)GetFromArrayList(sessions, i);

            if (session == NULL || session->conn == NULL)
            {
                continue;
            }

            if (session->conn->inputStream != NULL)
            {
                fd = fileno(session->conn->inputStream);

                /* Check for data to read from the input stream */
                if (FD_ISSET(fd, &read_fds))
                {
                    ReadFromSession(session);
                } /* End if(FD_ISSET(in_fd, &read_fds)) */
            }
            
            if (session->conn->outputStream != NULL)
            {
                fd = fileno(session->conn->outputStream);

                /** Check for data to write to the output stream */
                if (FD_ISSET(fd, &write_fds))
                {
                    WriteToSession(session);
                } /* End if(FD_ISSET(out_fd, &write_fds)) */
            }
        } /* End for(sessions) */
#else
perror("select() is not supported on this platform.");
        break;
#endif

/********** Generic Event Loop **********/

        /** Prune Sessions */
        PruneSessions(sessions);

        /** TODO: Other things should be processed here. */

    } /* End of Event Loop */

    DestroyTelnetListener(telnetListener);

    DestroyArrayList(sessions);

    Info("Shutting down %s", VBBS_VERSION_STRING);

    CloseLog();

#ifdef _POSIX_VERSION
    if (timeout != NULL)
    {
        free(timeout);
    }
#endif

    return EXIT_SUCCESS;
}
