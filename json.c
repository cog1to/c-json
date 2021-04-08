#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "json.h"
#include "linkedlist.h"
#include "genericlist.h"

enum json_parse_error {
  NOT_SUPPORTED = 1,
  OUT_OF_BOUNDS = 2,
  BAD_FORMAT = 3,
  BAD_SPEC = 4,
  PROP_NOT_FOUND = 5
};

/* Internal state. */

enum json_state {
  INIT = 0,
  MIDDLE = 1,
  EXPONENT = 3,
  FRACTION = 4,
  INSTRING = 5,
  INBOOL = 7,
  ESCAPE = 6,
  ARRAY_NEXT = 8,
  ARRAY_VALUE = 9,
  OBJECT_PROP_DELIM = 10,
  OBJECT_PROP_VALUE = 11,
  OBJECT_NEXT = 13,
  OBJECT_PROP_NAME = 14,
  OBJECT_PROP_NEXT = 15,
  END = 2
};

/* Internal API */

int json_parse_int(const char *input, int *offset, void *target);
int json_parse_float(const char *input, int *offset, void *target);
int json_parse_string(const char *input, int *offset, void *target);
int json_parse_bool(const char *input, int *offset, void *target);
int json_parse_value(const char *input, int *offset, void *target, json_descriptor_t descriptor);
int json_element_size(json_descriptor_t desc);
void *json_array_element_alloc(json_descriptor_t desc);
int json_parse_array(const char *input, int *offset, void *target, json_descriptor_t desc);
int json_parse_object(const char *input, int *offset, void *target, json_descriptor_t desc);

/* Implementation */

int json_parse_int(const char *input, int *offset, void *target) {
  int length = strlen(input) - (*offset);
  if (length <= 0) {
    return OUT_OF_BOUNDS;
  }

  char *buffer = calloc(strlen(input), sizeof(char));
  int state = INIT, error = 0;

  int index = *offset, buffer_offset = 0;
  while (index < strlen(input) && error == 0 && state != END) {
    char symbol = input[index];

    switch (state) {
    case INIT:
      if (symbol == '\n' || symbol == '\t' || symbol == ' ') {
        // Skip whitespace symbols.
        index += 1;
      } else if ((symbol >= '0' && symbol <= '9') || symbol == '-') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
        state = MIDDLE;
        index += 1;
      } else {
        error = BAD_FORMAT;
      }
      break;
    case MIDDLE:
      if (symbol >= '0' && symbol <= '9') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
        index += 1;
      } else if (symbol == ',' || symbol == '}' || symbol == '\n' || symbol == '\t' || symbol == ' ' || symbol == ']') {
        state = END;
      } else {
        error = BAD_FORMAT;
      }
      break;
    }
  }

  if (error == 0) {
    buffer[buffer_offset+1] = '\0';
    int *t_int = target;
    *t_int = atoi(buffer);

    // Advance the offset to the last unparsed symbol.
    *offset = index;
  }

  free(buffer);
  return error;
}

int json_parse_float(const char *input, int *offset, void *target) {
  int length = strlen(input) - (*offset);
  if (length <= 0) {
    return OUT_OF_BOUNDS;
  }

  char *buffer = calloc(strlen(input), sizeof(char));
  int state = INIT, error = 0;

  int index = *offset, buffer_offset = 0;
  while (index < strlen(input) && error == 0 && state != END) {
    char symbol = input[index];
    switch (state) {
    case INIT:
      if (symbol == '\n' || symbol == '\t' || symbol == ' ') {
        // Skip whitespace symbols.
        index += 1;
      } else if ((symbol >= '0' && symbol <= '9') || symbol == '-') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
        state = EXPONENT;
        index += 1;
      } else if (symbol == '.') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
        state = FRACTION;
        index += 1;
      } else {
        error = BAD_FORMAT;
      }
      break;
    case EXPONENT:
      if (symbol >= '0' && symbol <= '9') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
        index += 1;
      } else if (symbol == '.') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
        state = FRACTION;
        index += 1;
      } else if (symbol == ',' || symbol == '}' || symbol == '\n' || symbol == '\t' || symbol == ' ' || symbol == ']') {
        state = END;
      } else {
        error = BAD_FORMAT;
      }
      break;
    case FRACTION:
      if (symbol >= '0' && symbol <= '9') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
        index += 1;
      } else if (symbol == ',' || symbol == '}' || symbol == '\n' || symbol == '\t' || symbol == ' ' || symbol == ']') {
        state = END;
      } else {
        error = BAD_FORMAT;
      }
      break;
    }
  }

  if (error == 0) {
    buffer[buffer_offset] = '\0';
    double *t_double = target;
    *t_double = strtod(buffer, NULL);

    // Advance the offset to the last unparsed symbol.
    *offset = index;
  }

  free(buffer);
  return error;
}

