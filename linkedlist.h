#ifndef _H_LINKED_LIST
#define _H_LINKED_LIST

typedef struct linked_list_node linked_list_node_t;

struct linked_list_node {
  void *value;
  linked_list_node_t *next;
  void(*deinit)(void *);
};

typedef struct {
  linked_list_node_t *head;
} linked_list_t;

linked_list_t *linked_list_new();

void linked_list_free(linked_list_t *list);

int linked_list_size(linked_list_t *list);

int linked_list_append(linked_list_t *list, void *value, void(*deinit)(void *));

linked_list_node_t *linked_list_tail(linked_list_t *list);

#endif
