#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

DEFINE_LIST(int)
DEFINE_LIST(float)
DEFINE_LIST_STRING

/* -----------------------------------------------------------------------
 * CHECK macro
 * ----------------------------------------------------------------------- */
#define CHECK(name, condition) do { \
    if (condition) \
        printf("PASS %s\n", (name)); \
    else \
        printf("FAIL %s  (line %d)\n", (name), __LINE__); \
} while (0)

/* -----------------------------------------------------------------------
 * Helpers: count nodes and verify full circularity
 * ----------------------------------------------------------------------- */
static int count_int(intNode *head) {
    if (head == NULL) return 0;
    int n = 0;
    intNode *cur = head;
    do { n++; cur = cur->next; } while (cur != head);
    return n;
}

static int count_str(strNode *head) {
    if (head == NULL) return 0;
    int n = 0;
    strNode *cur = head;
    do { n++; cur = cur->next; } while (cur != head);
    return n;
}

/* Verify every node's prev->next == node and next->prev == node */
static int circular_ok_int(intNode *head) {
    if (head == NULL) return 1;
    intNode *cur = head;
    do {
        if (cur->next->prev != cur) return 0;
        if (cur->prev->next != cur) return 0;
        cur = cur->next;
    } while (cur != head);
    return 1;
}

static int circular_ok_str(strNode *head) {
    if (head == NULL) return 1;
    strNode *cur = head;
    do {
        if (cur->next->prev != cur) return 0;
        if (cur->prev->next != cur) return 0;
        cur = cur->next;
    } while (cur != head);
    return 1;
}

/* ===================================================================
 * INT TESTS
 * =================================================================== */

static void test_int_insert_basic(void) {
    printf("\n--- int: insert basic ---\n");
    intNode *head = NULL;
    int_insert(&head, 10);
    int_insert(&head, 20);
    int_insert(&head, 30);

    CHECK("head is 10",            head->data == 10);
    CHECK("second is 20",          head->next->data == 20);
    CHECK("third is 30",           head->next->next->data == 30);
    CHECK("wraps to head",         head->next->next->next == head);
    CHECK("prev of head is tail",  head->prev->data == 30);
    CHECK("count is 3",            count_int(head) == 3);
    CHECK("circular intact",       circular_ok_int(head));

    int_free_list(&head);
    CHECK("NULL after free",       head == NULL);
}

static void test_int_insert_single(void) {
    printf("\n--- int: single element ---\n");
    intNode *head = NULL;
    int_insert(&head, 42);

    CHECK("value correct",         head->data == 42);
    CHECK("next points to self",   head->next == head);
    CHECK("prev points to self",   head->prev == head);
    CHECK("count is 1",            count_int(head) == 1);

    int_free_list(&head);
}

static void test_int_insert_two(void) {
    printf("\n--- int: two elements ---\n");
    intNode *head = NULL;
    int_insert(&head, 1);
    int_insert(&head, 2);

    CHECK("head is 1",             head->data == 1);
    CHECK("head->next is 2",       head->next->data == 2);
    CHECK("head->next->next back", head->next->next == head);
    CHECK("head->prev is 2",       head->prev->data == 2);
    CHECK("circular intact",       circular_ok_int(head));

    int_free_list(&head);
}

static void test_int_delete_middle(void) {
    printf("\n--- int: delete middle ---\n");
    intNode *head = NULL;
    int_insert(&head, 10);
    int_insert(&head, 20);
    int_insert(&head, 30);

    CHECK("delete hit 0",          int_delete_node(&head, 20) == 0);
    CHECK("head still 10",         head->data == 10);
    CHECK("next is now 30",        head->next->data == 30);
    CHECK("count is 2",            count_int(head) == 2);
    CHECK("circular intact",       circular_ok_int(head));

    int_free_list(&head);
}

static void test_int_delete_head(void) {
    printf("\n--- int: delete head ---\n");
    intNode *head = NULL;
    int_insert(&head, 10);
    int_insert(&head, 20);
    int_insert(&head, 30);

    CHECK("delete head 0",         int_delete_node(&head, 10) == 0);
    CHECK("new head is 20",        head->data == 20);
    CHECK("new head prev is 30",   head->prev->data == 30);
    CHECK("count is 2",            count_int(head) == 2);
    CHECK("circular intact",       circular_ok_int(head));

    int_free_list(&head);
}

