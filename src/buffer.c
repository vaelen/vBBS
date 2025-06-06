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
#include <vbbs/buffer.h>
#include <vbbs/terminal.h>
#include <vbbs/log.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/********** Buffer **********/

const char *const commandNames[] = {
    "SE", "NOP", "DM", "BRK", "IP", "AO", "AYT", "EC", "EL",
    "GA", "SB", "WILL", "WONT", "DO", "DONT", "IAC"};

const char *const optionNames[256] = {
    "BINARY", "ECHO", "RECONNECTION", "SUPPRESS_GO_AHEAD",
    "APPROX_MESSAGE_SIZE", "STATUS", "TIMING_MARK", "REMOTE_TRANS_AND_ECHO",
    "OUTPUT_LINE_WIDTH", "OUTPUT_PAGE_SIZE", "OUTPUT_CARRIAGE_RETURN",
    "OUTPUT_HORIZ_TAB_STOPS", "OUTPUT_HORIZ_TABS", "OUTPUT_FORM_FEED",
    "OUTPUT_VERT_TAB_STOPS", "OUTPUT_VERT_TABS", "OUTPUT_LINE_FEED",
    "EXTENDED_ASCII", "LOGOUT", "BYTE_MACRO", "DATA_ENTRY", "SUPDUP",
    "SUPDUP_OUTPUT", "SEND_LOCATION", "TERMINAL_TYPE", "END_OF_RECORD",
    "TACACS_USER_ID", "OUTPUT_MARKING", "TERMINAL_LOCATION", "TN3270_REGIME",
    "X.3_PAD", "NEG_WINDOW_SIZE", "TERMINAL_SPEED", "FLOW_CONTROL",
    "LINEMODE", "X_DISPLAY_LOCATION", "ENV", "NEW_ENV", "AUTHENTICATION",
    "ENCRYPTION", "NEW_ENVIRON", "TN3270E", "XAUTH", "CHARSET", "RSP",
    "COM_PORT_OPTION", "SUPPRESS_LOCAL_ECHO", "START_TLS", "KERMIT",
    "SEND_URL", "FORWARD_X",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "PRAGMA_LOGIN", "SSPI_LOGON", "PRAGMA_HEARTBEAT",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "EXTENDED_OPTIONS_LIST"};

static const char *TelnetCommand(int command)
{
    if (command < 240 || command >= 256)
    {
        return "OUT_OF_RANGE";
    }
    return commandNames[command - 240];
}

static const char *TelnetOption(int option)
{
    if (option < 0 || option >= 256)
    {
        return "OUT_OF_RANGE";
    }
    return optionNames[option];
}

Buffer *NewBuffer(int size)
{
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    if (buffer == NULL)
    {
        return NULL;
    }
    buffer->bytes = (uint8_t *)malloc(size + 1);
    if (buffer->bytes == NULL)
    {
        free(buffer);
        return NULL;
    }
    memset(buffer->bytes, 0, size + 1);
    buffer->tail = buffer->bytes;
    buffer->length = 0;
    buffer->maxSize = size;
    buffer->convertNewlines = FALSE; /* Default to not converting newlines */
    buffer->handleANSI = FALSE;      /* Default to not handling ANSI */
    buffer->handleTelnet = FALSE;    /* Default to not handling Telnet */
    buffer->inANSI = FALSE;
    buffer->inTelnet = 0;
    buffer->inTelnetSB = 0;
    memset(buffer->commandBuffer, 0, sizeof(buffer->commandBuffer));
    buffer->windowSize = NULL;      /* No handler by default */
    buffer->terminalType = NULL;    /* No handler by default */
    buffer->connectionSpeed = NULL; /* No handler by default */
    return buffer;
}

static void _SetWindowSize(Buffer *buffer, int width, int height)
{
    if (buffer !=NULL && buffer->windowSize != NULL)
    {
        buffer->windowSize(buffer->userData, width, height);
    }
}

static void _SetTerminalType(Buffer *buffer, const char *type)
{
    if (buffer !=NULL && buffer->terminalType != NULL)
    {
        buffer->terminalType(buffer->userData, type);
    }
}

static void _SetConnectionSpeed(Buffer *buffer, int speed)
{
    if (buffer !=NULL && buffer->connectionSpeed != NULL)
    {
        buffer->connectionSpeed(buffer->userData, speed);
    }
}

void DestroyBuffer(Buffer *buffer)
{
    if (buffer->bytes != NULL)
    {
        free(buffer->bytes);
        buffer->bytes = NULL;
    }
    free(buffer);
}

void ClearBuffer(Buffer *buffer)
{
    buffer->tail = buffer->bytes;
    buffer->length = 0;
}

bool IsBufferEmpty(Buffer *buffer)
{
    return buffer->length == 0;
}

