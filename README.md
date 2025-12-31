# dynamic.c

This is a simple C library that provides dynamic list, string manipulation, and basic garbage collection functionalities.

Files included:

- dynamic.c
- dynamic.h

## Dynamic List

The dynamic list implementation allows you to create and manage generic lists that can grow and shrink in size as needed.

```c
#include "dynamic.h"
#include <stdio.h>

int main() {
    int* mylist = List_new(int, 1, 5, 9);
    List_append(mylist, 15);
    printf("List length: %zu\n", len(mylist));
    printf("List as string: %s\n", List_string(mylist, "%d"));
}
```

### List Functions

- `List_new(type, ...)`: Create a new list of the specified type with initial elements.
- `List_append(list, element)`: Append an element to the end of the list.
- `len(list)`: Get the current length of the list.
- `List_string(list, format)`: Convert the list to a string representation using the specified format.
- `List_free(list)`: Free the memory allocated for the list.
- `List_at(list, index)`: Access an element at the specified index.
- `List_resize(list, new_size)`: Resize the list to the new size.
- `List_pop(list)`: Remove and return the last element of the list.
- `List_remove(list, index)`: Remove the element at the specified index.

### `foreach` Macro

The `foreach` macro allows you to iterate over each element in the list easily.

```c
foreach(item, mylist) {
    printf("item %d -> %d\n", i, item);
}
```

### `List_string` Function

The `List_string` function converts a list to its string representation using a specified format for each element. The format is similar to that used in `printf` but must start with a `%` character and is applied to each element in the list.

```c
double* mylist = List_new(double, 1, 2, 3);
printf("%s\n", List_string(mylist, "%.2f")); // [1.00, 2.00, 3.00]
printf("%s\n", List_string(mylist, "hey"));  // [<cannot use 'hey' on 0xbcac04018>, ...]
```

Recursive lists are also supported by adding the prefix `[` to the format string.

```c
double** mylist = List_new(double*);
List_append(mylist, List_new(double, 1, 2, 3));
List_append(mylist, List_new(double, 7, 8, 9, 10));
List_append(mylist, List_new(double, 3));

printf("nested list -> %s\n", List_string(mylist, "[%.2lf"));
// nested list -> [[1.00, 2.00, 3.00], [7.00, 8.00, 9.00, 10.00], [3.00]]
```

## String Manipulation

The library provides functions for basic string manipulation. Strings are treated as dynamic lists of characters.

```c
#include "dynamic.h"
#include <stdio.h>

int main() {
    String s = String_new("Hello, world!");
    printf("String length: %zu\n", len(s));
    String_append(s, " How are you?");
    printf("Concatenated string: %s\n", s);
}
```

### String Functions

- `String_new(fmt, ...)`: Create a new string from a dynamic or C-style string with formatted input.
- `String_append(str, suffix)`: Append a suffix to the string.
- `String_free(str)`: Free the memory allocated for the string.
- `List_string(list, format)`: Convert a list to a string representation using the specified format.
- `String_from_format(format, ...)`: Create a new string using formatted input and a pointer to data.
- `String_concat(str1, str2)`: Concatenate two strings.
- `String_slice(str, start, end, step)`: Extract a substring from the string.
- `String_capitalize(str)`: Capitalize the first character of the string.
- `String_upper(str)`: Convert the string to uppercase.
- `String_lower(str)`: Convert the string to lowercase.
- `String_equals(str1, str2)`: Check if two strings are equal.
- `String_join(separator, list)`: Join a list of strings using the specified separator.

## Garbage Collection

The library includes basic garbage collection functionalities to manage memory of dynamic list and string objects automatically.

```c
#include "dynamic.h"
#include <stdio.h>

int main() {
    gc_frame();
    int* mylist = List_new(int, 1, 2, 3);
    gc_collect(NULL); // List is freed automatically
    return 0;
}
```

### Garbage Collector Frame

The garbage collector stores a stack of frames. Each call to `gc_frame()` pushes a new frame onto the stack. When `gc_collect(NULL)` is called, all objects created within the current frame are freed, and the frame is popped from the stack.

A frame is created by default at the start of the program, so you don't need to call `gc_frame()` unless you want to create nested frames.

```c
gc_frame(); // Push a new frame
// Dynamic objects created here ...
gc_collect(NULL); // Free all objects in the current frame and pop the frame
```

### Object Collection

The `gc_collect(obj)` function frees all objects in the current frame except for the specified object `obj`. If `obj` is `NULL`, all objects in the current frame are freed. The `obj` object is added to the next frame. `gc_collect` also pops the current frame from the stack and returns the specified object to make it easier to chain calls.

```c
int* foo() {
    gc_frame(); // Push a new frame
    int* mylist = List_new(int, 1, 2, 3);
    int* anotherlist = List_new(int, 4, 5, 6);
    return gc_collect(mylist); // Free anotherlist, send mylist to next frame
}
```

### Untracking

The `gc_keep(obj)` function removes the specified object `obj` from garbage collection tracking, meaning it will not be freed during garbage collection.

```c
int* mylist = List_new(int, 1, 2, 3);
gc_keep(mylist); // mylist will not be freed during gc_collect
// ...
gc_collect(NULL); // mylist remains allocated
// ...
List_free(mylist); // Manually free mylist when done
```

### Tracking non-dynamic objects

The `gc_track(obj)` function adds a non-dynamic object `obj` to garbage collection tracking, meaning it will be freed during garbage collection.

_Note: `obj` cannot be untracked or sent to another frame._

```c
int* myarray = malloc(10 * sizeof(int));
gc_track(myarray); // myarray will be freed during gc_collect
// ...
gc_collect(NULL); // myarray is freed
```

```c
int* myarray = malloc(10 * sizeof(int));
gc_track(myarray); // myarray will be freed during gc_collect
// ...
gc_keep(myarray); // Error: myarray cannot be untracked
gc_collect(myarray); // Error: myarray cannot be sent to another frame
```