static void test_int_delete_tail(void) {
    printf("\n--- int: delete tail ---\n");
    intNode *head = NULL;
    int_insert(&head, 10);
    int_insert(&head, 20);
    int_insert(&head, 30);

    CHECK("delete tail 0",         int_delete_node(&head, 30) == 0);
    CHECK("head still 10",         head->data == 10);
    CHECK("new tail is 20",        head->prev->data == 20);
    CHECK("count is 2",            count_int(head) == 2);
    CHECK("circular intact",       circular_ok_int(head));

    int_free_list(&head);
}

static void test_int_delete_only(void) {
    printf("\n--- int: delete only element ---\n");
    intNode *head = NULL;
    int_insert(&head, 99);

    CHECK("delete only 0",         int_delete_node(&head, 99) == 0);
    CHECK("head is NULL",          head == NULL);
}

static void test_int_delete_miss(void) {
    printf("\n--- int: delete miss ---\n");
    intNode *head = NULL;
    int_insert(&head, 10);
    int_insert(&head, 20);

    CHECK("miss returns -1",       int_delete_node(&head, 99) == -1);
    CHECK("list unchanged",        head->data == 10);
    CHECK("count still 2",         count_int(head) == 2);

    int_free_list(&head);
}

static void test_int_delete_empty(void) {
    printf("\n--- int: delete on empty ---\n");
    intNode *head = NULL;
    CHECK("empty returns -1",      int_delete_node(&head, 10) == -1);
}

static void test_int_delete_duplicate(void) {
    printf("\n--- int: delete first of duplicates ---\n");
    intNode *head = NULL;
    int_insert(&head, 10);
    int_insert(&head, 20);
    int_insert(&head, 20);
    int_insert(&head, 30);

    CHECK("delete dup hit 0",      int_delete_node(&head, 20) == 0);
    CHECK("count is 3",            count_int(head) == 3);
    /* second 20 should still exist */
    CHECK("second 20 still there", int_search(head, 20) != NULL);
    CHECK("circular intact",       circular_ok_int(head));

    int_free_list(&head);
}

static void test_int_search(void) {
    printf("\n--- int: search ---\n");
    intNode *head = NULL;
    int_insert(&head, 10);
    int_insert(&head, 20);
    int_insert(&head, 30);

    intNode *found = int_search(head, 20);
    CHECK("search hit non-NULL",   found != NULL);
    CHECK("search hit value",      found && found->data == 20);
    CHECK("search miss NULL",      int_search(head, 99) == NULL);
    CHECK("search head",           int_search(head, 10) != NULL);
    CHECK("search tail",           int_search(head, 30) != NULL);

    int_free_list(&head);
}

static void test_int_search_empty(void) {
    printf("\n--- int: search on empty ---\n");
    intNode *head = NULL;
    CHECK("search empty NULL",     int_search(head, 10) == NULL);
}

static void test_int_free_empty(void) {
    printf("\n--- int: free empty list ---\n");
    intNode *head = NULL;
    int_free_list(&head);   /* must not crash */
    CHECK("still NULL",            head == NULL);
}

static void test_int_reverse_three(void) {
    printf("\n--- int: reverse three elements ---\n");
    intNode *head = NULL;
    int_insert(&head, 1);
    int_insert(&head, 2);
    int_insert(&head, 3);

    int_reverse(&head);
    CHECK("new head is 3",         head->data == 3);
    CHECK("second is 2",           head->next->data == 2);
    CHECK("third is 1",            head->next->next->data == 1);
    CHECK("wraps to head",         head->next->next->next == head);
    CHECK("count still 3",         count_int(head) == 3);
    CHECK("circular intact",       circular_ok_int(head));

    int_free_list(&head);
}

static void test_int_reverse_single(void) {
    printf("\n--- int: reverse single element ---\n");
    intNode *head = NULL;
    int_insert(&head, 42);
    int_reverse(&head);
    CHECK("single reverse value",  head->data == 42);
    CHECK("still self-circular",   head->next == head);
    int_free_list(&head);
}

static void test_int_reverse_twice(void) {
    printf("\n--- int: reverse twice = original ---\n");
    intNode *head = NULL;
    int_insert(&head, 1);
    int_insert(&head, 2);
    int_insert(&head, 3);

    int_reverse(&head);
    int_reverse(&head);

    CHECK("head back to 1",        head->data == 1);
    CHECK("second back to 2",      head->next->data == 2);
    CHECK("third back to 3",       head->next->next->data == 3);
    CHECK("circular intact",       circular_ok_int(head));

    int_free_list(&head);
}

static void test_int_reverse_empty(void) {
    printf("\n--- int: reverse empty ---\n");
    intNode *head = NULL;
    intNode *r = int_reverse(&head);
    CHECK("reverse empty NULL",    r == NULL);
    CHECK("head still NULL",       head == NULL);
}

