#ifndef _H_JSON
#define _H_JSON

#include <stdlib.h>

/* Types */

enum json_type_t {
  INT = 0,
  FLOAT = 1,
  STRING = 3,
  BOOL = 4,
  ARRAY = 5,
  OBJECT = 6
};

typedef void *(*accessor_t)(void *);
typedef void *(*allocator_t)();

typedef struct {
  int type;
  void *descriptor;
} json_descriptor_t;

typedef struct {
  char *name;
  json_descriptor_t *descriptor;
  accessor_t accessor;
} json_property_descriptor_t;

typedef struct {
  allocator_t allocator;
  size_t size;
  int num_props;
  json_property_descriptor_t *props;
} json_object_descriptor_t;

/* API */

int json_parse(const char *input, void *target, json_descriptor_t descriptor);

#endif

