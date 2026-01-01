#include "dynamic.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 0

typedef struct GCItem {
    void* ptr;
    free_fn_t free_fn;
} GCItem;

static GCItem** gc = NULL;
static size_t gc_tracked = 0, gc_freed = 0, gc_untracked = 0;

static GCItem* gc_pop_frame(void) { return len(gc) > 0 ? gc[len(gc) - 1] : NULL; }

#if DEBUG == 1
#define GC_INFO(...) printf("GC INFO *** " __VA_ARGS__)
#else
#define GC_INFO(...)
#endif

_ListHeader* _List_get_header(void* list) { return (_ListHeader*)list - 1; }

size_t len(void* list) { return _List_get_header(list)->length; }

static void* _List_new_untracked(size_t element_size, size_t capacity) {
    _ListHeader* head = malloc(sizeof(_ListHeader) + element_size * capacity);
    if (head == NULL) return NULL;
    head->capacity = capacity;
    head->length = 0;
    head->element_size = element_size;
    return (void*)&head[1];
}

void* _List_new(size_t element_size, size_t capacity) {
    void* list = _List_new_untracked(element_size, capacity);
    gc_track(list, List_free);
    return list;
}

void* _List_from_array(size_t elem_size, const void* arr, size_t n) {
    void* list = _List_new(elem_size, n > 10 ? n : 10);

    if (n) {
        memcpy(list, arr, elem_size * n);
        _List_get_header(list)->length = n;
    }

    return list;
}

static void* _List_resize(void* list, size_t new_capacity, bool update_ptr) {
    _ListHeader* head = _List_get_header(list);
    _ListHeader* new_head = realloc(head, sizeof(_ListHeader) + head->element_size * new_capacity);
    if (new_head == NULL) return list;  // Fail-safe
    new_head->capacity = new_capacity;
    void* new_list = (void*)&new_head[1];

    // if List is being tracked, update the pointer
    GCItem* frame = gc_pop_frame();
    if (update_ptr && frame != NULL) {
        foreach (object, frame) {
            if (object.ptr == list) frame[i].ptr = new_list;
        }
    }

    return new_list;
}

void* __attribute__((warn_unused_result)) List_resize(void* list, size_t new_capacity) {
    return _List_resize(list, new_capacity, true);
}

void List_free(void* list) { free(_List_get_header(list)); }

void List_remove(void* list, size_t i, void* output) {
    assert(i >= 0 && i < len(list));
    _ListHeader* h = _List_get_header(list);
    h->length--;
    size_t tail = len(list) - i;
    if (output != NULL) memcpy(output, list, h->element_size);
    if (tail) {
        char* base = (char*)list;
        size_t esz = h->element_size;
        memmove(base + i * esz, base + (i + 1) * esz, tail * esz);
    }
}

void List_pop(void* list, void* output) {
    assert(len(list) > 0);
    List_remove(list, len(list) - 1, output);
}

size_t _List_convert_idx(void* list, int idx, const char* _fn, const char* _file, int _ln) {
    int _idx = (idx < 0) ? idx + len(list) : idx;
    (__builtin_expect(!(_idx >= 0 && _idx < len(list)), 0)
         ? __assert_rtn(_fn, _file, _ln, String_new("index %d out of bounds", idx))
         : (void)0);
    return (size_t)_idx;
}

void List_clear(void* list) { _List_get_header(list)->length = 0; }

int _List_index(void* list, const void* value) {
    const size_t element_size = _List_get_header(list)->element_size;
    for (int i = 0; i < len(list); i++) {
        if (memcmp(list + i * element_size, value, element_size) == 0) return i;
    }
    return -1;
}

int _List_index_fn(void* list, void* value, cmp_fn_t cmp_fn) {
    const size_t element_size = _List_get_header(list)->element_size;
    for (int i = 0; i < len(list); i++) {
        if (cmp_fn(*(void**)(list + i * element_size), value)) return i;
    }
    return -1;
}