/* ===================================================================
 * FLOAT TESTS
 * =================================================================== */

static void test_float_insert(void) {
    printf("\n--- float: insert and circular ---\n");
    floatNode *head = NULL;
    float_insert(&head, 1.5f);
    float_insert(&head, 2.5f);
    float_insert(&head, 3.5f);

    CHECK("head is 1.5",           head->data == 1.5f);
    CHECK("second is 2.5",         head->next->data == 2.5f);
    CHECK("wraps to head",         head->next->next->next == head);
    CHECK("prev is 3.5",           head->prev->data == 3.5f);

    float_free_list(&head);
    CHECK("NULL after free",       head == NULL);
}

static void test_float_delete(void) {
    printf("\n--- float: delete ---\n");
    floatNode *head = NULL;
    float_insert(&head, 1.0f);
    float_insert(&head, 2.0f);
    float_insert(&head, 3.0f);

    CHECK("delete hit 0",          float_delete_node(&head, 2.0f) == 0);
    CHECK("head still 1.0",        head->data == 1.0f);
    CHECK("next is 3.0",           head->next->data == 3.0f);

    float_free_list(&head);
}

/* ===================================================================
 * STRING TESTS
 * =================================================================== */

static void test_str_insert_basic(void) {
    printf("\n--- str: insert basic ---\n");
    strNode *head = NULL;
    str_insert(&head, "khanhquyen");
    str_insert(&head, "khoihuynh");
    str_insert(&head, "xuandiep");

    CHECK("head value",            strcmp(head->data, "khanhquyen") == 0);
    CHECK("second value",          strcmp(head->next->data, "khoihuynh") == 0);
    CHECK("third value",           strcmp(head->next->next->data, "xuandiep") == 0);
    CHECK("wraps to head",         head->next->next->next == head);
    CHECK("str content correct",   strcmp(head->data, "khanhquyen") == 0);
    CHECK("count is 3",            count_str(head) == 3);
    CHECK("circular intact",       circular_ok_str(head));

    str_free_list(&head);
    CHECK("NULL after free",       head == NULL);
}

static void test_str_insert_single(void) {
    printf("\n--- str: single element ---\n");
    strNode *head = NULL;
    str_insert(&head, "only");

    CHECK("value correct",         strcmp(head->data, "only") == 0);
    CHECK("next points self",      head->next == head);
    CHECK("prev points self",      head->prev == head);

    str_free_list(&head);
}

static void test_str_delete_middle(void) {
    printf("\n--- str: delete middle ---\n");
    strNode *head = NULL;
    str_insert(&head, "a");
    str_insert(&head, "b");
    str_insert(&head, "c");

    CHECK("delete hit 0",          str_delete(&head, "b") == 0);
    CHECK("head still a",          strcmp(head->data, "a") == 0);
    CHECK("next is c",             strcmp(head->next->data, "c") == 0);
    CHECK("count is 2",            count_str(head) == 2);
    CHECK("circular intact",       circular_ok_str(head));

    str_free_list(&head);
}

static void test_str_delete_head(void) {
    printf("\n--- str: delete head ---\n");
    strNode *head = NULL;
    str_insert(&head, "first");
    str_insert(&head, "second");
    str_insert(&head, "third");

    CHECK("delete head 0",         str_delete(&head, "first") == 0);
    CHECK("new head is second",    strcmp(head->data, "second") == 0);
    CHECK("new tail is third",     strcmp(head->prev->data, "third") == 0);
    CHECK("circular intact",       circular_ok_str(head));

    str_free_list(&head);
}

static void test_str_delete_tail(void) {
    printf("\n--- str: delete tail ---\n");
    strNode *head = NULL;
    str_insert(&head, "first");
    str_insert(&head, "second");
    str_insert(&head, "third");

    CHECK("delete tail 0",         str_delete(&head, "third") == 0);
    CHECK("head still first",      strcmp(head->data, "first") == 0);
    CHECK("new tail is second",    strcmp(head->prev->data, "second") == 0);
    CHECK("circular intact",       circular_ok_str(head));

    str_free_list(&head);
}

static void test_str_delete_only(void) {
    printf("\n--- str: delete only element ---\n");
    strNode *head = NULL;
    str_insert(&head, "only");

    CHECK("delete only 0",         str_delete(&head, "only") == 0);
    CHECK("head is NULL",          head == NULL);
}