int json_parse_string(const char *input, int *offset, void *target) {
  int length = strlen(input) - (*offset);
  if (length <= 0) {
    return OUT_OF_BOUNDS;
  }

  char *buffer = calloc(strlen(input), sizeof(char));
  int state = INIT, error = 0;

  int index = *offset, buffer_offset = 0;
  while (index < strlen(input) && error == 0 && state != END) {
    char symbol = input[index];

    switch (state) {
    case INIT:
      if (symbol == '\n' || symbol == '\t' || symbol == ' ') {
        // Skip whitespace symbols.
        index += 1;
      } else if (symbol == '"') {
        index += 1;
        state = INSTRING;
      } else {
        error = BAD_FORMAT;
      }
      break;
    case INSTRING:
      if (symbol == '\\') {
        index += 1;
        state = ESCAPE;
      } else if (symbol != '"') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
        index += 1;
      } else {
        index += 1;
        state = END;
      }
      break;
    case ESCAPE:
      buffer[buffer_offset] = symbol;
      buffer_offset += 1;
      state = INSTRING;
      index += 1;
      break;
    }
  }

  if (state != END) {
    error = BAD_FORMAT;
  }

  if (error == 0) {
    buffer[buffer_offset] = '\0';
    char **string_t = target;
    *string_t = buffer;

    // Advance the offset to the last unparsed symbol.
    *offset = index;
  }

  return error;
}

int json_parse_bool(const char *input, int *offset, void *target) {
  int length = strlen(input) - (*offset);
  if (length <= 0) {
    return OUT_OF_BOUNDS;
  }

  char *buffer = calloc(strlen(input), sizeof(char));
  int state = INIT, error = 0;

  int index = *offset, buffer_offset = 0;
  while (index < strlen(input) && error == 0 && state != END) {
    char symbol = input[index];
    switch (state) {
    case INIT:
      if (symbol == '\n' || symbol == '\t' || symbol == ' ') {
        // Skip whitespace symbols.
        index += 1;
      } else if ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z')) {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
        state = INBOOL;
        index += 1;
      } else {
        error = BAD_FORMAT;
      }
      break;
    case INBOOL:
      if ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z')) {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
        index += 1;
      } else if (symbol == ',' || symbol == '}' || symbol == '\n' || symbol == '\t' || symbol == ' ' || symbol == ']') {
        state = END;
      } else {
        error = BAD_FORMAT;
      }
      break;
    }
  }

  if (error == 0) {
    buffer[buffer_offset] = '\0';
    int *t_bool = target;
    if (strcmp("true", buffer) == 0) {
      *t_bool = 1;
    } else if (strcmp("false", buffer) == 0) {
      *t_bool = 0;
    } else {
       error = BAD_FORMAT;
    }

    // Advance the offset to the last unparsed symbol.
    *offset = index;
  }

  free(buffer);
  return error;
}

int json_parse_value(const char *input, int *offset, void *target, json_descriptor_t descriptor) {
  int error = 0;

  switch (descriptor.type) {
  case INT:
    error = json_parse_int(input, offset, target);
    break;
  case FLOAT:
    error = json_parse_float(input, offset, target);
    break;
  case STRING:
    error = json_parse_string(input, offset, target);
    break;
  case BOOL:
    error = json_parse_bool(input, offset, target);
    break;
  case ARRAY:
    error = json_parse_array(input, offset, target, descriptor);
    break;
  case OBJECT:
    error = json_parse_object(input, offset, target, descriptor);
    break;
  default:
    return NOT_SUPPORTED;
  }
  return error;
}

