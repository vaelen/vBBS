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

#include "shared.h"

#define TEST_STRING "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define SUCCESS "\033[0;32mSuccess\033[0m"
#define FAILURE "\033[0;31m\033[5mFailed\033[0m"

void testIsRingBufferEmpty(void) {
    RingBuffer *rb = NewRingBuffer(10);
    if (rb == NULL) {
        printf("Failed to create ring buffer\n");
        printTestResult("testIsRingBufferEmpty", FALSE);
        return;
    }
    if (IsRingBufferEmpty(rb)) {
        printTestResult("testIsRingBufferEmpty", TRUE);
    } else {
        printTestResult("testIsRingBufferEmpty", FALSE);
    }
    DestroyRingBuffer(rb);
}

void testIsRingBufferFull(void) {
    RingBuffer *rb = NewRingBuffer(2);
    if (rb == NULL) {
        printf("Failed to create ring buffer\n");
        printTestResult("testIsRingBufferFull", FALSE);
        return;
    }
    PushRingBuffer(rb, TEST_STRING[0]);
    PushRingBuffer(rb, TEST_STRING[1]);
    if (IsRingBufferFull(rb)) {
        printTestResult("testIsRingBufferFull", TRUE);
    } else {
        printTestResult("testIsRingBufferFull", FALSE);
    }
    DestroyRingBuffer(rb);
}

void testClearRingBuffer(void) {
    RingBuffer *rb = NewRingBuffer(strlen(TEST_STRING));
    if (rb == NULL) {
        printf("Failed to create ring buffer\n");
        printTestResult("testClearRingBuffer", FALSE);
        return;
    }
    WriteRingBuffer(rb, (const uint8_t *)TEST_STRING, strlen(TEST_STRING));
    ClearRingBuffer(rb);
    if (IsRingBufferEmpty(rb)) {
        printTestResult("testClearRingBuffer", TRUE);
    } else {
        printTestResult("testClearRingBuffer", FALSE);
    }
    DestroyRingBuffer(rb);
}

void testReadWriteRingBuffer(void) {
    uint8_t data[26];
    RingBuffer *rb = NewRingBuffer(strlen(TEST_STRING));
    if (rb == NULL) {
        printf("Failed to create ring buffer\n");
        printTestResult("testReadWriteRingBuffer", FALSE);
        return;
    }
    WriteRingBuffer(rb, (const uint8_t *)TEST_STRING, strlen(TEST_STRING));
    ReadRingBuffer(rb, data, strlen(TEST_STRING));
    if (memcmp(data, TEST_STRING, strlen(TEST_STRING)) == 0) {
        printTestResult("testReadWriteRingBuffer", TRUE);
    } else {
        printTestResult("testReadWriteRingBuffer", FALSE);
    }
    DestroyRingBuffer(rb);
}

void testRingBufferOverflow(void) {
    uint8_t data[sizeof(TEST_STRING)];
    RingBuffer *rb = NewRingBuffer(strlen(TEST_STRING));
    if (rb == NULL) {
        printf("Failed to create ring buffer\n");
        printTestResult("testRingBufferOverflow", FALSE);
        return;
    }
    WriteRingBuffer(rb, (const uint8_t *)TEST_STRING, strlen(TEST_STRING));
    ReadRingBuffer(rb, data, strlen(TEST_STRING));
    if (memcmp(data, TEST_STRING, strlen(TEST_STRING)) == 0) {
        printTestResult("testRingBufferOverflow", TRUE);
    } else {
        printTestResult("testRingBufferOverflow", FALSE);
    }
    DestroyRingBuffer(rb);
}

void testRingBufferPushPop(void) {
    uint8_t b, i;
    RingBuffer *rb = NewRingBuffer(10);
    if (rb == NULL) {
        printf("Failed to create ring buffer\n");
        printTestResult("testRingBufferPushPop", FALSE);
        return;
    }
    PushRingBuffer(rb, TEST_STRING[0]);
    PushRingBuffer(rb, TEST_STRING[1]);
    PushRingBuffer(rb, TEST_STRING[2]);
    for (i = 0; i < 3; i++) {
        b = PopRingBuffer(rb);
        if (b == TEST_STRING[i]) {
            printTestResult("testRingBufferPushPop", TRUE);
        } else {
            printTestResult("testRingBufferPushPop", FALSE);
        }
    }
    DestroyRingBuffer(rb);
}

void runAllRingBufferTests(void) {
    printf("Running Ring Buffer Tests...\n");
    testIsRingBufferEmpty();
    testRingBufferPushPop();
    testIsRingBufferFull();
    testClearRingBuffer();
    testReadWriteRingBuffer();
    testRingBufferOverflow();
    printf("\n");
}
