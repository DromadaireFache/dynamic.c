#include <stdio.h>

#include "dynamic.h"

String get_upper(String s) {
    gc_frame();
    String upper = String_concat("this is upper: ", String_upper(s));
    return gc_collect(upper);
}

void test1(void) {
    gc_frame();
    char** list = gc_keep(List_new(char*));

    List_append(list, "hello ");
    List_append(list, "world ");
    List_append(list, "from ");
    List_append(list, "nowhere!");

    printf("my list: %s\n", List_string(list, "%s"));

    String c = String_concat("hey yo", " yo what!!");
    printf("this is concatenated: %s\n", c);

    String s = String_new("abcd1234");
    printf("this is a slice: %s\n", String_slice(s, 0, 7, 2));

    printf("%s\n", get_upper(c));
    printf("%s\n", List_string(s, "%c"));

    double* list2 = List_new(double, 1, 3, 5, 7, 9);
    String list2_string = List_string(list2, "%.2lf");
    printf("List_string -> %s\n", list2_string);

    gc_collect(NULL);
    List_free(list);
}

void test_List_at(void) {
    gc_frame();
    double* list2 = List_new(double, 1, 3, 5, 7, 9);
    printf("list2[3]=%lf\n", List_at(list2, 3));
    printf("list2[-1]=%lf\n", List_at(list2, -1));
    printf("list2[12]=%lf\n", List_at(list2, 6 + 6));
    gc_collect(NULL);
}

void test_nested_list(void) {
    gc_frame();
    double** list = List_new(double*);
    List_append(list, List_new(double, 1, 2, 3));
    List_append(list, List_new(double, 4, 5, 6));
    List_append(list, List_new(double, 7, 8, 9, 10));
    List_append(list, List_new(double, 3));

    printf("nested list -> %s\n", List_string(list, "[%.2lf"));
    gc_collect(NULL);
}

int main() {}