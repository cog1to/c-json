#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "json.h"
#include "linkedlist.h"
#include "genericlist.h"

enum json_parse_error {
  NOT_SUPPORTED = 1,
  OUT_OF_BOUNDS = 2,
  BAD_FORMAT = 3
};

/* Int */

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
    free(list);
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
  size_t size;
  double *items;
} double_list_t;

typedef struct {
  size_t size;
  double_list_t *items;
} double_list_list_t;

int main(int argc, char **argv) {
  const char *input = "[[ .54, 1.23, -6.5], [2, 3, 4.0]]";
  double_list_list_t output; 

  json_descriptor_t desc = {
    .type = ARRAY,
    .descriptor = &(json_descriptor_t) {
      .type = ARRAY,
      .descriptor = &(json_descriptor_t) {
        .type = FLOAT
      }
    }
  };

  int error = json_parse(input, &output, desc);

  if (error != 0) {
    printf("Parsing error: %d\n", error);
  } else {
    printf("[");
    for (int idx = 0; idx < output.size; idx++) {
      double_list_t lst = output.items[idx];
      printf("[");
      for (int lidx = 0; lidx < lst.size; lidx++) {
        printf("%f,", lst.items[lidx]);
      }
      printf("]");
    }
    printf("]\n");
  }

  return 0;
}

