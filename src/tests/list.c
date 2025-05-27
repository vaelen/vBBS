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

#include <vbbs.h>
#include "shared.h"

static void testNewArrayListAndDestroyArrayList(void) {
    ArrayList *list = NewArrayList(4, free);
    int passed = (list != NULL && list->size == 0 && list->capacity >= 4);
    DestroyArrayList(list);
    printTestResult("testNewArrayListAndDestroyArrayList", passed);
}

static void testAddToArrayListAndGetFromArrayList(void) {
    ArrayList *list = NewArrayList(2, free);
    int a = 42, b = 99;
    int *pa, *pb, passed;
    pa = (int *)malloc(sizeof(int));
    *pa = a;
    pb = (int *)malloc(sizeof(int));
    *pb = b;
    AddToArrayList(list, pa);
    AddToArrayList(list, pb);
    pa = NULL;
    pb = NULL;
    pa = (int *)GetFromArrayList(list, 0);
    pb = (int *)GetFromArrayList(list, 1);
    passed = (pa && pb && *pa == 42 && *pb == 99 && list->size == 2);
    DestroyArrayList(list);
    printTestResult("testAddToArrayListAndGetFromArrayList", passed);
}

static void testRemoveFromArrayList(void) {
    ArrayList *list = NewArrayList(2, free);
    int a = 1, b = 2, c = 3;
    int *pa, *pb, *pc;
    int *p0, *p1, passed;
    pa = (int *)malloc(sizeof(int));
    pb = (int *)malloc(sizeof(int));
    pc = (int *)malloc(sizeof(int));
    *pa = a;
    *pb = b;
    *pc = c;
    AddToArrayList(list, pa);
    AddToArrayList(list, pb);
    AddToArrayList(list, pc);
    RemoveFromArrayList(list, 1); /* remove b */
    p0 = (int *)GetFromArrayList(list, 0);
    p1 = (int *)GetFromArrayList(list, 1);
    passed = (list->size == 2 && p0 && p1 && *p0 == 1 && *p1 == 3);
    DestroyArrayList(list);
    printTestResult("testRemoveFromArrayList", passed);
}

static void testRemoveFromArrayListOutOfBounds(void) {
    ArrayList *list = NewArrayList(2, free);
    int a = 1, b = 2, c = 3;
    int *pa, *pb, *pc;
    int *p0, *p1, passed;
    pa = (int *)malloc(sizeof(int));
    pb = (int *)malloc(sizeof(int));
    pc = (int *)malloc(sizeof(int));
    *pa = a;
    *pb = b;
    *pc = c;
    AddToArrayList(list, pa);
    AddToArrayList(list, pb);
    AddToArrayList(list, pc);
    RemoveFromArrayList(list, 3); /* out of bounds */
    p0 = (int *)GetFromArrayList(list, 0);
    p1 = (int *)GetFromArrayList(list, 1);
    passed = (list->size == 3 && p0 && p1 && *p0 == 1 && *p1 == 2);
    DestroyArrayList(list);
    printTestResult("testRemoveFromArrayListOutOfBounds", passed);
}

static void testRemoveFromArrayListNegativeIndex(void) {
    ArrayList *list = NewArrayList(2, free);
    int a = 1, b = 2, c = 3;
    int *pa, *pb, *pc;
    int *p0, *p1, passed;
    pa = (int *)malloc(sizeof(int));
    pb = (int *)malloc(sizeof(int));
    pc = (int *)malloc(sizeof(int));
    *pa = a;
    *pb = b;
    *pc = c;
    AddToArrayList(list, pa);
    AddToArrayList(list, pb);
    AddToArrayList(list, pc);
    RemoveFromArrayList(list, -1); /* negative index */
    p0 = (int *)GetFromArrayList(list, 0);
    p1 = (int *)GetFromArrayList(list, 1);
    passed = (list->size == 3 && p0 && p1 && *p0 == 1 && *p1 == 2);
    DestroyArrayList(list);
    printTestResult("testRemoveFromArrayListNegativeIndex", passed);
}

static void testRemoveFromArrayListDuplicateValues(void) {
    ArrayList *list = NewArrayList(2, free);
    int a = 1, b = 2;
    int *pa, *pb;
    int *p0, *p1, passed;
    pa = (int *)malloc(sizeof(int));
    pb = (int *)malloc(sizeof(int));
    *pa = a;
    *pb = b;
    AddToArrayList(list, pa);
    AddToArrayList(list, pa);
    AddToArrayList(list, pb);
    RemoveFromArrayList(list, 0); /* remove first occurrence of pa */
    p0 = (int *)GetFromArrayList(list, 0);
    p1 = (int *)GetFromArrayList(list, 1);
    passed = (list->size == 2 && p0 && p1 && *p0 == a && *p1 == b);
    DestroyArrayList(list);
    printTestResult("testRemoveFromArrayListDuplicateValues", passed);
}

static void testClearArrayList(void) {
    ArrayList *list = NewArrayList(2, free);
    int a = 1, b = 2, passed = 0;
    int *pa, *pb;
    pa = (int *)malloc(sizeof(int));
    pb = (int *)malloc(sizeof(int));
    *pa = a;
    *pb = b;
    AddToArrayList(list, pa);
    AddToArrayList(list, pb);
    ClearArrayList(list);
    passed = (list->size == 0);
    DestroyArrayList(list);
    printTestResult("testClearArrayList", passed);
}