static void test_str_delete_miss(void) {
    printf("\n--- str: delete miss ---\n");
    strNode *head = NULL;
    str_insert(&head, "hello");
    str_insert(&head, "world");

    CHECK("miss returns -1",       str_delete(&head, "nothere") == -1);
    CHECK("count unchanged",       count_str(head) == 2);

    str_free_list(&head);
}

static void test_str_delete_empty(void) {
    printf("\n--- str: delete on empty ---\n");
    strNode *head = NULL;
    CHECK("empty returns -1",      str_delete(&head, "x") == -1);
}

static void test_str_search(void) {
    printf("\n--- str: search ---\n");
    strNode *head = NULL;
    str_insert(&head, "hello");
    str_insert(&head, "world");
    str_insert(&head, "foo");

    strNode *found = str_search(head, "world");
    CHECK("search hit non-NULL",   found != NULL);
    CHECK("search hit value",      found && strcmp(found->data, "world") == 0);
    CHECK("search miss NULL",      str_search(head, "nothere") == NULL);
    CHECK("search head",           str_search(head, "hello") != NULL);
    CHECK("search tail",           str_search(head, "foo") != NULL);

    str_free_list(&head);
}

static void test_str_search_empty(void) {
    printf("\n--- str: search empty ---\n");
    strNode *head = NULL;
    CHECK("search empty NULL",     str_search(head, "x") == NULL);
}

static void test_str_free_empty(void) {
    printf("\n--- str: free empty list ---\n");
    strNode *head = NULL;
    str_free_list(&head);   /* must not crash */
    CHECK("still NULL",            head == NULL);
}

static void test_str_reverse(void) {
    printf("\n--- str: reverse ---\n");
    strNode *head = NULL;
    str_insert(&head, "a");
    str_insert(&head, "b");
    str_insert(&head, "c");

    str_reverse(&head);
    CHECK("new head is c",         strcmp(head->data, "c") == 0);
    CHECK("second is b",           strcmp(head->next->data, "b") == 0);
    CHECK("third is a",            strcmp(head->next->next->data, "a") == 0);
    CHECK("count still 3",         count_str(head) == 3);
    CHECK("circular intact",       circular_ok_str(head));

    str_free_list(&head);
}

static void test_str_reverse_twice(void) {
    printf("\n--- str: reverse twice = original ---\n");
    strNode *head = NULL;
    str_insert(&head, "x");
    str_insert(&head, "y");
    str_insert(&head, "z");

    str_reverse(&head);
    str_reverse(&head);

    CHECK("head back to x",        strcmp(head->data, "x") == 0);
    CHECK("second back to y",      strcmp(head->next->data, "y") == 0);
    CHECK("third back to z",       strcmp(head->next->next->data, "z") == 0);
    CHECK("circular intact",       circular_ok_str(head));

    str_free_list(&head);
}

static void test_str_reverse_empty(void) {
    printf("\n--- str: reverse empty ---\n");
    strNode *head = NULL;
    strNode *r = str_reverse(&head);
    CHECK("reverse empty NULL",    r == NULL);
}

static void test_str_empty_string(void) {
    printf("\n--- str: empty string value ---\n");
    strNode *head = NULL;
    str_insert(&head, "");
    CHECK("empty string stored",   strcmp(head->data, "") == 0);
    CHECK("search empty string",   str_search(head, "") != NULL);
    CHECK("delete empty string",   str_delete(&head, "") == 0);
    CHECK("head NULL after",       head == NULL);
}

/* ===================================================================
 * main
 * =================================================================== */
int main(void) {
    printf("=== macro list tests ===\n");

    /* int */
    test_int_insert_basic();
    test_int_insert_single();
    test_int_insert_two();
    test_int_delete_middle();
    test_int_delete_head();
    test_int_delete_tail();
    test_int_delete_only();
    test_int_delete_miss();
    test_int_delete_empty();
    test_int_delete_duplicate();
    test_int_search();
    test_int_search_empty();
    test_int_free_empty();
    test_int_reverse_three();
    test_int_reverse_single();
    test_int_reverse_twice();
    test_int_reverse_empty();

    /* float */
    test_float_insert();
    test_float_delete();

    /* string */
    test_str_insert_basic();
    test_str_insert_single();
    test_str_delete_middle();
    test_str_delete_head();
    test_str_delete_tail();
    test_str_delete_only();
    test_str_delete_miss();
    test_str_delete_empty();
    test_str_search();
    test_str_search_empty();
    test_str_free_empty();
    test_str_reverse();
    test_str_reverse_twice();
    test_str_reverse_empty();
    test_str_empty_string();

    printf("\n=== done ===\n");
    return 0;
}