void* _List_repeat(void* list, size_t count) {
    _ListHeader* head = _List_get_header(list);
    size_t new_length = head->length * count;
    size_t old_size = head->length * head->element_size;
    void* new_list = _List_new(head->element_size, new_length > 10 ? new_length : 10);

    for (int i = 0; i < count; i++) {
        memcpy(new_list + i * old_size, list, old_size);
    }

    _List_get_header(new_list)->length = new_length;
    return new_list;
}

void* _List_copy(void* list) {
    _ListHeader* head = _List_get_header(list);
    void* new_list = _List_new(head->element_size, head->capacity);
    memcpy(new_list, list, head->length * head->element_size);
    _List_get_header(new_list)->length = head->length;
    return new_list;
}

void _List_sort(void* list, int (*cmp_fn)(const void*, const void*)) {
    qsort(list, len(list), _List_get_header(list)->element_size, cmp_fn);
}

static char* _get_format_type(const char* _Format) {
    char* format_type = (char*)_Format;
    char* ignore_chars = "%.-1234567890";
    while (*format_type) {
        bool found = false;
        for (char* c = ignore_chars; *c != 0; c++) {
            if (*format_type == *c) {
                found = true;
                break;
            }
        }
        if (found)
            format_type++;
        else
            break;
    }
    return format_type;
}

static String _String_from_format(const char* _Format, const char* _Format_type, void* item) {
    if (_Format_type == NULL) _Format_type = _get_format_type(_Format);

#define _FMT_CHECK(fmt, type) \
    if (String_equals((String)_Format_type, (fmt))) return String_new(_Format, *(type*)item)

    _FMT_CHECK("d", int);
    if (String_equals((String)_Format_type, ("s")))
        return String_new("\"%s\"", String_new(_Format, *(char**)item));
    _FMT_CHECK("zu", size_t);
    _FMT_CHECK("f", float);
    _FMT_CHECK("lf", double);
    _FMT_CHECK("p", void*);
    if (String_equals((String)_Format_type, ("c")))
        return String_new("'%s'", String_new(_Format, *(char*)item));
    _FMT_CHECK("i", int);
    _FMT_CHECK("u", unsigned int);
    _FMT_CHECK("o", unsigned int);
    _FMT_CHECK("x", unsigned int);
    _FMT_CHECK("X", unsigned int);
    _FMT_CHECK("F", float);
    _FMT_CHECK("e", double);
    _FMT_CHECK("E", double);
    _FMT_CHECK("g", double);
    _FMT_CHECK("G", double);
    _FMT_CHECK("a", double);
    _FMT_CHECK("A", double);
    _FMT_CHECK("ld", long);
    _FMT_CHECK("li", long);
    _FMT_CHECK("lu", unsigned long);
    _FMT_CHECK("lo", unsigned long);
    _FMT_CHECK("lx", unsigned long);
    _FMT_CHECK("lX", unsigned long);
    _FMT_CHECK("lld", long long);
    _FMT_CHECK("lli", long long);
    _FMT_CHECK("llu", unsigned long long);
    _FMT_CHECK("llo", unsigned long long);
    _FMT_CHECK("llx", unsigned long long);
    _FMT_CHECK("llX", unsigned long long);
    _FMT_CHECK("hd", short);
    _FMT_CHECK("hi", short);
    _FMT_CHECK("hu", unsigned short);
    _FMT_CHECK("ho", unsigned short);
    _FMT_CHECK("hx", unsigned short);
    _FMT_CHECK("hX", unsigned short);
    _FMT_CHECK("hhd", signed char);
    _FMT_CHECK("hhi", signed char);
    _FMT_CHECK("hhu", unsigned char);
    _FMT_CHECK("hho", unsigned char);
    _FMT_CHECK("hhx", unsigned char);
    _FMT_CHECK("hhX", unsigned char);
    _FMT_CHECK("Lf", long double);
    _FMT_CHECK("Lf", long double);
    _FMT_CHECK("Le", long double);
    _FMT_CHECK("LE", long double);
    _FMT_CHECK("Lg", long double);
    _FMT_CHECK("LG", long double);
    _FMT_CHECK("zd", ssize_t);
    _FMT_CHECK("td", ptrdiff_t);
    return String_new("<cannot use '%s' on %p>", _Format, item);
}

