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

#define TEST_STRING "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define SUCCESS "\033[0;32mSuccess\033[0m"
#define FAILURE "\033[0;31m\033[5mFailed\033[0m"

void testIsBufferEmpty(void) {
    Buffer *buffer = NewBuffer(10);
    if (buffer == NULL) {
        printf("Failed to create buffer\n");
        return;
    }
    if (IsBufferEmpty(buffer)) {
        printf("           IsBufferEmpty: %s\n", SUCCESS);
    } else {
        printf("           IsBufferEmpty: %s\n", FAILURE);
    }
    DestroyBuffer(buffer);
}

void testIsBufferFull(void) {
    Buffer *buffer = NewBuffer(2);
    if (buffer == NULL) {
        printf("Failed to create buffer\n");
        return;
    }
    WriteToBuffer(buffer, TEST_STRING, 2);
    if (IsBufferFull(buffer)) {
        printf("            IsBufferFull: %s\n", SUCCESS);
    } else {
        printf("            IsBufferFull: %s\n", FAILURE);
    }
    DestroyBuffer(buffer);
}

void testClearBuffer(void) {
    Buffer *buffer = NewBuffer(strlen(TEST_STRING));
    if (buffer == NULL) {
        printf("Failed to create buffer\n");
        return;
    }
    WriteToBuffer(buffer, TEST_STRING, strlen(TEST_STRING));
    ClearBuffer(buffer);
    if (IsBufferEmpty(buffer)) {
        printf("             ClearBuffer: %s\n", SUCCESS);
    } else {
        printf("             ClearBuffer: %s\n", FAILURE);
    }
    DestroyBuffer(buffer);
}

void testReadWriteBuffer(void) {
    int bytesWritten = 0;
    uint8_t data[26];
    Buffer *buffer = NewBuffer(strlen(TEST_STRING));
    if (buffer == NULL) {
        printf("Failed to create buffer\n");
        return;
    }
    bytesWritten = WriteToBuffer(buffer, TEST_STRING, strlen(TEST_STRING));
    if (bytesWritten != strlen(TEST_STRING)) {
        printf("           WriteToBuffer: %s (Expected %lu, found %d)\n", 
            FAILURE, strlen(TEST_STRING), bytesWritten);
    } else {
        printf("           WriteToBuffer: %s\n", SUCCESS);
    }
    
    ReadFromBuffer(buffer, data, strlen(TEST_STRING));
    if (memcmp(data, TEST_STRING, strlen(TEST_STRING)) == 0) {
        printf("          ReadFromBuffer: %s\n", SUCCESS);
    } else {
        printf("          ReadFromBuffer: %s\n", FAILURE);
    }

    if (IsBufferEmpty(buffer)) {
        printf("             ShiftBuffer: %s\n", SUCCESS);
    } else {
        printf("             ShiftBuffer: %s\n", FAILURE);
    }
    DestroyBuffer(buffer);
}

void testBufferOverflow(void) {
    int bytesWritten = 0;
    Buffer *buffer = NewBuffer(strlen(TEST_STRING)-1);
    if (buffer == NULL) {
        printf("Failed to create buffer\n");
        return;
    }
    bytesWritten = WriteToBuffer(buffer, TEST_STRING, strlen(TEST_STRING));
    if (bytesWritten < 0) {
        printf("  WriteToBuffer Overflow: %s\n", SUCCESS);
    } else {
        printf("  WriteToBuffer Overflow: %s (%d)\n", FAILURE, bytesWritten);
    }
    DestroyBuffer(buffer);
}

int main(void) {
    testIsBufferEmpty();
    testIsBufferFull();
    testClearBuffer();
    testReadWriteBuffer();
    testBufferOverflow();
    printf("\n");
    return 0;
}
