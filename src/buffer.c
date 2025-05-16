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
#include <vbbs/log.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/********** Buffer **********/

Buffer* NewBuffer(int size)
{
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    if (buffer == NULL)
    {
        return NULL;
    }
    buffer->bytes = (uint8_t *)malloc(size+1);
    if (buffer->bytes == NULL)
    {
        free(buffer);
        return NULL;
    }
    memset(buffer->bytes, 0, size+1);
    buffer->tail = buffer->bytes;
    buffer->length = 0;
    buffer->maxSize = size;
    return buffer;
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
    if (buffer->length + length > buffer->maxSize)
    {
        return -1; // Buffer overflow
    }
    memcpy(buffer->tail, data, length);
    buffer->tail += length;
    buffer->length += length;
    return length; // Success
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

/********** InputBuffer **********/

InputBuffer* NewInputBuffer(size_t size)
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

    buffer->inputMode = LINE_INPUT_MODE;
    buffer->nextCharacter = 0;
    buffer->nextLine[0] = '\0';
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
    int bytesToRead = BufferRemaining(buffer->buffer);
    if (bytesToRead <= 0)
    {
        return 0; // Buffer is full
    }

    bytesRead = fread(buffer->buffer->tail, 1, bytesToRead, in);
    if (bytesRead > 0)
    {
        buffer->buffer->length += bytesRead;
        buffer->buffer->tail += bytesRead;
    }
    return bytesRead;
}

bool IsNextLineReady(InputBuffer *buffer)
{
    int i;

    if (buffer->nextLine[0] != '\0')
    {
        return TRUE;
    }

    /* Look for a newline in the buffer. */
    for (i = 0; i < buffer->buffer->length; i++)
    {
        if (buffer->buffer->bytes[i] == '\n')
        {
            /* Found a newline, copy the line to nextLine. */
            strncpy(buffer->nextLine, (char *)buffer->buffer->bytes, i);
            buffer->nextLine[i] = '\0';

            /* Check for a carriage return before the line feed */
            if (buffer->nextLine[i - 1] == '\r')
            {
                buffer->nextLine[i - 1] = '\0';
            }

            // Check for a carriage return after the line feed */
            if (buffer->buffer->bytes[i + 1] == '\r')
            {
                /* This causes the memmove to remove the carriage return*/
                i++;
            }

            /* Remove the line from the buffer. */
            memmove(buffer->buffer->bytes, buffer->buffer->bytes + i + 1,
                buffer->buffer->length - i - 1);
            buffer->buffer->length -= (i + 1);
            buffer->buffer->tail -= (i + 1);
            return TRUE;
        }
    }
    return FALSE;
}

void ClearNextLine(InputBuffer *buffer)
{
    memset(buffer->nextLine, 0, sizeof(buffer->nextLine));
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
