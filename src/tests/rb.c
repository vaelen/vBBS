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
#include <stdio.h>
#include <string.h>

#define TEST_STRING "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define SUCCESS "\033[0;32mSuccess\033[0m"
#define FAILURE "\033[0;31m\033[5mFailed\033[0m"

void testIsRingBufferEmpty(void) {
    RingBuffer *rb = NewRingBuffer(10);
    if (rb == NULL) {
        printf("Failed to create ring buffer\n");
        return;
    }
    if (IsRingBufferEmpty(rb)) {
        printf("       IsRingBufferEmpty: %s\n", SUCCESS);
    } else {
        printf("       IsRingBufferEmpty: %s\n", FAILURE);
    }
    DestroyRingBuffer(rb);
}

void testIsRingBufferFull(void) {
    RingBuffer *rb = NewRingBuffer(2);
    if (rb == NULL) {
        printf("Failed to create ring buffer\n");
        return;
    }
    PushRingBuffer(rb, TEST_STRING[0]);
    PushRingBuffer(rb, TEST_STRING[1]);
    if (IsRingBufferFull(rb)) {
        printf("        IsRingBufferFull: %s\n", SUCCESS);
    } else {
        printf("        IsRingBufferFull: %s\n", FAILURE);
    }
    DestroyRingBuffer(rb);
}

void testClearRingBuffer(void) {
    RingBuffer *rb = NewRingBuffer(strlen(TEST_STRING));
    if (rb == NULL) {
        printf("Failed to create ring buffer\n");
        return;
    }
    WriteRingBuffer(rb, (const uint8_t *)TEST_STRING, strlen(TEST_STRING));
    ClearRingBuffer(rb);
    if (IsRingBufferEmpty(rb)) {
        printf("         ClearRingBuffer: %s\n", SUCCESS);
    } else {
        printf("         ClearRingBuffer: %s\n", FAILURE);
    }
    DestroyRingBuffer(rb);
}

void testReadWriteRingBuffer(void) {
    uint8_t data[26];
    RingBuffer *rb = NewRingBuffer(strlen(TEST_STRING));
    if (rb == NULL) {
        printf("Failed to create ring buffer\n");
        return;
    }
    WriteRingBuffer(rb, (const uint8_t *)TEST_STRING, strlen(TEST_STRING));
    ReadRingBuffer(rb, data, strlen(TEST_STRING));
    if (memcmp(data, TEST_STRING, strlen(TEST_STRING)) == 0) {
        printf("    Read/WriteRingBuffer: %s\n", SUCCESS);
    } else {
        printf("    Read/WriteRingBuffer: %s\n", FAILURE);
    }
    DestroyRingBuffer(rb);
}

void testRingBufferOverflow(void) {
    uint8_t data[sizeof(TEST_STRING)];
    RingBuffer *rb = NewRingBuffer(strlen(TEST_STRING));
    if (rb == NULL) {
        printf("Failed to create ring buffer\n");
        return;
    }
    WriteRingBuffer(rb, (const uint8_t *)TEST_STRING, strlen(TEST_STRING));
    ReadRingBuffer(rb, data, strlen(TEST_STRING));
    if (memcmp(data, TEST_STRING, strlen(TEST_STRING)) == 0) {
        printf("     ReadBuffer Overflow: %s\n", SUCCESS);
    } else {
        printf("     ReadBuffer Overflow: %s\n", FAILURE);
    }
    DestroyRingBuffer(rb);
}

void testRingBufferPushPop(void) {
    uint8_t b, i;
    RingBuffer *rb = NewRingBuffer(10);
    if (rb == NULL) {
        printf("Failed to create ring buffer\n");
        return;
    }
    PushRingBuffer(rb, TEST_STRING[0]);
    PushRingBuffer(rb, TEST_STRING[1]);
    PushRingBuffer(rb, TEST_STRING[2]);
    for (i = 0; i < 3; i++) {
        b = PopRingBuffer(rb);
        if (b == TEST_STRING[i]) {
            printf("  Push/PopRingBuffer (%d): %s\n", i, SUCCESS);
        } else {
            printf("  Push/PopRingBuffer (%d): %s\n", i, FAILURE);
        }
    }
    DestroyRingBuffer(rb);
}

int main(void) {
    testIsRingBufferEmpty();
    testRingBufferPushPop();
    testIsRingBufferFull();
    testClearRingBuffer();
    testReadWriteRingBuffer();
    testRingBufferOverflow();
    printf("\n");
    return 0;
}
