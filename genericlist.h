#ifndef _H_GENERIC_LIST
#define _H_GENERIC_LIST

typedef struct {
  size_t size;
  void *items;
} list_t;

/**
 * Creates a new empty list.
 *
 * @return A pointer to a new list.
 */
list_t *list_new();

/**
 * Creates a new list with fixed capacity.
 *
 * @param count: Number of items the list should hold.
 * @param element_size: Size of each element.
 *
 * @return Pointer to the allocated list.
 */
list_t *list_new_fixed(int count, size_t element_size);

/**
 * Frees the list and it's content by iterating over it and deallocating 
 * each element.
 *
 * @param list: List to deallocate.
 * @param element_size: Size of a single element.
 * @param deallocator: A deallocator function to call on each element.
 */
void list_free(list_t *list, size_t element_size, void(*deallocator)(void *));

#define NEW_LIST(count, type) list_new_fixed(count, sizeof(type));
#define TO_GENERIC_LIST(list) (list_t*)list;
#define TO_SPECIFIC_LIST(list, type) (type*)list;

#endif
