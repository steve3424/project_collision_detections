#ifndef FREE_LIST_H
#define FREE_LIST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "small_list.h"

typedef struct FreeList {
  SmallList sl;
  int first_free;
} FreeList;

void FreeList_Init(FreeList* fl, const unsigned int element_bytes);

int   FreeList_Insert(FreeList* fl, const void* element);
void* FreeList_GetAtIndexRef(const FreeList* fl, const unsigned int i);
void  FreeList_GetAtIndexCopy(const FreeList* fl, const unsigned int i, void* element_out);
void  FreeList_EraseAtIndex(FreeList* fl, const unsigned int i);
void  FreeList_Clear(FreeList* fl);
void  FreeList_Free(FreeList* fl);

// Caller must free pointer that is returned
int* FreeList_GetFreeIndices(const FreeList* fl, int* num_free_indices_out);
int  FreeList_GetNumElements(const FreeList* fl);
int  FreeList_GetNumFreeIndices(const FreeList* fl);
bool FreeList_IndexIsFree(const FreeList* fl, const unsigned int i);

void FreeList_PrintInfo(const FreeList* fl);
void FreeList_PrintData(const FreeList* fl, void (*PrintElement)(const void*));
void FreeList_PrintFreeIndices(const FreeList* fl);

#endif