static void testIsArrayListEmptyAndArrayListSize(void) {
    ArrayList *list = NewArrayList(2, free);
    int passed = IsArrayListEmpty(list) && ArrayListSize(list) == 0;
    int a = 5;
    int *pa = (int *)malloc(sizeof(int));
    *pa = a;
    AddToArrayList(list, pa);
    passed = passed && !IsArrayListEmpty(list) && ArrayListSize(list) == 1;
    DestroyArrayList(list);
    printTestResult("testIsArrayListEmptyAndArrayListSize", passed);
}

static void testArrayListWithDestructor(void) {
    ArrayList *list;
    int passed;
    int *a, *b;
    list = NewArrayList(2, free);
    a = malloc(sizeof(int));
    b = malloc(sizeof(int));
    AddToArrayList(list, a);
    AddToArrayList(list, b);
    ClearArrayList(list);
    passed = (list->size == 0);
    DestroyArrayList(list);
    printTestResult("testArrayListWithDestructor", passed);
}

static void testArrayListContains(void) {
    ArrayList *list = NewArrayList(2, free);
    int a = 1, b = 2, c = 3;
    int *pa, *pb, *pc;
    pa = (int *)malloc(sizeof(int));
    pb = (int *)malloc(sizeof(int));
    pc = (int *)malloc(sizeof(int));
    *pa = a;
    *pb = b;
    *pc = c;
    AddToArrayList(list, pa);
    AddToArrayList(list, pb);
    AddToArrayList(list, pc);
    printTestResult("testArrayListContains", 
        ArrayListContains(list, pa, NULL) && 
        ArrayListContains(list, pb, NULL));
    printTestResult("testArrayListDoesNotContainNull",
        !ArrayListContains(list, NULL, NULL));
    AddToArrayList(list, NULL);
    printTestResult("testArrayListContainsNull",
        ArrayListContains(list, NULL, NULL));
    DestroyArrayList(list);
}

static void testArrayListContainsWithComparator(void) {
    ArrayList *list = NewArrayList(2, free);
    int a = 1, b = 2;
    int *pa, *pb;
    pa = (int *)malloc(sizeof(int));
    pb = (int *)malloc(sizeof(int));
    *pa = a;
    *pb = b;
    AddToArrayList(list, pa);
    AddToArrayList(list, pb);
    printTestResult("testArrayListContainsWithComparator", 
        ArrayListContains(list, &b, IntListItemComparator));
    DestroyArrayList(list);
}

static void testSortArrayList(void) {
    ArrayList *list = NewArrayList(5, free);
    int a = 5, b = 3, c = 8, d = 1, e = 4;
    int *pa, *pb, *pc, *pd, *pe;
    pa = (int *)malloc(sizeof(int));
    pb = (int *)malloc(sizeof(int));
    pc = (int *)malloc(sizeof(int));
    pd = (int *)malloc(sizeof(int));
    pe = (int *)malloc(sizeof(int));
    *pa = a; *pb = b; *pc = c; *pd = d; *pe = e;
    AddToArrayList(list, pa);
    AddToArrayList(list, pb);
    AddToArrayList(list, pc);
    AddToArrayList(list, pd);
    AddToArrayList(list, pe);
    
    SortArrayList(list, IntListItemComparator);
    
    printTestResult("testSortArrayList", 
        IntListItemComparator(list->items[0], &d) == 0 &&
        IntListItemComparator(list->items[1], &b) == 0 &&
        IntListItemComparator(list->items[2], &e) == 0 &&
        IntListItemComparator(list->items[3], &a) == 0 &&
        IntListItemComparator(list->items[4], &c) == 0);
    
    DestroyArrayList(list);
}

void testBubbleSort(void)
{
    int a = 5, b = 3, c = 8, d = 1, e = 4;
    int *list[5];
    list[0] = &a;
    list[1] = &b;
    list[2] = &c;
    list[3] = &d;
    list[4] = &e;
    // Before sorting: 5, 3, 8, 1, 4
    BubbleSort((void**)list, 5, IntListItemComparator);
    printTestResult("testBubbleSort", 
        IntListItemComparator(list[0], &d) == 0 &&
        IntListItemComparator(list[1], &b) == 0 &&
        IntListItemComparator(list[2], &e) == 0 &&
        IntListItemComparator(list[3], &a) == 0 &&
        IntListItemComparator(list[4], &c) == 0);
}

void testQuickSort(void)
{
    int a = 5, b = 3, c = 8, d = 1, e = 4;
    int *list[5];
    list[0] = &a;
    list[1] = &b;
    list[2] = &c;
    list[3] = &d;
    list[4] = &e;
    // Before sorting: 5, 3, 8, 1, 4
    QuickSort((void**)list, 0, 4, IntListItemComparator);
    printTestResult("testQuickSort", 
        IntListItemComparator(list[0], &d) == 0 &&
        IntListItemComparator(list[1], &b) == 0 &&
        IntListItemComparator(list[2], &e) == 0 &&
        IntListItemComparator(list[3], &a) == 0 &&
        IntListItemComparator(list[4], &c) == 0);
}

void runAllListTests(void) {
    printf("Running List Tests...\n");
    testNewArrayListAndDestroyArrayList();
    testAddToArrayListAndGetFromArrayList();
    testArrayListContains();
    testArrayListContainsWithComparator();
    testRemoveFromArrayList();
    testRemoveFromArrayListOutOfBounds();
    testRemoveFromArrayListNegativeIndex();
    testRemoveFromArrayListDuplicateValues();
    testClearArrayList();
    testIsArrayListEmptyAndArrayListSize();
    testArrayListWithDestructor();
    testBubbleSort();
    testQuickSort();
    testSortArrayList();
    printf("\n");
}
