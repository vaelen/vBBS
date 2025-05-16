#ifndef VBBS_TYPES_H
#define VBBS_TYPES_H

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

#if defined(__STDC__)
# define STANDARD_C_1989
# if defined(__STDC_VERSION__)
#  define STANDARD_C_1990
#  if (__STDC_VERSION__ >= 199409L)
#   define PSTANDARD_C_1994
#  endif
#  if (__STDC_VERSION__ >= 199901L)
#   define STANDARD_C_1999
#  endif
#  if (__STDC_VERSION__ >= 201112L)
#    define STANDARD_C_2011
#  endif
#  if (__STDC_VERSION__ >= 201710L)
#   define STANDARD_C_2018
#  endif
# endif
#endif

#include <stddef.h>
#include <limits.h>

#ifdef __unix__
#include <unistd.h>
#endif

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

enum {
    FALSE = 0,
    TRUE = 1
};

typedef unsigned char bool;

#ifdef STANDARD_C_1999
#include <stdint.h>
#else

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

#if __ULONG_MAX__ == 4294967295
typedef signed long int int32_t;
typedef unsigned long int uint32_t;
#else
typedef signed int int32_t;
typedef unsigned int uint32_t;
#endif /* 32bit long check */

#endif /* STANDARD_C_1999 */

#endif
