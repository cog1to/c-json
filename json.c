#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "json.h"

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
  END = 2
};

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
      } else if (symbol >= '0' || symbol <= '9' || symbol == '-') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
      } else {
        error = BAD_FORMAT;
      }
      break;
    case MIDDLE:
      if (symbol >= '0' || symbol <= '9') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
      } else if (symbol == ',' || symbol == '}' || symbol == '\n' || symbol == '\t' || symbol == ' ') {
        state = END;
      } else {
        error = BAD_FORMAT;
      }
      break;
    }
    index += 1;
  }

  if (error == 0) {
    buffer[buffer_offset+1] = '\0';
    int *t_int = target;
    *t_int = atoi(buffer);

    // Advance the offset to the last unparsed symbol.
    *offset += buffer_offset;
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
      } else if (symbol >= '0' || symbol <= '9' || symbol == '-') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
        state = EXPONENT;
      } else {
        error = BAD_FORMAT;
      }
      break;
    case EXPONENT:
      if (symbol >= '0' || symbol <= '9') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
      } else if (symbol == '.') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
        state = FRACTION;
      } else if (symbol == ',' || symbol == '}' || symbol == '\n' || symbol == '\t' || symbol == ' ') {
        state = END;
      } else {
        error = BAD_FORMAT;
      }
      break;
    case FRACTION:
      if (symbol >= '0' || symbol <= '9') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
      } else if (symbol == ',' || symbol == '}' || symbol == '\n' || symbol == '\t' || symbol == ' ') {
        state = END;
      } else {
        error = BAD_FORMAT;
      }
      break;
    }
    index += 1;
  }

  if (error == 0) {
    buffer[buffer_offset] = '\0';
    double *t_double = target;
    *t_double = strtod(buffer, NULL);

    // Advance the offset to the last unparsed symbol.
    *offset += buffer_offset;
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
      } else if (symbol == '"') {
        state = INSTRING;
      } else {
        error = BAD_FORMAT;
      }
      break;
    case INSTRING:
      if (symbol == '\\') {
        state = ESCAPE;
      } else if (symbol != '"') {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
      } else {
        state = END;
      }
      break;
    case ESCAPE:
      buffer[buffer_offset] = symbol;
      buffer_offset += 1;
      state = INSTRING;
      break;
    }
    index += 1;
  }

  if (state != END) {
    error = BAD_FORMAT;
  }

  if (error == 0) {
    buffer[buffer_offset] = '\0';
    char **string_t = target;
    *string_t = buffer;

    // Advance the offset to the last unparsed symbol.
    *offset += buffer_offset;
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
      } else if ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z')) {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
        state = INBOOL;
      } else {
        error = BAD_FORMAT;
      }
      break;
    case INBOOL:
      if ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z')) {
        buffer[buffer_offset] = symbol;
        buffer_offset += 1;
      } else if (symbol == ',' || symbol == '}' || symbol == '\n' || symbol == '\t' || symbol == ' ') {
        state = END;
      } else {
        error = BAD_FORMAT;
      }
      break;
    }
    index += 1;
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
    *offset += buffer_offset;
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
  default:
    return NOT_SUPPORTED;
  }
  return 0;
}

int json_parse_array(const char *input, int *offset, void *target, json_array_descriptor_t desc) {
  int error = NOT_SUPPORTED;
  return error;
}

/* API */

int json_parse(const char *input, void *target, json_descriptor_t descriptor) {
  int error = 0, offset = 0;

  error = json_parse_value(input, &offset, target, descriptor);
  return error;
}

/* Test */

int main(int argc, char **argv) {
  const char *input = "  false ";
  int result = -1;

  json_descriptor_t desc = {
    .type = BOOL
  };

  int error = json_parse(input, &result, desc);

  if (error != 0) {
    printf("Parsing error: %d\n", error);
  } else {
    printf("Parsing result: %d\n", result);
  }

  return 0;
}

