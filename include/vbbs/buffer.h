#ifndef _VBBS_BUFFER_H
#define _VBBS_BUFFER_H

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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* WindowSizeHandler is called when the window size is received. */
typedef void (*WindowSizeHandler)(void *userData, int width, int height);
/* TerminalTypeHanlder is called when the terminal type is received. */
typedef void (*TerminalTypeHandler)(void *userData, const char *type);
/* ConnectionSpeedHandler is called when the connection speed is received. */
typedef void (*ConnectionSpeedHandler)(void *userData, int speed);

typedef struct Buffer {
   uint8_t *bytes; /* The underlying byte array */
   uint8_t *tail;  /* A pointer to the end of the buffer */
   int length;    /* The current size of the buffer */
   int maxSize; /* The maximum size of the buffer */
   bool convertNewlines; /* Whether to convert newlines */
   bool handleANSI; /* Whether to handle ANSI escape codes */
   bool handleTelnet; /* Whether to handle Telnet commands */
   bool inANSI; /* True when currently processing an ANSI escape sequence */
   int inTelnet; /* True when currently processing a Telnet command */
   int inTelnetSB; /* True when in a Telnet SE sequence */
   uint8_t commandBuffer[256]; /* Buffer for ANSI/Telnet commands */
   void *userData; /* User data for custom handlers */
   WindowSizeHandler windowSize; /* Handler for window size changes */
   TerminalTypeHandler terminalType; /* Handler for terminal type */
   ConnectionSpeedHandler connectionSpeed; /* Handler for connection speed */
} Buffer;

Buffer* NewBuffer(int size);
void DestroyBuffer(Buffer *buffer);
void ClearBuffer(Buffer *buffer);
bool IsBufferEmpty(Buffer *buffer);
bool IsBufferFull(Buffer *buffer);
int BufferRemaining(Buffer *buffer);
int WriteToBuffer(Buffer *buffer, const char *data, int length);
int WriteStringToBuffer(Buffer *buffer, const char *data);
int WriteBufferToStream(Buffer *buffer, FILE *out);
int ReadFromBuffer(Buffer *buffer, uint8_t *data, int length);
void BytesToHexString(char *bytes, int bytesSize, char *out, int outSize);
/** 
 * ShiftBuffer shifts the contents of the buffer by the specified offset.
 * Another way to think of this is that it removes the first offset bytes.
 * If the offset is greater than or equal to the length of the buffer, 
 * the buffer is cleared. 
 */
void ShiftBuffer(Buffer *buffer, int offset);

/**
 * RemoveFromBuffer removes a specified number of bytes from the buffer
 * starting at the given offset. If the offset is greater than or equal to
 * the length of the buffer, it does nothing.
 */
void RemoveFromBuffer(Buffer *buffer, int offset, int length);

/**
 * Remove non-printable characters from a string.
 */
void CleanString(char *str);

typedef enum {
   LINE_INPUT_MODE,
   CHARACTER_INPUT_MODE
} InputMode;

/** 
 * A Buffer wraps a Buffer and adds features that are
 * useful for processing input.
 */
typedef struct InputBuffer {
   Buffer *buffer;
   InputMode inputMode;
   char nextLine[256];
   bool nextLineReady;
} InputBuffer;

InputBuffer* NewInputBuffer(int size);
void DestroyInputBuffer(InputBuffer *buffer);
int ReadDataFromStream(InputBuffer *buffer, FILE *in);
bool IsNextLineReady(InputBuffer *buffer);
void ClearNextLine(InputBuffer *buffer);
void SetInputMode(InputBuffer *buffer, InputMode mode);

bool FindNextLine(InputBuffer *buffer);
bool FindNextCharacter(InputBuffer *buffer);

#endif