int json_element_size(json_descriptor_t desc) {
  json_object_descriptor_t *obj_desc = NULL;

  switch (desc.type) {
  case BOOL:
  case INT:
    return sizeof(int);
  case FLOAT:
    return sizeof(double);
  case STRING:
    return sizeof(char *);
  case ARRAY:
    return sizeof(list_t);
  case OBJECT:
    obj_desc = desc.descriptor;
    return obj_desc->size;
  }
}

void *json_array_element_alloc(json_descriptor_t desc) {
  return calloc(1, json_element_size(desc));
}

int json_parse_array(const char *input, int *offset, void *target, json_descriptor_t desc) {
  int length = strlen(input) - (*offset);
  if (length <= 0) {
    return OUT_OF_BOUNDS;
  }

  json_descriptor_t *element_desc = desc.descriptor;
  if (element_desc == NULL) {
    return BAD_FORMAT;
  }

  int state = INIT, error = 0, index = *offset;
  linked_list_t *list = linked_list_new();
  void *elem_target = NULL;

  while (index < strlen(input) && error == 0 && state != END) {
    char symbol = input[index];

    switch (state) {
    case INIT:
      if (symbol == '\n' || symbol == '\t' || symbol == ' ') {
        // Skip whitespace symbols.
      } else if (symbol == '[') {
        state = ARRAY_VALUE;
      } else {
        error = BAD_FORMAT;
      }
      index += 1;
      break;
    case ARRAY_VALUE:
      elem_target = json_array_element_alloc(desc);
      error = json_parse_value(input, &index, elem_target, *element_desc);
      if (error == 0) {
        linked_list_append(list, elem_target, NULL);
        state = ARRAY_NEXT;
      }
      break;
    case ARRAY_NEXT:
      if (symbol == '\n' || symbol == '\t' || symbol == ' ') {
        // Skip whitespace symbols.
      } else if (symbol == ',') {
        state = ARRAY_VALUE;
      } else if (symbol == ']') {
        state = END;
      }
      index += 1;
      break;
    }
  }

  int list_size = linked_list_size(list), element_size = json_element_size(*element_desc), arr_ind = 0;
  if (error == 0 && list_size > 0) {
    void *array = calloc(list_size, element_size);

    linked_list_node_t *node = list->head, *next = node->next;
    while (node != NULL) {
      next = node->next;
      memcpy(array + arr_ind * element_size, node->value, element_size);

      free(node->value);
      free(node);

      arr_ind += 1;
      node = next;
    }

    free(list);

    list_t *target_list = target;
    target_list->size = list_size;
    target_list->items = array;

    *offset = index;
  } else if (error != 0) {
    linked_list_free(list);
  }

  return error;
}

json_property_descriptor_t *json_object_get_property(json_object_descriptor_t desc, const char *name) {
  for (int idx = 0; idx < desc.num_props; idx++) {
    json_property_descriptor_t prop = desc.props[idx];
    if (strcmp(prop.name, name) == 0) {
      return &desc.props[idx];
    }
  }
  return NULL;
}