bool IsBufferFull(Buffer *buffer)
{
    return buffer->length == buffer->maxSize;
}

int BufferRemaining(Buffer *buffer)
{
    return buffer->maxSize - buffer->length;
}

int WriteToBuffer(Buffer *buffer, const char *data, int length)
{
    int i;
    int cmd, option, width, height, speed;
    char terminalType[64];

    for (i = 0; i < length; i++)
    {
        if (FALSE)
        Debug(
            "Writing byte %d: '%c' (0x%02X) T: %d DB: %d Size: %4d, Max: %4d",
            i,
            data[i] >= 32 && data[i] < 127 ? data[i] : '?',
            (unsigned char)data[i],
            buffer->inTelnet, buffer->inTelnetSB,
            buffer->tail - buffer->bytes,
            buffer->maxSize);

        if (buffer->tail - buffer->bytes >= buffer->maxSize)
        {
            break;
        }

        if (data[i] == '\n' && buffer->convertNewlines)
        {
            if (buffer->tail - buffer->bytes >= buffer->maxSize - 1)
            {
                i--; /* Not enough space for CR+LF */
                break;
            }
            /* Convert newline to carriage return + line feed */
            *buffer->tail++ = '\r';
            *buffer->tail++ = '\n';
        }
        else if(data[i] == (char)TELNET_IAC && buffer->handleTelnet)
        {
            if (buffer->inTelnet == 0)
            {
                /* Start of Telnet command */
                buffer->inTelnet = 1;
                buffer->commandBuffer[0] = data[i];
                buffer->inTelnetSB = 0; /* Reset subnegotiation state */
            }
            else if (buffer->inTelnet > 0 && buffer->inTelnet < 3)
            {
                /* Continue processing Telnet command */
                buffer->commandBuffer[buffer->inTelnet++] = data[i];
            }
        }
        else if (buffer->inTelnet == 0 && data[i] == (char)TELNET_IAC &&
            buffer->handleTelnet)
        {
            /* Start processing Telnet command */
            buffer->inTelnet = 1;
        }
        else if (buffer->inTelnet > 0)
        {
            buffer->commandBuffer[buffer->inTelnet - 1] = data[i];

            if (buffer->inTelnet == 1 && data[i] == (char)TELNET_SE)
            {
                /* End of telnet subnegotiation */
                option = buffer->commandBuffer[1];
                cmd = buffer->commandBuffer[2]; /* Subnegotiation command */

                buffer->commandBuffer[buffer->inTelnetSB + 2] = '\0';

                Debug("[TELNET] Subnegotiation, option: %s",
                      TelnetOption(option));

                if (cmd == 0) switch(option)
                {
                    case TELNET_OPTION_WINDOW_SIZE:
                        width = (buffer->commandBuffer[2] << 8) |
                            buffer->commandBuffer[3];
                        height = (buffer->commandBuffer[4] << 8) |
                            buffer->commandBuffer[5];
                        Debug("Telnet window size: %dx%d", width, height);
                        _SetWindowSize(buffer, width, height);
                        break;
                    case TELNET_OPTION_TERMINAL_TYPE:
                        strncpy(terminalType,
                               (const char *)&buffer->commandBuffer[3],
                               buffer->inTelnetSB - 1);
                        Debug("Telnet terminal type: %s", terminalType);
                        _SetTerminalType(buffer, terminalType);
                        break;
                    case TELNET_OPTION_TERMINAL_SPEED:
                        speed = (buffer->commandBuffer[2] << 8) |
                            buffer->commandBuffer[3];
                        Debug("Telnet connection speed: %d", speed);
                        _SetConnectionSpeed(buffer, speed);
                        break;
                    default:
                        Debug("[TELNET] Unhandled subnegotiation option: %s",
                              TelnetOption(option));
                }

                buffer->inTelnetSB = 0; /* Reset subnegotiation state */
                buffer->inTelnet = -1;   /* Reset Telnet state */
                memset(buffer->commandBuffer, 0, sizeof(buffer->commandBuffer));
            }
            else if (buffer->inTelnet == 2)
            {
                /* End of normal telnet command */
                cmd = buffer->commandBuffer[0];
                option = buffer->commandBuffer[1];

                Debug("Received telnet command: %s %s",
                      TelnetCommand(cmd), TelnetOption(option));

                if (cmd == TELNET_WILL)
                {
                    switch (option)
                    {
                    case TELNET_OPTION_WINDOW_SIZE:
                        /* Respond with DO */
                        WriteStringToBuffer(buffer, TELNET_REQ_WINDOW_SIZE);
                        Debug("Sending REQ WINDOW_SIZE");
                        break;
                    case TELNET_OPTION_TERMINAL_TYPE:
                        /* Respond with DO */
                        WriteStringToBuffer(buffer, TELNET_REQ_TERMINAL_TYPE);
                        Debug("Sending REQ TERMINAL_TYPE");
                        break;
                    case TELNET_OPTION_TERMINAL_SPEED:
                        /* Respond with DO */
                        WriteStringToBuffer(buffer, TELNET_REQ_TERMINAL_SPEED);
                        Debug("Sending REQ TERMINAL_SPEED");
                        break;
                    default:
                        Debug("Unhandled WILL command for option: %s",
                              TelnetOption(option));
                    }
                }

                /* We have the command, now check it */
                if (cmd == TELNET_SB)
                {
                    /* Start of subnegotiation */
                    buffer->inTelnetSB = 1;
                }
                else
                {
                    memset(buffer->commandBuffer, 0, 
                        sizeof(buffer->commandBuffer));
                }
                buffer->inTelnet = -1; /* Reset Telnet state */
            }
            buffer->inTelnet++;

        }
        else if (buffer->inTelnetSB > 0)
        {
            buffer->commandBuffer[1 + buffer->inTelnetSB++] = data[i];
        }
        else
        {
            *buffer->tail++ = data[i];
        }
    }
    buffer->length = buffer->tail - buffer->bytes; /* Update length */
    return i;                                      /* Success */
}

