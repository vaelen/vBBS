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

#include <errno.h>

#ifdef __unix__
#include <sys/select.h>
#endif

int main(int argc, char *argv[])
{
    Connection *conn;
    Session session;
    TelnetListener *listener;
    int port = 23;

#ifdef __unix__
    int in_fd, out_fd;
    fd_set read_fds, write_fds;
    struct timeval timeout;
    int max_fd;
#endif    

    if (argc > 1)
    {
        port = atoi(argv[1]);
    }

    Info("Hello, vBBS!");

    listener = NewTelnetListener(port);
    if (listener == NULL)
    {
        Error("Failed to create Telnet listener on port %d.", port);
        return EXIT_FAILURE;
    }
    
    conn = TelnetListenerAccept(listener);
    if (conn == NULL)
    {
        Error("Failed to accept Telnet connection.");
        DestroyTelnetListener(listener);
        return EXIT_FAILURE;
    }

    InitSession(&session);
    session.conn = conn;
    Connected(&session);

    /** Event Loop */
    while (session.conn->connectionStatus != DISCONNECTED)
    {
#ifdef __unix__
        in_fd = fileno(session.conn->inputStream);
        out_fd = fileno(session.conn->outputStream);
        max_fd = MAX(in_fd, out_fd);
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        if (!IsBufferFull(session.conn->inputBuffer->buffer))
        {
            FD_SET(in_fd, &read_fds);
        }

        if (!IsBufferEmpty(session.conn->outputBuffer))
        {
            FD_SET(out_fd, &write_fds);
        }

        timeout.tv_sec = 5;  // Set timeout to 5 seconds
        timeout.tv_usec = 0;

        if(select(max_fd + 1, &read_fds, &write_fds, NULL, NULL) < 0)
        {
            perror("select() failed");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(in_fd, &read_fds))
        {
            ReadDataFromStream(session.conn->inputBuffer, 
                session.conn->inputStream);
            if(feof(session.conn->inputStream))
            {
                Debug("End of file reached on input stream.");
                Disconnect(conn);
                break;
            }
            else if (ferror(session.conn->inputStream))
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
                        Disconnect(conn);
                        break;
                }
            }
            if (IsNextLineReady(session.conn->inputBuffer))
            {
                session.eventHandler(&session);
            }
        }

        if (FD_ISSET(out_fd, &write_fds))
        {
            WriteBufferToConnection(session.conn);
            if(feof(session.conn->outputStream))
            {
                Debug("End of file reached on input stream.");
                Disconnect(conn);
                break;
            }
            else if (ferror(session.conn->outputStream))
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
                        Disconnect(conn);
                        break;
                }
            }
        }
#else
perror("select() is not supported on this platform.");
        break;
#endif

        /** TODO: Other things should be processed here. */

    } /* End of Event Loop */

    Disconnect(conn);
    DestroyConnection(conn);

    DestroyTelnetListener(listener);

    return EXIT_SUCCESS;
}
