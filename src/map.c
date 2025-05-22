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
#include <vbbs/list.h>
#include <vbbs/crc.h>
#include <string.h>
#include <stdlib.h>

MapEntry *NewMapEntry(const char *key, void *value, 
    ListItemDestructor valueDestructor)
{
    size_t keyLength;
    MapEntry *entry;

    if (!key) 
    {
        return NULL;
    }

    keyLength = strlen(key);

    entry = malloc(sizeof(MapEntry));
    if (!entry) 
    {
        return NULL;
    }

    entry->key = malloc(keyLength + 1);
    if (!entry->key) 
    {
        free(entry);
        return NULL;
    }
    memcpy(entry->key, key, keyLength);
    entry->key[keyLength] = '\0';

    entry->value = value;
    entry->valueDestructor = valueDestructor;

    return entry;
}

void MapEntryDestructor(void *item)
{
    DestroyMapEntry((MapEntry *)item);
}

void DestroyMapEntry(MapEntry *entry)
{
    if (entry == NULL) 
    {
        return;
    }

    if (entry->key != NULL) 
    {
        free(entry->key);
        entry->key = NULL;
    }

    if (entry->value)
    {
        if (entry->valueDestructor) 
        {
            entry->valueDestructor(entry->value);
        }
        else 
        {
            free(entry->value);
        }
        entry->value = NULL;
    }
    free(entry);
}

Map *NewMap(ListItemDestructor valueDestructor)
{
    int i, j;
    Map *map = malloc(sizeof(Map));
    if (!map) 
    {
        return NULL;
    }

    map->valueDestructor = valueDestructor;

    /** TODO: Make map self-balancing with fewer starting buckets. */
    map->bucketCount = 32;
    map->buckets = malloc(sizeof(ArrayList *) * map->bucketCount);
    if (!map->buckets) 
    {
        free(map);
        return NULL;
    }

    for (i = 0; i < map->bucketCount; i++) 
    {
        map->buckets[i] = NewArrayList(5, MapEntryDestructor);
        if (!map->buckets[i]) 
        {
            for (j = 0; j < i; j++) 
            {
                DestroyArrayList(map->buckets[j]);
            }
            free(map->buckets);
            free(map);
            return NULL;
        }
    }

    map->size = 0;

    return map;
}

void DestroyMap(Map *map)
{
    int i;
    if (!map) 
    {
        return;
    }

    if (map->buckets) 
    {
        for (i = 0; i < map->bucketCount; i++) 
        {
            DestroyArrayList(map->buckets[i]);
        }
        free(map->buckets);
        map->buckets = NULL;
    }
    free(map);
}

void MapPut(Map *map, const char *key, void *value)
{
    MapPutWithDestructor(map, key, value, map->valueDestructor);
}

void MapPutWithDestructor(Map *map, const char *key, void *value,
    ListItemDestructor valueDestructor)
{
    int i;
    int bucketIndex;
    ArrayList *bucket = NULL;
    MapEntry *entry = NULL;

    if (!map || !key) 
    {
        return;
    }

    bucketIndex = Checksum(key, strlen(key)) % map->bucketCount;
    bucket = map->buckets[bucketIndex];

    for (i = 0; i < bucket->size; i++) 
    {
        entry = (MapEntry *)GetFromArrayList(bucket, i);
        if (entry && strcmp(entry->key, key) == 0) 
        {
            if (entry->value)
            {
                entry->valueDestructor ? entry->valueDestructor(entry->value) 
                : free(entry->value);
            }
            entry->value = value;
            return;
        }
    }

    entry = NewMapEntry(key, value, valueDestructor);
    if (!entry) 
    {
        return;
    }
    AddToArrayList(bucket, entry);
    map->size++;
}

void *MapGet(const Map *map, const char *key)
{
    int i;
    int bucketIndex;
    ArrayList *bucket = NULL;
    MapEntry *entry = NULL;

    if (!map || !key) 
    {
        return NULL;
    }

    bucketIndex = Checksum(key, strlen(key)) % map->bucketCount;
    bucket = map->buckets[bucketIndex];

    for (i = 0; i < bucket->size; i++) 
    {
        entry = (MapEntry *)GetFromArrayList(bucket, i);
        if (entry && strcmp(entry->key, key) == 0) 
        {
            return entry->value;
        }
    }

    return NULL;
}

void MapRemove(Map *map, const char *key)
{
    int i;
    int bucketIndex;
    ArrayList *bucket = NULL;
    MapEntry *entry = NULL;

    if (!map || !key) 
    {
        return;
    }

    bucketIndex = Checksum(key, strlen(key)) % map->bucketCount;
    bucket = map->buckets[bucketIndex];

    for (i = 0; i < bucket->size; i++) 
    {
        entry = (MapEntry *)GetFromArrayList(bucket, i);
        if (entry && strcmp(entry->key, key) == 0) 
        {
            RemoveFromArrayList(bucket, i);
            DestroyMapEntry(entry);
            map->size--;
            return;
        }
    }
}

void MapClear(Map *map)
{
    int i;
    ArrayList *bucket = NULL;

    if (map == NULL || map->buckets == NULL) 
    {
        return;
    }

    for (i = 0; i < map->bucketCount; i++) 
    {
        bucket = map->buckets[i];
        ClearArrayList(bucket);
    }
    map->size = 0;
}
