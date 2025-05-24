#ifndef VBBS_RINGBUFFER_H
#define VBBS_RINGBUFFER_H

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

/** 
 * A FIFO queue with a limited size that overwrites the oldest member
 * when full.
 */
typedef struct
{
    uint8_t *buffer;     /* Pointer to the buffer */
    int head;         /* Index of the head of the buffer */
    int tail;         /* Index of the tail of the buffer */
    int size;         /* Current size of the buffer */
    int maxSize;      /* Maximum size of the buffer */
} RingBuffer;

RingBuffer* NewRingBuffer(int size);
void DestroyRingBuffer(RingBuffer *rb);
void WriteRingBuffer(RingBuffer *rb, const uint8_t *data, int size);
void ReadRingBuffer(RingBuffer *rb, uint8_t *data, int size);
void ClearRingBuffer(RingBuffer *rb);
bool IsRingBufferEmpty(RingBuffer *rb);
bool IsRingBufferFull(RingBuffer *rb);
uint8_t PeekRingBuffer(RingBuffer *rb);
void PushRingBuffer(RingBuffer *rb, uint8_t byte);
uint8_t PopRingBuffer(RingBuffer *rb);
void WriteStringToRingBuffer(RingBuffer *rb, const char *str);

#endif
