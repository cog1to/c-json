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

typedef void *(*allocator_t)();
typedef void (*deallocator_t)(void *);

typedef struct {
  int type;
  void *descriptor;
} json_descriptor_t;

typedef struct {
  char *name;
  json_descriptor_t descriptor;
  size_t offset;
} json_property_descriptor_t;

typedef struct {
  allocator_t allocator;
  deallocator_t deallocator;
  size_t size;
  int num_props;
  json_property_descriptor_t *props;
} json_object_descriptor_t;

/* API */

int json_parse(const char *input, void *target, json_descriptor_t descriptor);

/**
 * JSON spec costructors.
 *
 * Example usage:
 * 
 * json_descriptor_t desc = 
 * JSON_ARRAY
 *   JSON_OBJECT(myobj_alloc, myobj_dealloc, sizeof(myobj), 2)
 *     JSON_PROPERTY(value1, JSON_INT, offsetof(myobj, value1)),
 *     JSON_PROPERTY(value2, JSON_INT, offsetof(myobj, value2))
 *   JSON_OBJECT_END
 * JSON_ARRAY_END;
 *
 **/

#define JSON_PROPERTY(pname, desc, off) { \
.name = #pname, \
.descriptor = desc, \
.offset = off \
}

#define JSON_INT { .type = INT }
#define JSON_FLOAT { .type = FLOAT }
#define JSON_STRING { .type = STRING }
#define JSON_BOOL { .type = BOOL }

#define JSON_ARRAY { \
.type = ARRAY, \
.descriptor = &(json_descriptor_t)

#define JSON_ARRAY_END }

#define JSON_OBJECT(alloc, dealloc, osize, num) { \
.type = OBJECT, \
.descriptor = &(json_object_descriptor_t){ \
  .allocator = alloc, \
  .deallocator = dealloc, \
  .size = osize, \
  .num_props = num, \
  .props = (json_property_descriptor_t[]){

#define JSON_OBJECT_END } \
} \
}

#endif