String String_from_format(const char* _Format, void* item) {
    return _String_from_format(_Format, NULL, item);
}

String List_string(void* list, const char* _Format) {
    assert(_Format != NULL && _Format[0] != 0);
    collected;
    String* sb = List_new(String);
    _ListHeader* head = _List_get_header(list);
    size_t list_size = head->length * head->element_size;
    const char* format_type = _get_format_type(_Format);
    // printf("format_type=%s\n", format_type);

    for (int i = 0; i < list_size; i += head->element_size) {
        String element_string;
        if (_Format[0] == '[') {
            void** plist = (void**)list;
            element_string = List_string(plist[i / head->element_size], _Format + 1);
        } else {
            element_string = _String_from_format(_Format, format_type, &list[i]);
        }
        List_append(sb, element_string);
    }

    String elements = String_join(", ", sb);
    return gc_collect(String_new("[%s]", elements));
}

__attribute__((constructor)) static void gc_init(void) {
    gc = _List_new_untracked(sizeof(GCItem*), 10);
    GCItem* frame = _List_new_untracked(sizeof(GCItem), 10);
    List_append(gc, frame);
}

__attribute__((destructor)) static void gc_cleanup(void) {
    foreach (frame, gc) {
        foreach (object, frame) {
            GC_INFO("free(object=%p);\n", object.ptr);
            object.free_fn(object.ptr);
            gc_freed++;
        }
        GC_INFO("free(frame=%p);\n", _List_get_header(frame));
        free(_List_get_header(frame));
    }
    GC_INFO("free(gc=%p);\n", _List_get_header(gc));
    free(_List_get_header(gc));
    GC_INFO("Final stats: tracked %zu, untracked %zu, freed %zu.\n", gc_tracked, gc_untracked,
            gc_freed);
}

#define _List_append_noupdate(list, item)                                                         \
    {                                                                                             \
        _ListHeader* head = _List_get_header(list);                                               \
        list[head->length++] = (item);                                                            \
        if (head->length >= head->capacity) list = _List_resize(list, head->capacity * 2, false); \
    }

void* gc_track(void* p, free_fn_t free_fn) {
    GCItem* frame = gc_pop_frame();
    if (frame == NULL) return p;
    if (free_fn == NULL) free_fn = free;
    _List_append_noupdate(frame, ((GCItem){.ptr = p, .free_fn = free_fn}));
    gc[len(gc) - 1] = frame;  // In case frame gets resized
    GC_INFO("Frame #%zu: tracking %p\n", len(gc), p);
    gc_tracked++;
    return p;
}

void gc_frame(void) {
    GCItem* frame = _List_new_untracked(sizeof(GCItem), 10);
    List_append(gc, frame);
}

void* gc_keep(void* p) {
    GCItem* frame = gc_pop_frame();
    if (frame == NULL) return p;

    foreach (object, frame) {
        if (object.ptr != p) continue;
        List_remove(frame, i, NULL);
        gc_untracked++;
        break;
    }

    GC_INFO("Frame #%zu: untracking %p\n", len(gc), p);

    return p;
}

void* gc_collect(void* p) {
    GCItem* frame = gc_pop_frame();
    if (frame == NULL) return p;

    bool found = false;
    GCItem object_found;

    foreach (object, frame) {
        if (p != NULL && object.ptr == p) {
            found = true;
            object_found = object;
            continue;
        }
        GC_INFO("collected: free(object=%p);\n", object.ptr);
        object.free_fn(object.ptr);
        gc_freed++;
    }
    GC_INFO("free(frame=%p);\n", _List_get_header(frame));
    free(_List_get_header(frame));
    List_pop(gc, NULL);

    if (found) {
        GCItem* frame = gc_pop_frame();
        if (frame != NULL) {
            _List_append_noupdate(frame, object_found);
            gc[len(gc) - 1] = frame;  // In case frame gets resized
        }
    }

    return p;
}

void* gc_calloc(size_t count, size_t size) { return gc_track(calloc(count, size), free); }

