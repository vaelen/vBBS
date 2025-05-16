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
#include <stdlib.h>

ArrayList *NewArrayList(int initialCapacity, ListItemDestructor destructor)
{
    ArrayList *list = (ArrayList *)malloc(sizeof(ArrayList));
    if (list == NULL)
    {
        return NULL;
    }

    list->items = (void **)malloc(sizeof(void *) * initialCapacity);
    if (list->items == NULL)
    {
        free(list);
        return NULL;
    }

    list->size = 0;
    list->capacity = initialCapacity;
    list->destructor = destructor;
    return list;
}

void DestroyArrayList(ArrayList *list)
{
    if (list == NULL)
    {
        return;
    }

    ClearArrayList(list);

    free(list->items);
    free(list);
}

void AddToArrayList(ArrayList *list, void *item)
{
    if (list->size >= list->capacity)
    {
        list->capacity *= 2;
        list->items = 
            (void **)realloc(list->items, sizeof(void *) * list->capacity);
    }

    list->items[list->size] = item;
    list->size++;
}

void *GetFromArrayList(ArrayList *list, int index)
{
    if (index < 0 || index >= list->size)
    {
        return NULL;
    }

    return list->items[index];
}

void RemoveFromArrayList(ArrayList *list, int index)
{
    int i;

    if (index < 0 || index >= list->size)
    {
        return;
    }

    if (list->destructor != NULL)
    {
        list->destructor(list->items[index]);
    }
    else
    {
        free(list->items[index]);
    }

    for (i = index; i < list->size - 1; i++)
    {
        list->items[i] = list->items[i + 1];
    }

    list->size--;
}

void ClearArrayList(ArrayList *list)
{
    int i;

    for (i = 0; i < list->size; i++)
    {
        if (list->destructor != NULL)
        {
            list->destructor(list->items[i]);
        }
        else
        {
            free(list->items[i]);
        }
    }

    list->size = 0;
}
