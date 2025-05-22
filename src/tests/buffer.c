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
#include <stdio.h>
#include <string.h>

#include "shared.h"

#define TEST_STRING "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define SUCCESS "\033[0;32mSuccess\033[0m"
#define FAILURE "\033[0;31m\033[5mFailed\033[0m"

void testIsBufferEmpty(void) {
    Buffer *buffer = NewBuffer(10);
    if (buffer == NULL) {
        printf("Failed to create buffer\n");
        printTestResult("testIsBufferEmpty", FALSE);
        return;
    }
    if (IsBufferEmpty(buffer)) {
        printTestResult("testIsBufferEmpty", TRUE);
    } else {
        printTestResult("testIsBufferEmpty", FALSE);
    }
    DestroyBuffer(buffer);
}

void testIsBufferFull(void) {
    Buffer *buffer = NewBuffer(2);
    if (buffer == NULL) {
        printf("Failed to create buffer\n");
        printTestResult("testIsBufferFull", FALSE);
        return;
    }
    WriteToBuffer(buffer, TEST_STRING, 2);
    if (IsBufferFull(buffer)) {
        printTestResult("testIsBufferFull", TRUE);
    } else {
        printTestResult("testIsBufferFull", FALSE);
    }
    DestroyBuffer(buffer);
}

void testClearBuffer(void) {
    Buffer *buffer = NewBuffer(strlen(TEST_STRING));
    if (buffer == NULL) {
        printf("Failed to create buffer\n");
        printTestResult("testClearBuffer", FALSE);
        return;
    }
    WriteToBuffer(buffer, TEST_STRING, strlen(TEST_STRING));
    ClearBuffer(buffer);
    if (IsBufferEmpty(buffer)) {
        printTestResult("testClearBuffer", TRUE);
    } else {
        printTestResult("testClearBuffer", FALSE);
    }
    DestroyBuffer(buffer);
}

void testReadWriteBuffer(void) {
    int bytesWritten = 0;
    uint8_t data[26];
    Buffer *buffer = NewBuffer(strlen(TEST_STRING));
    if (buffer == NULL) {
        printf("Failed to create buffer\n");
        printTestResult("testReadWriteBuffer", FALSE);
        return;
    }
    bytesWritten = WriteToBuffer(buffer, TEST_STRING, strlen(TEST_STRING));
    if (bytesWritten != strlen(TEST_STRING)) {
        printTestResult("testWriteToBuffer", FALSE);
    } else {
        printTestResult("testWriteToBuffer", TRUE);
    }
    
    ReadFromBuffer(buffer, data, strlen(TEST_STRING));
    if (memcmp(data, TEST_STRING, strlen(TEST_STRING)) == 0) {
        printTestResult("testReadFromBuffer", TRUE);
    } else {
        printTestResult("testReadFromBuffer", FALSE);
    }

    if (IsBufferEmpty(buffer)) {
        printTestResult("testShiftBuffer", TRUE);
    } else {
        printTestResult("testShiftBuffer", FALSE);
    }
    DestroyBuffer(buffer);
}

void testBufferOverflow(void) {
    int bytesWritten = 0;
    Buffer *buffer = NewBuffer(strlen(TEST_STRING)-1);
    if (buffer == NULL) {
        printf("Failed to create buffer\n");
        printTestResult("testBufferOverflow", FALSE);
        return;
    }
    bytesWritten = WriteToBuffer(buffer, TEST_STRING, strlen(TEST_STRING));
    if (bytesWritten < 0) {
        printTestResult("testBufferOverflow", TRUE);
    } else {
        printTestResult("testBufferOverflow", FALSE);
    }
    DestroyBuffer(buffer);
}

void runAllBufferTests(void) {
    printf("Running Buffer Tests...\n");
    testIsBufferEmpty();
    testIsBufferFull();
    testClearBuffer();
    testReadWriteBuffer();
    testBufferOverflow();
    printf("\n");
}