void* gc_malloc(size_t size) { return gc_track(malloc(size), free); }

void* gc_realloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    GCItem* frame = gc_pop_frame();
    foreach (object, frame) {
        if (object.ptr == ptr) frame[i].ptr = new_ptr;
    }
    return new_ptr;
}

size_t _gc_frame_nbr(void) { return len(gc); }

String String_new(const char* _Format, ...) {
    String s = List_new(char);
    va_list args;
    va_start(args, _Format);

    int length = vsnprintf(NULL, 0, _Format, args) + 1;
    if (length <= 0) {  // in case nothing can be printed
        List_append(s, (char)0);
        return s;
    }

    s = List_resize(s, length);
    vsnprintf(s, length, _Format, args);

    _ListHeader* head = _List_get_header(s);
    head->length = length - 1;

    va_end(args);
    return s;
}

void String_free(String s) { List_free(s); }

String String_concat(String s1, String s2) { return String_new("%s%s", s1, s2); }

String String_slice(String s, int start, int last, int step) {
    size_t len_s = strlen(s);
    if (start < 0) start += len_s;
    if (last < 0) last += len_s + 1;

    if (start == last) return String_new("");
    assert(start >= 0 && start < len_s);
    assert(last > 0 && last <= len_s);
    assert(last > start);
    assert(step != 0);

    int k = step > 0 ? step : -step;
    size_t new_len = ((last - start) + k - 1) / k;

    String slice = List_new(char);
    slice = List_resize(slice, new_len + 1);

    int j = 0;
    if (step > 0)
        for (int i = start; i < last; i += step) slice[j++] = s[i];
    else
        for (int i = last - 1; i >= start; i += step) slice[j++] = s[i];

    slice[j] = 0;
    return slice;
}

String String_capitalize(String s) {
    String result = String_new(s);
    if (len(result) > 0 && islower(result[0])) result[0] = toupper(result[0]);
    return result;
}

String String_upper(String s) {
    String result = String_new(s);
    for (char* c = result; *c != 0; c++) {
        if (islower(*c)) *c = toupper(*c);
    }
    return result;
}

String String_lower(String s) {
    String result = String_new(s);
    for (char* c = result; *c != 0; c++) {
        if (isupper(*c)) *c = tolower(*c);
    }
    return result;
}

bool String_equals(String s1, String s2) { return strcmp(s1, s2) == 0; }

String String_join(String sep, String* list) {
    size_t len_sep = strlen(sep);
    size_t length = 0;
    foreach (s, list) {
        if (i) length += len_sep;
        length += strlen(s);
    }

    String result = List_new(char);
    result = List_resize(result, length + 1);
    char* p = result;

    foreach (t, list) {
        if (i) {
            memcpy(p, sep, len_sep);
            p += len_sep;
        }
        size_t len_s = strlen(t);
        memcpy(p, t, len_s);
        p += len_s;
    }

    *p = 0;
    return result;
}

bool String_isalpha(String s) {
    char* c = s;
    while (*c) {
        if (!isalpha(*c)) return false;
        c++;
    }
    return true;
}

bool String_isdigit(String s) {
    char* c = s;
    while (*c) {
        if (!isdigit(*c)) return false;
        c++;
    }
    return true;
}

bool String_isalnum(String s) {
    char* c = s;
    while (*c) {
        if (!isalnum(*c)) return false;
        c++;
    }
    return true;
}

bool String_startswith(String s, String prefix) {
    if (strlen(prefix) > strlen(s)) return false;
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

bool String_endswith(String s, String suffix) {
    if (strlen(suffix) > strlen(s)) return false;
    return strncmp(s + strlen(s) - strlen(suffix), suffix, strlen(suffix)) == 0;
}

bool String_contains(String s, char c) {
    while (*s)
        if (*s++ == c) return true;
    return false;
}

String String_strip(String s, char* characters) {
    int start = 0, end = strlen(s);

    while (start < end && String_contains(characters, s[start])) start++;
    while (end - 1 >= start && String_contains(characters, s[end - 1])) end--;

    return String_slice(s, start, end, 1);
}