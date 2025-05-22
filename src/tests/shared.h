#ifndef _TESTS_SHARED_H
#define _TESTS_SHARED_H

#include <vbbs/types.h>

#define SUCCESS "\033[0;32mSuccess\033[0m"
#define FAILURE "\033[0;31m\033[5mFailed\033[0m"

void printTestResult(const char *testName, bool passed);
void runAllListTests(void);
void runAllBufferTests(void);
void runAllCRCTests(void);
void runAllRingBufferTests(void);
void runAllMapTests(void);

#endif
