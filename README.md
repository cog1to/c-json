# C-JSON

A primitive JSON parser capable of mapping JSON fields into struct fields.
Current version supports following mappings:

* `int` -> `int`
* `float` -> `double`
* `bool` -> `int`
* `string` -> `char*`
* `array` -> `list_t`
* `object` -> `struct`

## Example

The parser requires a special descriptor that shows how to map JSON data
to target's values/fields. An example usage:

```c
typedef struct {
  int value1;
  int value2;
} myobj;

// Custom allocator/deallocator functions are required in case of parsing
// lists of objects, so the parser would know how to properly get and free
// memory for them.
void *myobj_alloc() { return malloc(sizeof(myobj)); }
void myobj_dealloc(void *obj) { free(obj); }

// You can use list_t from genericlist.h and cast it to your list type, or
// define your custom list type and pass it to the parser/use inside your
// struct.
typedef struct {
  int size;
  myobj *items;
} mylist;

int main(int argc , char **argv) {
  // Declare a JSON descriptor.
  json_descriptor_t desc =
  JSON_ARRAY
    JSON_OBJECT(myobj_alloc, myobj_dealloc, sizeof(myobj), 2)
      JSON_PROPERTY(value1, JSON_INT, offsetof(myobj, value1)),
      JSON_PROPERTY(value2, JSON_INT, offsetof(myobj, value2))
    JSON_OBJECT_END
  JSON_ARRAY_END;

  // Call json_parse, passing the descriptor and a pointer to the target.
  mylist target = { 0 };
  json_parse(
    "[{\"value1\": 100, \"value2\": 200}, {\"value1\": 300, \"value2\": 400}]",
    &target,
    desc
  );

  // Check the result.
  for (int i = 0; i < target.size; i++) {
    printf("value1: %d, value2: %d\n",
      target.items[i].value1,
      target.items[i].value2
    );
  }

  return 0;
}
```

Currently the parser can handle partial specs (where not all of the fields are
defined), but doesn't yet support nullable fields. So if you use those, you'll
need to provide some default values for them prior to passing the struct to the
parser.

## Descriptors

Parser uses descriptors to understand what types or structs should the input
be parsed into.

There are a few macros defined in `json.h` to help construct the JSON
descriptors. Otherwise you can look into descriptor types defined there and
do it manually.

Below are some examples of making descriptors and parsing JSON data.

### Primitive types

Primitive types are defined by a single `JSON_XXX` macro, like this:

```c
json_descriptor_t desc = JSON_INT;
int target = 0;
json_parse("666", &target, desc);
```

### Lists

For lists, you can either use a generic list structure defined in
`genericlist.h`, or define your custom strongly-typed list type, following this
pattern:

```c
typedef struct {
  size_t size; // Must be first.
  int *items;
} list_of_ints;

...

json_descriptor_t desc =
  JSON_ARRAY
    JSON_INT
  JSON_ARRAY_END;

list_of_ints target = { 0 };
json_parse("[1, 2, 3, 4]", &target, desc);
```

Lists/arrays can contain primitive types, objects or other lists.

### Objects

Objects are parsed into structs, and descriptors for them are defined with
`JSON_OBJECT` and `JSON_OBJECT_END`. Objects can have nested arrays and other
objects inside them. The first macro accepts allocator, deallocator, and a
number of properties that object/struct has inside.

To define a property, use
`JSON_PROPERTY(name, JSON_TYPE, offset_to_prop_in_struct)` macro.

```c
json_descriptor_t desc =
  JSON_OBJECT(myobj_alloc, myobj_dealloc, sizeof(myobj), 2)
    JSON_PROPERTY(foo, JSON_INT, offsetof(myobj, foo)),
    JSON_PROPERTY(bar, JSON_FLOAT, offsetof(myobj, bar))
  JSON_OBJECT_END;

myobj target = { 0 };
json_parse("{\"foo\": 1, \"bar\": -5.43}", &target, desc);
```

# TODO

* Nullable types.
* Review and refactor parsing state-machine code.

# Origin story

I was just curious what would it take to write a JSON parser in C relatively
"from scratch".

I used some C JSON parsers before and didn't quite like them. I wanted
something that I can understand and maintain myself, and that will have only
the features that I need.

It's a hobby project, and an educational project first and foremost. It most
likely could be done better and programmed better, and there are other C
libraries doing the same thing, better than this one.

