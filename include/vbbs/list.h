#ifndef _VBBS_LIST_H
#define _VBBS_LIST_H
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

typedef void (*ListItemDestructor)(void *item);
typedef bool (*ListItemComparator)(const void *item1, const void *item2);

typedef struct ArrayList
{
    void **items;
    int size;
    int capacity;
    ListItemDestructor destructor;
} ArrayList;

/** 
 * If no destructor is provided, the calling code must handle memory
 * management of the items in the list. 
 */
ArrayList *NewArrayList(int initialCapacity, ListItemDestructor destructor);
void DestroyArrayList(ArrayList *list);
void AddToArrayList(ArrayList *list, void *item);
void *GetFromArrayList(ArrayList *list, int index);
void RemoveFromArrayList(ArrayList *list, int index);
void ClearArrayList(ArrayList *list);
bool IsArrayListEmpty(ArrayList *list);
int ArrayListSize(ArrayList *list);
/** 
 * Check for the existance of a given value in the list.
 * If comparator is null, it will compare pointers. 
 */
bool ArrayListContains(const ArrayList *list, const void *item, 
    ListItemComparator comparator);

bool DefaultListItemComparator(const void *item1, const void *item2);
bool IntListItemComparator(const void *item1, const void *item2);
bool UIntListItemComparator(const void *item1, const void *item2);
bool StringListItemComparator(const void *item1, const void *item2);
bool StringListItemComparatorIgnoreCase(const void *item1, const void *item2);
bool UInt8ListItemComparator(const void *item1, const void *item2);
bool UInt16ListItemComparator(const void *item1, const void *item2);
bool UInt32ListItemComparator(const void *item1, const void *item2);
bool Int8ListItemComparator(const void *item1, const void *item2);
bool Int16ListItemComparator(const void *item1, const void *item2);
bool Int32ListItemComparator(const void *item1, const void *item2);


#endif /* _VBBS_LIST_H */