int json_parse_object(const char *input, int *offset, void *target, json_descriptor_t desc) {
  int length = strlen(input) - (*offset);
  if (length <= 0) {
    return OUT_OF_BOUNDS;
  }

  json_object_descriptor_t *obj_desc = desc.descriptor;
  if (obj_desc == NULL) {
    return BAD_SPEC;
  }

  int state = INIT, error = 0, index = *offset;
  char *prop_name = NULL;
  json_property_descriptor_t *prop;

  printf("parsing object...\n");

  while (index < strlen(input) && error == 0 && state != END) {
    char symbol = input[index];
    printf("  symbol: '%c'\n", symbol);

    switch (state) {
    case INIT:
      if (symbol == '\n' || symbol == '\t' || symbol == ' ') {
        // Skip whitespace symbols.
      } else if (symbol == '{') {
        printf("going from INIT to OBJECT_NEXT\n");
        state = OBJECT_NEXT;
      } else {
        printf("going from INIT to BAD_FORMAT\n");
        error = BAD_FORMAT;
      }
      index += 1;
      break;
    case OBJECT_PROP_NAME:
      printf("parsing name\n");
      error = json_parse_string(input, &index, &prop_name);

      if (error == 0) {
        printf("parsed name\n");
        prop = json_object_get_property(*obj_desc, prop_name);
        if (prop == NULL) {
          printf("prop %s not found\n");
          error = PROP_NOT_FOUND;
        } else {
          printf("going to DELIM from PROP_NAME\n");
          state = OBJECT_PROP_DELIM;
        }
      }
      break;
    case OBJECT_PROP_DELIM:
      if (symbol == '\n' || symbol == '\t' || symbol == ' ') {
        // Skip whitespace symbols.
        index += 1;
      } else if (symbol == ':') {
        printf("going from OBJECT_DELIM to VALUE\n");
        state = OBJECT_PROP_VALUE;
        index += 1;
      } else {
        error = BAD_FORMAT;
      }
      break;
    case OBJECT_PROP_VALUE:
      error = json_parse_value(input, &index, prop->accessor(target), prop->descriptor);
      if (error != 0) {
        printf("failed to parse prop value\n");
      }

      free(prop_name);
      prop_name = NULL;

      printf("going to PROP_NEXT\n");
      state = OBJECT_PROP_NEXT;
      break;
    case OBJECT_PROP_NEXT:
      if (symbol == '\n' || symbol == '\t' || symbol == ' ') {
        // Skip whitespace symbols.
        index += 1;
      } else if (symbol == ',') {
        state = OBJECT_NEXT;
        index += 1;
      } else if (symbol == '}') {
        state = END;
        index += 1;
      } else {
        error = BAD_FORMAT;
      }
    case OBJECT_NEXT:
      if (symbol == '\n' || symbol == '\t' || symbol == ' ') {
        index += 1;
        // Skip whitespace symbols.
      } else if (symbol == '}') {
        index += 1;
        state = END;
      } else if (symbol == '"') {
        state = OBJECT_PROP_NAME;
      } else {
        error = BAD_FORMAT;
      }
      break;
    }
  }

  if (prop_name != NULL) {
    free(prop_name);
  }

  if (error == 0) {
    *offset = index;
  }

  return error;
}

/* API */

int json_parse(const char *input, void *target, json_descriptor_t descriptor) {
  int error = 0, offset = 0;

  error = json_parse_value(input, &offset, target, descriptor);
  return error;
}

/* Test */

typedef struct {
  int value;
} intobject;

typedef struct {
  intobject value;
} myobject;

void *myobj_value(myobject *obj) {
  return &(obj->value);
}

void *intobj_value(intobject *obj) {
  return &(obj->value);
}

#define TO_GENERIC_ACCESSOR(func) (void *(*)(void *))func

int main(int argc, char **argv) {
  const char *input = "{\"value\": {\"value\": 12}}";
  myobject output;

  json_descriptor_t desc = {
    .type = OBJECT,
    .descriptor = &(json_object_descriptor_t){
      .num_props = 1,
      .props = (json_property_descriptor_t[]){
        {
          .name = "value",
          .descriptor = {
            .type = OBJECT,
            .descriptor = &(json_object_descriptor_t){
              .num_props = 1,
              .props = (json_property_descriptor_t[]){
                {
                  .name = "value",
                  .descriptor = {
                    .type = INT
                  },
                  .accessor = TO_GENERIC_ACCESSOR(intobj_value)
                }
              }
            }
          },
          .accessor = TO_GENERIC_ACCESSOR(myobj_value)
        }
      }
    }
  };

  int error = json_parse(input, &output, desc);
  if (error != 0) {
    printf("Parsing error: %d\n", error);
  } else {
    printf("Parsed succesfully: %d\n", output.value.value);

  }

  return 0;
}

