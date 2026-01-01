#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct _ListHeader {
    size_t capacity;
    size_t length;
    size_t element_size;
} _ListHeader;

#define List_new(type, ...)                                      \
    (type*)_List_from_array(sizeof(type), (type[]){__VA_ARGS__}, \
                            sizeof((type[]){__VA_ARGS__}) / sizeof(type))

#define List_print(list, format)                 \
    {                                            \
        putchar('[');                            \
        for (size_t i = 0; i < len(list); i++) { \
            if (i) printf(", ");                 \
            printf(format, list[i]);             \
        }                                        \
        putchar(']');                            \
    }

#define List_append(list, item)                                                               \
    {                                                                                         \
        __auto_type _item = (item);                                                           \
        static_assert(__builtin_types_compatible_p(__typeof__((list)[0]), __typeof__(_item)), \
                      "List_append: item type mismatch");                                     \
        _ListHeader* head = _List_get_header(list);                                           \
        (list)[head->length++] = (_item);                                                     \
        if (head->length >= head->capacity) list = List_resize(list, head->capacity * 2);     \
    }

#define List_at(list, idx) \
    (list)[_List_convert_idx((list), (idx), __func__, __FILE_NAME__, __LINE__)]

#define foreach(var, list)     \
    __auto_type var = list[0]; \
    for (int i = 0; i < len(list); var = list[++i])

#define List_insert(list, idx, element)                                                   \
    {                                                                                     \
        _ListHeader* head = _List_get_header(list);                                       \
        head->length++;                                                                   \
        if (head->length >= head->capacity) list = List_resize(list, head->capacity * 2); \
        size_t i = _List_convert_idx((list), (idx), __func__, __FILE_NAME__, __LINE__);   \
        size_t tail = len(list) - i;                                                      \
        if (tail) {                                                                       \
            char* base = (char*)list;                                                     \
            size_t esz = head->element_size;                                              \
            memmove(base + (i + 1) * esz, base + i * esz, tail * esz);                    \
        }                                                                                 \
        (list)[i] = (element);                                                            \
    }

#define List_set(list, idx, element)                                                    \
    {                                                                                   \
        size_t i = _List_convert_idx((list), (idx), __func__, __FILE_NAME__, __LINE__); \
        (list)[i] = (element);                                                          \
    }

typedef bool (*cmp_fn_t)(void*, void*);
#define List_index(list, value) _List_index((list), &(__typeof__((list)[0])){(value)})
#define List_contains(list, value) (_List_index((list), &(__typeof__((list)[0])){(value)}) != -1)
#define List_index_fn(list, value, fn) _List_index_fn((list), (void*)(value), (cmp_fn_t)(fn))
#define List_contains_fn(list, value, fn) \
    (_List_index_fn((list), (void*)(value), (cmp_fn_t)(fn)) != -1)

#define List_extend(list1, list2)                                                      \
    {                                                                                  \
        __auto_type _l2 = (list2);                                                     \
        _ListHeader* head = _List_get_header(list1);                                   \
        size_t cur_len = head->length;                                                 \
        head->length += len(_l2);                                                      \
        if (head->length >= head->capacity) {                                          \
            size_t new_capacity =                                                      \
                head->length > 2 * head->capacity ? head->length : 2 * head->capacity; \
            list1 = List_resize(list1, new_capacity);                                  \
        }                                                                              \
        memcpy(list1 + cur_len, _l2, len(_l2) * head->element_size);                   \
    }

#define List_repeat(list, count) ((typeof(list))_List_repeat((list), (count)))

#define List_copy(list) ((typeof(list))_List_copy((list)))

#define List_sort(list, cmp_fn) _List_sort((list), (int (*)(const void*, const void*))(cmp_fn))

void* _List_new(size_t element_size, size_t length);
void* List_resize(void* list, size_t new_capacity);
_ListHeader* _List_get_header(void* list);
void* _List_from_array(size_t elem_size, const void* arr, size_t n);
size_t len(void* list);
void List_free(void* list);
void List_remove(void* list, size_t i, void* output);
void List_pop(void* list, void* output);
size_t _List_convert_idx(void* list, int idx, const char* _fn, const char* _file, int _ln);
void List_clear(void* list);
int _List_index(void* list, const void* value);
int _List_index_fn(void* list, void* value, bool (*cmp_fn)(void*, void*));
void* _List_repeat(void* list, size_t count);
void* _List_copy(void* list);
void _List_sort(void* list, int (*cmp_fn)(const void*, const void*));

// String stuff

#define WHITESPACE " \n\t\r"

typedef char* String;
String String_new(const char* _Format, ...);
void String_free(String s);
String List_string(void* list, const char* _Format);
String String_from_format(const char* _Format, void* item);
String String_concat(String s1, String s2);
String String_slice(String s, int start, int last, int step);
String String_capitalize(String s);
String String_upper(String s);
String String_lower(String s);
bool String_equals(String s1, String s2);
String String_join(String sep, String* list);
bool String_isalpha(String s);
bool String_isdigit(String s);
bool String_isalnum(String s);
bool String_startswith(String s, String prefix);
bool String_endswith(String s, String suffix);
bool String_contains(String s, char c);
String String_strip(String s, char* characters);
/*
center()
count()
expandtabs()
find()
index()
isidentifier()
islower()
isprintable()
isspace()
istitle()
isupper()
ljust()
lstrip()
partition()
replace()
rfind()
rindex()
rjust()
rpartition()
rsplit()
rstrip()
split()
splitlines()
swapcase()
title()
*/

#define String_append(s1, s2)                                                              \
    {                                                                                      \
        _ListHeader* head = _List_get_header(s1);                                          \
        size_t new_len = head->length + strlen(s2);                                        \
        if (new_len + 1 >= head->capacity) {                                               \
            size_t new_capacity = head->capacity * 2 > new_len ? head->capacity : new_len; \
            s1 = List_resize(s1, new_capacity);                                            \
        }                                                                                  \
        memcpy((s1) + head->length, (s2), strlen(s2));                                     \
    }

// Garbage collector stuff

typedef void (*free_fn_t)(void*);
void* gc_track(void* p, free_fn_t free_fn);  // Start tracking object for collection
void gc_frame(void);                         // Create new garbage collection layer
void* gc_keep(void* p);                      // Stop tracking object
void* gc_collect(void* p);                   // Collect and move object to previous collection layer
void* gc_calloc(size_t count, size_t size);  // Garbage collected calloc
void* gc_malloc(size_t size);                // Garbage collected malloc
void* gc_realloc(void* ptr, size_t size);    // Garbage collected realloc