#include "free_list.h"
#include "logging.h"

// When traversing through free list the element should be interpreted as an int
static inline int FreeList_GetElementAsInt(const FreeList* fl, const unsigned int i) {
  int* free_index = (int*)SmallList_GetAtIndexRef(&fl->sl, i);
  return *free_index;
}

void FreeList_Init(FreeList* fl, const unsigned int element_bytes) {
  assert(fl);
  assert(0 < element_bytes);

  SmallList_Init(&fl->sl, element_bytes);
  fl->first_free = -1;
}

int FreeList_Insert(FreeList* fl, const void* element) {
  assert(fl);
  assert(element);

  if(fl->first_free != -1) {
    const int index = fl->first_free;
    fl->first_free = FreeList_GetElementAsInt(fl, index);
    SmallList_SetAtIndex(&fl->sl, element, index);
    return index;
  }
  else {
    SmallList_PushBack(&fl->sl, element);
    return fl->sl.num_elements - 1;
  }
}

void* FreeList_GetAtIndexRef(const FreeList* fl, const unsigned int i) {
  assert(fl);
  assert(!FreeList_IndexIsFree(fl, i));
  
  return SmallList_GetAtIndexRef(&fl->sl, i);
}

void FreeList_GetAtIndexCopy(const FreeList* fl, const unsigned int i, void* element_out) {
  assert(fl);
  assert(element_out);

  void* element_ref = SmallList_GetAtIndexRef(&fl->sl, i);
  memcpy(element_out, element_ref, fl->sl.element_bytes);
}

// TODO: If index is already freed this could cause an infinite loop
//       Only way to check is to go through entire free list (linear time, skips throughout array)
void FreeList_EraseAtIndex(FreeList* fl, const unsigned int i) {
  assert(fl);
  assert(i < fl->sl.num_elements);
  assert(!FreeList_IndexIsFree(fl, i));

  // grab element and cast to int as it will be interpreted as free index
  int* element_as_int = (int*)SmallList_GetAtIndexRef(&fl->sl, i);
  *element_as_int = fl->first_free;
  fl->first_free = i;
}

void FreeList_Clear(FreeList* fl) {
  assert(fl);

  SmallList_Clear(&fl->sl);
  fl->first_free = -1;
}

void FreeList_Free(FreeList* fl) {
  assert(fl);

  SmallList_Free(&fl->sl);
  fl->first_free = -1;
}

int FreeList_GetNumFreeIndices(const FreeList* fl) {
  assert(fl);

  int num_free = 0;
  int index = fl->first_free;
  while(index != -1) {
    num_free++;
    index = FreeList_GetElementAsInt(fl, index);
  }

  return num_free;
}

int FreeList_GetNumElements(const FreeList* fl) {
  assert(fl);

  int num_free_indices = FreeList_GetNumFreeIndices(fl);
  return fl->sl.num_elements - num_free_indices;
}

// CALLER MUST FREE THIS DATA
int* FreeList_GetFreeIndices(const FreeList* fl, int* num_free_indices_out) {
  assert(fl);
  assert(num_free_indices_out);

  int num_free_indices = FreeList_GetNumFreeIndices(fl);

  *num_free_indices_out = num_free_indices;
  if(num_free_indices == 0) {
    return NULL;
  }
  else {
    int* frees = malloc(sizeof(int) * num_free_indices);
    if(!frees) {
      LOG("Couldn't malloc for free indices\n");
      *num_free_indices_out = -1;
      return NULL;
    }
    LOG("%s(): Malloced %ld bytes at address %p...\n", __func__, sizeof(int) * num_free_indices,
                                                         frees);

    int index = fl->first_free;
    int i = 0;
    while(index != -1) {
      frees[i] = index;
      index = FreeList_GetElementAsInt(fl, index);
      ++i;
    }
    return frees;
  }
}

bool FreeList_IndexIsFree(const FreeList* fl, const unsigned int i) {
  assert(fl);
  
  int index = fl->first_free;
  while(index != -1) {
    if(index == i) {
      return true;
    }
    index = FreeList_GetElementAsInt(fl, index);
  }

  return false;
}

void FreeList_PrintInfo(const FreeList* fl) {
  assert(fl);

  printf("\n*** FREE LIST INFO ***\n");
  printf("first_free: %d\n", fl->first_free);
  printf("num_free: %d\n", FreeList_GetNumFreeIndices(fl));
  printf("data address: %p\n", fl->sl.data);
  printf("num_elements: %d\n", FreeList_GetNumElements(fl));
  printf("element_bytes: %d\n", fl->sl.element_bytes);
  printf("capacity: %d\n", fl->sl.capacity);
}

void FreeList_PrintData(const FreeList* fl, void (*PrintElement)(const void*)) {
  assert(fl);
  assert(PrintElement);

  printf("FreeList data: ");

  // SmallList.num_elements doesn't change
  // In FreeList it acts as the end pointer
  // Thats why we use it in the loop below
  int num_elements = FreeList_GetNumElements(fl);
  if(num_elements == 0) {
    printf("[]\n");
  }
  else {
    printf("[");
    int i;
    for(i = 0; i < fl->sl.num_elements - 1; ++i) {
      if(FreeList_IndexIsFree(fl, i)) {
        printf("free,");
      }
      else {
        PrintElement(SmallList_GetAtIndexRef(&fl->sl, i));
        printf(",");
      }
    }
    // Print last element
    if(FreeList_IndexIsFree(fl, i)) {
      printf("free");
    }
    else {
      PrintElement(SmallList_GetAtIndexRef(&fl->sl, i));
    }
    printf("]\n");
  }
}

void FreeList_PrintFreeIndices(const FreeList* fl) {
  int first_free = fl->first_free;
  if(first_free == -1) {
    printf("Free indices: []\n");
  }
  else {
    int num_free;
    int* frees = FreeList_GetFreeIndices(fl, &num_free);
    if(frees) {
      printf("Free indices: [");
      for(int i = 0; i < num_free - 1; ++i) {
        printf("%d,", frees[i]);
      }
      printf("%d]\n", frees[num_free - 1]);

      LOG("%s(): freeing at address %p...\n", __func__, frees);
      free(frees);
    }
  }
}
