#include <stdlib.h>

#include "linkedlist.h"

linked_list_t *linked_list_new() {
  linked_list_t *list = calloc(1, sizeof(linked_list_t));
  return list;
}

void linked_list_free(linked_list_t *list) {
  linked_list_node_t *node = list->head, *next = NULL;
  
  while(node != NULL) {
    next = node->next;
    if (node->value != NULL) {
      if (node->deinit != NULL) {
        node->deinit(node->value);
      } else {
        free(node->value);
      }
      free(node);
    }
    node = next;
  }
}

int linked_list_size(linked_list_t *list) {
  int size = 0;
  linked_list_node_t *node = list->head;

  while (node != NULL) {
    size += 1;
    node = node->next;
  }

  return size;
}

linked_list_node_t *linked_list_tail(linked_list_t *list) {
  linked_list_node_t *node = list->head;

  if (node == NULL) {
    return NULL; 
  } else {
    while (node->next != NULL) {
      node = node->next;
    }
  }

  return node;
}

int linked_list_append(linked_list_t *list, void *value, void(*deinit)(void *)) {
  linked_list_node_t *tail = linked_list_tail(list);

  linked_list_node_t *node = malloc(sizeof(linked_list_node_t *));
  node->value = value;
  node->deinit = deinit;
  node->next = NULL;

  if (tail != NULL) {
    tail->next = node;
  } else {
    list->head = node;
  }
}
