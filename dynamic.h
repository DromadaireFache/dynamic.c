#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
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
size_t _gc_frame_nbr(void);

static inline void _gc_cleanup(void* frame_nbr_p) {
    size_t frame_nbr = *(size_t*)frame_nbr_p;
    while (frame_nbr <= _gc_frame_nbr()) gc_collect(NULL);
}

static inline void _auto_free(const void* p) { free(*(void**)p); }

#define $(function_body) \
    {                    \
        collected;       \
        function_body    \
    }

#define var __auto_type
#define let const __auto_type
#define collected \
    gc_frame();   \
    __attribute__((cleanup(_gc_cleanup))) size_t __$__ = _gc_frame_nbr()
#define defer __attribute__((cleanup(_auto_free)))

#define new(dynamic) dynamic##_new
typedef void* List;

// Print function

static inline void _print_cstr(const char* s) { printf("%s", s); }
static inline void _print_int(int v) { printf("%d", v); }
static inline void _print_uint(unsigned v) { printf("%u", v); }
static inline void _print_long(long v) { printf("%ld", v); }
static inline void _print_ulong(unsigned long v) { printf("%lu", v); }
static inline void _print_llong(long long v) { printf("%lld", v); }
static inline void _print_ullong(unsigned long long v) { printf("%llu", v); }
static inline void _print_float(double v) { printf("%f", v); }
static inline void _print_double(double v) { printf("%lf", v); }
static inline void _print_ptr(const void* p) { printf("%p", p); }

#define _p(arg)                            \
    _Generic((arg),                        \
        char*: _print_cstr,                \
        signed char: _print_int,           \
        unsigned char: _print_uint,        \
        short: _print_int,                 \
        unsigned short: _print_uint,       \
        int: _print_int,                   \
        unsigned int: _print_uint,         \
        long: _print_long,                 \
        unsigned long: _print_ulong,       \
        long long: _print_llong,           \
        unsigned long long: _print_ullong, \
        float: _print_float,               \
        double: _print_double,             \
        void*: _print_ptr,                 \
        default: _print_ptr)((arg))

#define print(...)                                                                             \
    _print_impl(__VA_ARGS__, _print_20, _print_19, _print_18, _print_17, _print_16, _print_15, \
                _print_14, _print_13, _print_12, _print_11, _print_10, _print_9, _print_8,     \
                _print_7, _print_6, _print_5, _print_4, _print_3, _print_2, _print_1)

#define _print_impl(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, \
                    _18, _19, _20, N, ...)                                                      \
    N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20)

#define _print_1(_1, ...) (_p(_1))
#define _print_2(_1, _2, ...) (_p(_1), _p(_2))
#define _print_3(_1, _2, _3, ...) (_p(_1), _p(_2), _p(_3))
#define _print_4(_1, _2, _3, _4, ...) (_p(_1), _p(_2), _p(_3), _p(_4))
#define _print_5(_1, _2, _3, _4, _5, ...) (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5))
#define _print_6(_1, _2, _3, _4, _5, _6, ...) (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6))
#define _print_7(_1, _2, _3, _4, _5, _6, _7, ...) \
    (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6), _p(_7))
#define _print_8(_1, _2, _3, _4, _5, _6, _7, _8, ...) \
    (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6), _p(_7), _p(_8))
#define _print_9(_1, _2, _3, _4, _5, _6, _7, _8, _9, ...) \
    (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6), _p(_7), _p(_8), _p(_9))
#define _print_10(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...) \
    (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6), _p(_7), _p(_8), _p(_9), _p(_10))
#define _print_11(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, ...) \
    (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6), _p(_7), _p(_8), _p(_9), _p(_10), _p(_11))
#define _print_12(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, ...)                      \
    (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6), _p(_7), _p(_8), _p(_9), _p(_10), _p(_11), \
     _p(_12))
#define _print_13(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, ...)                 \
    (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6), _p(_7), _p(_8), _p(_9), _p(_10), _p(_11), \
     _p(_12), _p(_13))
#define _print_14(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, ...)            \
    (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6), _p(_7), _p(_8), _p(_9), _p(_10), _p(_11), \
     _p(_12), _p(_13), _p(_14))
#define _print_15(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...)       \
    (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6), _p(_7), _p(_8), _p(_9), _p(_10), _p(_11), \
     _p(_12), _p(_13), _p(_14), _p(_15))
#define _print_16(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, ...)  \
    (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6), _p(_7), _p(_8), _p(_9), _p(_10), _p(_11), \
     _p(_12), _p(_13), _p(_14), _p(_15), _p(_16))
#define _print_17(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, ...) \
    (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6), _p(_7), _p(_8), _p(_9), _p(_10), _p(_11),     \
     _p(_12), _p(_13), _p(_14), _p(_15), _p(_16), _p(_17))
#define _print_18(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                  ...)                                                                             \
    (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6), _p(_7), _p(_8), _p(_9), _p(_10), _p(_11),     \
     _p(_12), _p(_13), _p(_14), _p(_15), _p(_16), _p(_17), _p(_18))
#define _print_19(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                  _19, ...)                                                                        \
    (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6), _p(_7), _p(_8), _p(_9), _p(_10), _p(_11),     \
     _p(_12), _p(_13), _p(_14), _p(_15), _p(_16), _p(_17), _p(_18), _p(_19))
#define _print_20(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, \
                  _19, _20, ...)                                                                   \
    (_p(_1), _p(_2), _p(_3), _p(_4), _p(_5), _p(_6), _p(_7), _p(_8), _p(_9), _p(_10), _p(_11),     \
     _p(_12), _p(_13), _p(_14), _p(_15), _p(_16), _p(_17), _p(_18), _p(_19), _p(_20))

#define println(...) (print(__VA_ARGS__), putchar('\n'))