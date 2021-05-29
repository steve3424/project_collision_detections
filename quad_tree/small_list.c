#include "logging.h"
#include "small_list.h"

static inline void* SmallList_GetStartAddress(const SmallList* sl) {
  return sl->data == NULL ? (void*)&sl->buffer : sl->data;
}

void SmallList_Init(SmallList* sl, unsigned int element_bytes) {
  assert(sl);
  assert(0 < element_bytes);

  sl->data = NULL;
  sl->num_elements = 0;
  sl->element_bytes = element_bytes;
  sl->capacity = BUFFER_BYTES / element_bytes;
}

void SmallList_PushBack(SmallList* sl, const void* element) {
  assert(sl);
  assert(element);

  if(sl->num_elements == sl->capacity) {
    SmallList_Resize(sl, (sl->capacity * 2));
  }

  void* begin = SmallList_GetStartAddress(sl);
  char* dst = (char*)begin + (sl->num_elements * sl->element_bytes);
  memcpy((void*)dst, element, sl->element_bytes);
  sl->num_elements++;
}

void SmallList_PopBackCopy(SmallList* sl, void* element_out) {
  assert(sl);
  assert(element_out);
  assert(0 < sl->num_elements);
  
  sl->num_elements--;
  void* begin = SmallList_GetStartAddress(sl);
  char* source = (char*)begin + (sl->num_elements * sl->element_bytes);
  memcpy(element_out, (void*)source, sl->element_bytes);
}

void* SmallList_GetAtIndexRef(const SmallList* sl, const unsigned int i) {
  assert(sl);
  assert(i < sl->num_elements);

  void* begin = SmallList_GetStartAddress(sl);
  char* element = (char*)begin + (i * sl->element_bytes);
  return (void*)element;
}

void SmallList_GetAtIndexCopy(const SmallList* sl, const unsigned int i, void* element_out) {
  assert(sl);
  assert(element_out);
  assert(i < sl->num_elements);

  void* begin = SmallList_GetStartAddress(sl);
  char* element = (char*)begin + (i * sl->element_bytes);
  memcpy(element_out, (void*)element, sl->element_bytes);
}

void SmallList_SetAtIndex(SmallList* sl, const void* element, const unsigned int i) {
  assert(sl);
  assert(element);
  assert(i < sl->num_elements);
  
  void* begin = SmallList_GetStartAddress(sl);
  char* dest = (char*)begin + (i * sl->element_bytes);
  memcpy((void*)dest, element, sl->element_bytes);
}

void SmallList_Resize(SmallList* sl, const unsigned int new_cap) {
  assert(sl);

  if(new_cap > sl->capacity) {
    void* new_data = calloc(new_cap, sl->element_bytes);
    if(!new_data) {
      LOG("%s(): Couldn't calloc on resize\n", __func__);
      return;
    }
    void* begin = SmallList_GetStartAddress(sl);
    memcpy(new_data, begin, (sl->num_elements * sl->element_bytes));
    if(sl->data) {
      free(sl->data);
      //LOG("%s(): %p address freed...\n", __func__, sl->data);
    }
    //LOG("%s(): %p address calloced %ld bytes...\n", 
    //        __func__,
    //        new_data,
    //        (long int)new_cap * (long int)sl->element_bytes); 
    sl->data = new_data;
    sl->capacity = new_cap;
  }
}

void SmallList_Clear(SmallList* sl) {
  assert(sl);

  sl->num_elements = 0;
}

void SmallList_Free(SmallList* sl) {
  assert(sl);

  if(sl->data) {
    free(sl->data);
    //LOG("%s(): %p address freed...\n", __func__, sl->data);
  }
  sl->data = NULL;
  sl->num_elements = 0;
  sl->capacity = BUFFER_BYTES / sl->element_bytes;
}

void SmallList_PrintInfo(const SmallList* sl) {
  printf("\n*** SMALL LIST INFO ***\n");
  printf("data: %p\n", SmallList_GetStartAddress(sl));
  printf("num_elements: %d\n", sl->num_elements);
  printf("element_bytes: %d\n", sl->element_bytes);
  printf("capacity: %d\n", sl->capacity);
}

void SmallList_PrintData(const SmallList* sl, void(*PrintElement)(const void*)) {
  if(sl->num_elements == 0) {
    printf("[]\n");
  }
  else {
    printf("[");
    void* begin = SmallList_GetStartAddress(sl);
    int i;
    char* element;
    for(i = 0; i < (sl->num_elements - 1); ++i) {
      element = (char*)begin + (sl->element_bytes * i);
      PrintElement((void*)element);
      printf(",");
    }
    element = (char*)begin + (sl->element_bytes * i);
    PrintElement((void*)element);
    printf("]\n");
  }
}
