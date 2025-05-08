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
#include <vbbs/rb.h>
#include <stdlib.h>

RingBuffer* NewRingBuffer(size_t size)
{
    RingBuffer *rb = (RingBuffer *)malloc(sizeof(RingBuffer));
    if (rb == NULL)
    {
        return NULL;
    }
    rb->buffer = (uint8_t *)malloc(size);
    if (rb->buffer == NULL)
    {
        free(rb);
        return NULL;
    }
    rb->head = 0;
    rb->tail = 0;
    rb->size = 0;
    rb->maxSize = size;
    return rb;
}

void DestroyRingBuffer(RingBuffer *rb)
{
    if (rb->buffer != NULL)
    {
        free(rb->buffer);
        rb->buffer = NULL;
    }
    free(rb);
}

void WriteRingBuffer(RingBuffer *rb, const uint8_t *data, size_t size)
{
    size_t i;
    for (i = 0; i < size; i++)
    {
        PushRingBuffer(rb, data[i]);
    }
}

void ReadRingBuffer(RingBuffer *rb, uint8_t *data, size_t size)
{
    size_t i;
    for (i = 0; i < size; i++)
    {
        data[i] = PopRingBuffer(rb);
    }
}

void ClearRingBuffer(RingBuffer *rb)
{
    rb->head = 0;
    rb->tail = 0;
    rb->size = 0;
}

bool IsRingBufferEmpty(RingBuffer *rb)
{
    return rb->size == 0;
}

bool IsRingBufferFull(RingBuffer *rb)
{
    return rb->size == rb->maxSize;
}

/** Push a value onto the end of the buffer. */
void PushRingBuffer(RingBuffer *rb, uint8_t byte)
{
    rb->buffer[rb->head] = byte;
    rb->head = (rb->head + 1) % rb->maxSize;
    if (rb->size < rb->maxSize)
    {
        rb->size++;
    }
    else
    {
        /* Overwrite the last byte */
        rb->tail = (rb->tail + 1) % rb->maxSize;
    }
}

/** Pop a value off of the front of the buffer. */
uint8_t PopRingBuffer(RingBuffer *rb)
{
    uint8_t byte = 0;
    if (rb->size > 0)
    {
        byte = rb->buffer[rb->tail];
        rb->tail = (rb->tail + 1) % rb->maxSize;
        rb->size--;
    }
    return byte;
}