int WriteStringToBuffer(Buffer *buffer, const char *data)
{
    int length = strlen(data);
    return WriteToBuffer(buffer, data, length);
}

int WriteBufferToStream(Buffer *buffer, FILE *out)
{
    return fwrite(buffer->bytes, 1, buffer->length, out);
}

int ReadFromBuffer(Buffer *buffer, uint8_t *data, int length)
{
    int bytesRead = MIN(length, buffer->length);
    memcpy(data, buffer->bytes, bytesRead);
    ShiftBuffer(buffer, bytesRead);
    return bytesRead;
}

/**
 * ShiftBuffer shifts the contents of the buffer by the specified offset.
 * Another way to think of this is that it removes the first offset bytes.
 * If the offset is greater than or equal to the length of the buffer,
 * the buffer is cleared.
 */
void ShiftBuffer(Buffer *buffer, int offset)
{
    if (offset >= buffer->length)
    {
        ClearBuffer(buffer);
        return;
    }
    memmove(buffer->bytes, buffer->bytes + offset, buffer->length - offset);
    buffer->tail -= offset;
    buffer->length -= offset;
}

/**
 * RemoveFromBuffer removes a specified number of bytes from the buffer
 * starting at the given offset. If the offset is greater than or equal to
 * the length of the buffer, it does nothing.
 */
void RemoveFromBuffer(Buffer *buffer, int offset, int length)
{
    if (offset < 0 || offset >= buffer->length || length <= 0)
    {
        return; /* Invalid offset or length */
    }
    if (offset + length > buffer->length)
    {
        length = buffer->length - offset; /* Adjust length to fit */
    }
    memmove(buffer->bytes + offset, buffer->bytes + offset + length,
            buffer->length - (offset + length));
    buffer->tail -= length;
    buffer->length -= length;
}

/********** InputBuffer **********/

InputBuffer *NewInputBuffer(int size)
{
    InputBuffer *buffer = (InputBuffer *)malloc(sizeof(InputBuffer));
    if (buffer == NULL)
    {
        return NULL;
    }

    buffer->buffer = NewBuffer(size);
    if (buffer->buffer == NULL)
    {
        free(buffer);
        return NULL;
    }

    buffer->buffer->handleANSI = TRUE;   /* Default to handling ANSI */
    buffer->buffer->handleTelnet = TRUE; /* Default to handling Telnet */

    buffer->inputMode = LINE_INPUT_MODE;
    buffer->nextLine[0] = '\0';
    buffer->nextLineReady = FALSE;

    memset(buffer->nextLine, 0, sizeof(buffer->nextLine));
    return buffer;
}

void DestroyInputBuffer(InputBuffer *buffer)
{
    if (buffer->buffer != NULL)
    {
        DestroyBuffer(buffer->buffer);
        buffer->buffer = NULL;
    }
    free(buffer);
}

int ReadDataFromStream(InputBuffer *buffer, FILE *in)
{
    int bytesRead = 0;
    int bytesToRead = MIN(BufferRemaining(buffer->buffer), 1024);
    char buf[1024];

    if (bytesToRead <= 0)
    {
        return 0; /* Buffer is full */
    }

    bytesRead = fread(buf, 1, bytesToRead, in);
    if (bytesRead > 0)
    {
        /**
         * We write as a separate step to ensure that we handle
         * any necessary conversions (like newline conversion) in the
         * WriteToBuffer function.
         * This also allows us to handle Telnet commands and ANSI sequences
         * correctly.
         */
        WriteToBuffer(buffer->buffer, buf, bytesRead);
    }
    
    return bytesRead;
}

