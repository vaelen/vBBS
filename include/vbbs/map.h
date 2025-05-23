#ifndef VBBS_MAP_H
#define VBBS_MAP_H

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
#include <vbbs/list.h>
#include <vbbs/crc.h>

typedef struct MapEntry {
    char *key;
    void *value;
    ListItemDestructor valueDestructor;
} MapEntry;

typedef struct Map {
    ArrayList **buckets;
    size_t size;
    int bucketCount;
    ListItemDestructor valueDestructor;
} Map;

MapEntry *NewMapEntry(const char *key, void *value, 
    ListItemDestructor valueDestructor);
void MapEntryDestructor(void *item);
void DestroyMapEntry(MapEntry *entry);

Map *NewMap(ListItemDestructor valueDestructor);
void DestroyMap(Map *map);
void MapPut(Map *map, const char *key, void *value);
void MapPutWithDestructor(Map *map, const char *key, void *value, 
    ListItemDestructor valueDestructor);
void *MapGet(const Map *map, const char *key);
void MapRemove(Map *map, const char *key);
void MapClear(Map *map);
bool MapContainsKey(const Map *map, const char *key);
bool MapContainsValue(const Map *map, const void *value,
    ListItemComparator comparator);

#endif
