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
#include <string.h>
#include <stdio.h>

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

    if (list->items != NULL)
    {
        free(list->items);
        list->items = NULL;
    }
    free(list);
}

void AddToArrayList(ArrayList *list, void *item)
{
    if (list == NULL || list->items == NULL)
    {
        return;
    }

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
    if (list == NULL || list->items == NULL)
    {
        return NULL;
    }

    if (index < 0 || index >= list->size)
    {
        return NULL;
    }

    return list->items[index];
}

void RemoveFromArrayList(ArrayList *list, int index)
{
    int i;
    void *value;

    if (list == NULL || list->items == NULL)
    {
        return;
    }

    if (index < 0 || index >= list->size)
    {
        return;
    }

    value = list->items[index];

    for (i = index; i < list->size - 1; i++)
    {
        list->items[i] = list->items[i + 1];
    }

    list->size--;

    /* If there is another copy of this same pointer in the list,
        then don't free this copy. */
    if (!ArrayListContains(list, value, NULL) && list->destructor != NULL)
    {
        list->destructor(value);
    }

}

void ClearArrayList(ArrayList *list)
{
    if (list == NULL || list->items == NULL)
    {
        return;
    }

    while(!IsArrayListEmpty(list))
    {
        /* Remove the last item. 
            We do this so that we can reuse the pointer cleanup code
            from RemoveFromArrayList. */
        RemoveFromArrayList(list, list->size - 1);
    }

    list->size = 0;
}

bool IsArrayListEmpty(ArrayList *list)
{
    if (list == NULL)
    {
        return TRUE;
    }
    return list->size == 0;
}

int ArrayListSize(ArrayList *list)
{
    if (list == NULL)
    {
        return -1;
    }
    return list->size;
}

bool ArrayListContains(const ArrayList *list, const void *item,
    ListItemComparator comparator)
{
    int i;

    if (list == NULL || list->items == NULL)
    {
        return FALSE;
    }

    if (comparator == NULL)
    {
        comparator = DefaultListItemComparator;
    }

    for (i = 0; i < list->size; i++)
    {
        if (comparator(list->items[i], item) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

#define COMPARE_NULLS(item1, item2) \
    if (item1 == NULL && item2 == NULL) return 0; \
    if (item1 == NULL && item2 != NULL) return -1; \
    if (item1 != NULL && item2 == NULL) return 1;

int DefaultListItemComparator(const void *item1, const void *item2)
{
    COMPARE_NULLS(item1, item2);
    return item1 == item2 ? 0 : (item1 < item2 ? -1 : 1);
}

int IntListItemComparator(const void *item1, const void *item2)
{
    COMPARE_NULLS(item1, item2);
    return *(int *)item1 - *(int *)item2;
}

int UIntListItemComparator(const void *item1, const void *item2)
{
    COMPARE_NULLS(item1, item2);
    return *(unsigned int *)item1 - *(unsigned int *)item2;
}

int StringListItemComparator(const void *item1, const void *item2)
{
    COMPARE_NULLS(item1, item2);
    return strcmp((const char *)item1, (const char *)item2);
}

int StringListItemComparatorIgnoreCase(const void *item1, const void *item2)
{
    COMPARE_NULLS(item1, item2);
    return strcasecmp((const char *)item1, (const char *)item2);
}

int UInt8ListItemComparator(const void *item1, const void *item2)
{
    COMPARE_NULLS(item1, item2);
    return *(uint8_t *)item1 - *(uint8_t *)item2;
}

int UInt16ListItemComparator(const void *item1, const void *item2)
{
    COMPARE_NULLS(item1, item2);
    return *(uint16_t *)item1 - *(uint16_t *)item2;
}

int UInt32ListItemComparator(const void *item1, const void *item2)
{
    COMPARE_NULLS(item1, item2);
    return *(uint32_t *)item1 - *(uint32_t *)item2;
}

int Int8ListItemComparator(const void *item1, const void *item2)
{
    COMPARE_NULLS(item1, item2);
    return *(int8_t *)item1 - *(int8_t *)item2;
}

int Int16ListItemComparator(const void *item1, const void *item2)
{
    COMPARE_NULLS(item1, item2);
    return *(int16_t *)item1 - *(int16_t *)item2;
}

int Int32ListItemComparator(const void *item1, const void *item2)
{
    COMPARE_NULLS(item1, item2);
    return *(int32_t *)item1 - *(int32_t *)item2;
}

void SortArrayList(ArrayList *list, ListItemComparator comparator)
{
    if (list == NULL || list->items == NULL || list->size <= 1)
    {
        return;
    }

    if (comparator == NULL)
    {
        comparator = DefaultListItemComparator;
    }

    if (list->size < 5)
    {
        BubbleSort((void**)list->items, list->size, comparator);
        return;
    }

    QuickSort((void**)list->items, 0, list->size - 1, comparator);

}

void BubbleSort(void **array, int size, 
    ListItemComparator comparator)
{
    int i, j;
    void *temp;

    if (comparator == NULL)
    {
        comparator = DefaultListItemComparator;
    }

    for (i = 0; i < size - 1; i++)
    {
        for (j = 0; j < size - i - 1; j++)
        {
            if (comparator(array[j], array[j + 1]) > 0)
            {
                temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

void QuickSort(void **array, int left, int right, 
    ListItemComparator comparator)
{
    int i = left, j = right, p = (left + right) / 2;
    void *lo, *mid, *hi;
    void *pivot;
    void *temp;

    if (array == NULL || left < 0 || right < 0 || left >= right)
    {
        return;
    }

    if (comparator == NULL)
    {
        comparator = DefaultListItemComparator;
    }

    lo = array[left];
    mid = array[p];
    hi = array[right];

    // Choose the pivot as the median of the first, middle, and last elements
    if (comparator(mid, lo) < 0)
    {
        array[left] = mid;
        array[p] = lo;
        lo = array[left];
        mid = array[p];
        hi = array[right];
    }
    if (comparator(hi, lo) < 0)
    {
        array[left] = hi;
        array[right] = lo;
        lo = array[left];
        mid = array[p];
        hi = array[right];
    }
    if (comparator(hi, mid) < 0)
    {
        array[p] = hi;
        array[right] = mid;
        lo = array[left];
        mid = array[p];
        hi = array[right];
    }
    pivot = mid;

    while (i >= 0 && j >= 0 && i < j)
    {
        while (comparator(array[i], pivot) < 0)
        {
            i++;
        }
        while (comparator(pivot, array[j]) < 0)
        {
            j--;
        }
        if (i <= j)
        {
            // Swap elements
            temp = array[i];
            array[i] = array[j];
            array[j] = temp;
            i++;
            j--;
        }
    }

    if (left < j)
    {
        QuickSort(array, left, p, comparator);
    }
    if (i < right)
    {
        QuickSort(array, p+1, right, comparator);
    }
}

