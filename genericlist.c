#include <stdlib.h>
#include <stdio.h>

#include "genericlist.h"

list_t *list_new() {
  return calloc(1, sizeof(list_t));
}

list_t *list_new_fixed(int count, size_t element_size) {
  list_t *list = calloc(1, sizeof(list_t));
  list->size = count;
  list->items = calloc(count, element_size);
}

void list_free(list_t *list, size_t element_size, void(*deallocator)(void *)) {
  if (deallocator != NULL) {
    for (int idx = 0; idx < list->size; idx++) {
      deallocator(list->items + idx * element_size);
    }
  }
  free(list);
}

