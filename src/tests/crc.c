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
#include <vbbs/crc.h>
#include <stdio.h>
#include <string.h>

#define TEST_STRING "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define EXPECTED_CHECKSUM 0x1B
#define EXPECTED_CRC16_CCITT 0xB65E
#define EXPECTED_CRC16_XMODEM 0xE8AF

#define SUCCESS "\033[0;32mSuccess\033[0m"
#define FAILURE "\033[0;31m\033[5mFailed\033[0m"

void testChecksum(void) {
    uint8_t chk = ChecksumS(TEST_STRING);
    printf("    Checksum: Expected:  0x%02X, Actual:   0x%02X\t%s\n", 
        EXPECTED_CHECKSUM, chk,
        chk == EXPECTED_CHECKSUM ? SUCCESS : FAILURE);
}


void testCRC16_CCITT(void) {
    uint16_t crc = CRC16S(CRC16_CCITT, TEST_STRING);
    printf(" CRC16/CCITT: Expected:0x%04X, Actual: 0x%04X\t%s\n", 
        EXPECTED_CRC16_CCITT, crc, 
        crc == EXPECTED_CRC16_CCITT ? SUCCESS : FAILURE);
}

void testCRC16_XMODEM(void) {
    uint16_t crc = CRC16S(CRC16_XMODEM, TEST_STRING);
    printf("CRC16/XMODEM: Expected:0x%04X, Actual: 0x%04X\t%s\n", 
        EXPECTED_CRC16_XMODEM, crc, 
        crc == EXPECTED_CRC16_XMODEM ? SUCCESS : FAILURE);
}

int main(void) {
    testChecksum();
    testCRC16_CCITT();
    testCRC16_XMODEM();
    printf("\n");
    return 0;
}