void SetInputMode(InputBuffer *buffer, InputMode mode)
{
    if (buffer == NULL)
    {
        return; /* Invalid buffer */
    }
    buffer->inputMode = mode;
    buffer->nextLineReady = FALSE; /* Reset next line ready flag */
    ClearNextLine(buffer);          /* Clear next line content */
    ClearBuffer(buffer->buffer); /* Clear the buffer */
}

bool IsNextLineReady(InputBuffer *buffer)
{
    if (buffer == NULL || buffer->buffer == NULL)
    {
        return FALSE; /* Invalid buffer */
    }

    if (buffer->nextLineReady)
    {
        return TRUE;
    }

    switch (buffer->inputMode)
    {
    case LINE_INPUT_MODE:
        buffer->nextLineReady = FindNextLine(buffer);
        break;
    case CHARACTER_INPUT_MODE:
        buffer->nextLineReady = FindNextCharacter(buffer);
        break;
    default:
        buffer->nextLineReady = FALSE; /* Invalid input mode */
        break;
    }

    return buffer->nextLineReady;
}

bool FindNextLine(InputBuffer *buffer)
{
    int i;
    int length;
    uint8_t *buf;

    if (buffer == NULL || buffer->buffer == NULL)
    {
        return FALSE; /* Invalid buffer */
    }

    length = buffer->buffer->length;
    buf = buffer->buffer->bytes;

    if (length == 0)
    {
        return FALSE; /* No data to process */
    }

    for (i = 0; i < length; i++)
    {
        if (buf[i] > 0 && buf[i] != '\r' && buf[i] != '\n' &&
            (buf[i] < 0x20 || buf[i] > 0x7E))
        {
            /* Skip non-printable characters */
            /*Debug("Skipping non-printable character %02X at position %d",
                buf[i], i);*/
            continue;
        }
        /**
         * The client may send CR, CR+LF, or CR+NULL (telnet).
         * We simply ignore any LF or NULL characters, whether or not they
         * are preceded by a CR.
         */
        if (buf[i] == '\0' || buf[i] == '\n')
        {
            RemoveFromBuffer(buffer->buffer, i, 1);
            length--; /* Adjust length after removal */
            i--;      /* Adjust index after removal */
            continue;
        }
        if (buf[i] == '\r')
        {
            /* Found a line ending, copy the line. */
            strncpy(buffer->nextLine, (char *)buf, i);
            /* Null-terminate the string */
            buffer->nextLine[i] = '\0';
            /* Remove the line from the buffer */
            ShiftBuffer(buffer->buffer, i + 1);
            return TRUE; /* Line found */
        }
    }

    /* If we reach here, no line ending was found */
    buffer->nextLine[0] = '\0'; /* Clear nextLine */

    return FALSE; /* No line found */
}

bool FindNextCharacter(InputBuffer *buffer)
{
    if (buffer == NULL || buffer->buffer == NULL)
    {
        return FALSE; /* Invalid buffer */
    }

    if (!IsBufferEmpty(buffer->buffer))
    {
        /* Copy the first character to nextLine */
        buffer->nextLine[0] = buffer->buffer->bytes[0];
        buffer->nextLine[1] = '\0';     /* Null-terminate the string */
        ShiftBuffer(buffer->buffer, 1); /* Remove the character from the buffer */
        Debug("Next character: '%s'", buffer->nextLine);
        return TRUE; /* Character found */
    }

    Debug("No character found in buffer.");
    buffer->nextLine[0] = '\0'; /* Clear nextLine */
    return FALSE;               /* No character found */
}

void ClearNextLine(InputBuffer *buffer)
{
    if (buffer == NULL)
    {
        return;
    }
    memset(buffer->nextLine, 0, sizeof(buffer->nextLine));
    buffer->nextLineReady = FALSE;
}

void CleanString(char *str)
{
    int i;
    int l = strlen(str);
    if (str == NULL)
    {
        return;
    }
    for (i = 0; i < l; i++)
    {
        if (str[i] == 0)
        {
            break;
        }
        if (str[i] < 0x20 || str[i] > 0x7E)
        {
            if (str[i + 1] == 0)
            {
                str[i] = '\0';
                break;
            }
            else
            {
                /* Shift the rest of the string */
                memmove(str + i, str + i + 1, l - i);
                i--;
                l--;
                continue;
            }
        }
    }
}

void BytesToHexString(char *bytes, int bytesSize, char *out, int outSize)
{
    int i;
    int offset = 0;

    for (i = 0; i < bytesSize && offset < outSize - 1; i++)
    {
        offset += snprintf(out + offset, outSize - offset, "%02X ",
                           bytes[i]);
    }
    out[offset] = '\0';
}
