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
#include <vbbs/map.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"

void testNewMapEntry(void) {
    MapEntry *entry = NULL;
    int a = 42;
    int *pa = malloc(sizeof(int));
    *pa = a;
    entry = NewMapEntry("key1", pa, NULL);
    printTestResult("testNewMapEntry", entry != NULL && entry->key != NULL && 
        strcmp(entry->key, "key1") == 0 && *(int *)entry->value == 42);
    DestroyMapEntry(entry);
}

void testNewMap(void) {
    Map *map = NULL;
    map = NewMap(NULL);
    printTestResult("testNewMap", map != NULL && map->size == 0);
    DestroyMap(map);
}

void testMapPutAndGet(void) {
    Map *map = NewMap(NULL);
    int a = 42, b = 99;
    int *pa, *pb;
    pa = (int *)malloc(sizeof(int));
    *pa = a;
    pb = (int *)malloc(sizeof(int));
    *pb = b;
    MapPut(map, "key1", pa);
    MapPut(map, "key2", pb);
    pa = NULL;
    pb = NULL;
    pa = (int *)MapGet(map, "key1");
    pb = (int *)MapGet(map, "key2");
    printTestResult("testMapPutAndGet", pa && pb && *pa == 42 && *pb == 99 && 
        map->size == 2);
    DestroyMap(map);
}

void testMapReplace(void) {
    Map *map = NewMap(free);
    int a = 42, b = 99;
    int *pa, *pb;
    pa = (int *)malloc(sizeof(int));
    *pa = a;
    pb = (int *)malloc(sizeof(int));
    *pb = b;
    MapPut(map, "key1", pa);
    MapPut(map, "key2", pb);
    MapPut(map, "key1", pb);
    pa = NULL;
    pb = NULL;
    pa = (int *)MapGet(map, "key1");
    pb = (int *)MapGet(map, "key2");
    printTestResult("testMapReplace", pa == pb && *pa == 99 && *pb == 99 && 
        map->size == 2);
    DestroyMap(map);
}

void testMapRemove(void) {
    Map *map = NewMap(NULL);
    int a = 42, b = 99;
    int *pa, *pb;
    pa = (int *)malloc(sizeof(int));
    *pa = a;
    pb = (int *)malloc(sizeof(int));
    *pb = b;
    MapPut(map, "key1", pa);
    MapPut(map, "key2", pb);
    MapRemove(map, "key1");
    pa = NULL;
    pb = NULL;
    pa = (int *)MapGet(map, "key1");
    pb = (int *)MapGet(map, "key2");
    printTestResult("testMapRemove", pa == NULL && pb && *pb == 99 && 
        map->size == 1);
    DestroyMap(map);
}

void testMapClear(void) {
    Map *map = NewMap(NULL);
    int a = 42, b = 99;
    int *pa, *pb;
    pa = (int *)malloc(sizeof(int));
    *pa = a;
    pb = (int *)malloc(sizeof(int));
    *pb = b;
    MapPut(map, "key1", pa);
    MapPut(map, "key2", pb);
    MapClear(map);
    pa = NULL;
    pb = NULL;
    pa = (int *)MapGet(map, "key1");
    pb = (int *)MapGet(map, "key2");
    printTestResult("testMapClear", pa == NULL && pb == NULL && map->size == 0);
    DestroyMap(map);
}

void runAllMapTests(void) {
    printf("Running Map Tests...\n");
    testNewMapEntry();
    testNewMap();
    testMapPutAndGet();
    testMapReplace();
    testMapRemove();
    testMapClear();
    printf("\n");
}
