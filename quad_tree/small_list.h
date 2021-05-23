#ifndef SMALL_LIST_H
#define SMALL_LIST_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define BUFFER_BYTES 256

typedef struct SmallList {
  void* data;
  char buffer[BUFFER_BYTES];
  unsigned int num_elements;
  unsigned int element_bytes;
  unsigned int capacity;
} SmallList;

void  SmallList_Init(SmallList* sl, const unsigned int element_bytes);
void  SmallList_PushBack(SmallList* sl, const void* element);
void* SmallList_PopBackRef(SmallList* sl);
void  SmallList_PopBackCopy(SmallList* sl, void* element_out);

void* SmallList_GetAtIndexRef(const SmallList* sl, const unsigned int i);
void  SmallList_GetAtIndexCopy(const SmallList* sl, const unsigned int i, void* element_out);
void  SmallList_SetAtIndex(SmallList* sl, const void* element, const unsigned int i);

void  SmallList_Resize(SmallList* sl, const unsigned int new_cap);
void  SmallList_Clear(SmallList* sl);
void  SmallList_Free(SmallList* sl);

void  SmallList_PrintInfo(const SmallList* sl);
void  SmallList_PrintData(const SmallList* sl, void(*PrintElement)(const void*));

#endif
