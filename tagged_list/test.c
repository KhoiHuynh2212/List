#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

#define CHECK(name, condition) do { \
    if (condition) \
        printf("PASS %s\n", name); \
    else \
        printf("FAIL %s\n", name); \
} while(0)

/* ═════════════════════════════════════════
   INSERT INT
   ═════════════════════════════════════════ */
void test_insertIntNode() {
    printf("\n--- insertIntNode ---\n");
    Node *head = NULL;

    // happy path
    insertIntNode(&head, 10);
    CHECK("insert into empty list", head != NULL);
    CHECK("head data correct", head->data.node_int == 10);
    CHECK("head kind is INTEGER", head->kind == INTEGER);
    CHECK("head->prev is NULL", head->prev == NULL);
    CHECK("head->next is NULL", head->next == NULL);

    // append second
    insertIntNode(&head, 20);
    CHECK("second node data", head->next->data.node_int == 20);
    CHECK("second node prev points to head", head->next->prev == head);
    CHECK("second node next is NULL", head->next->next == NULL);

    // append third
    insertIntNode(&head, 30);
    CHECK("third node data", head->next->next->data.node_int == 30);

    // deep copy — mutating original shouldn't affect list
    // (int is passed by value so this is always safe, just confirming)
    CHECK("kind stays INTEGER after multiple inserts", head->kind == INTEGER);

    free_list(&head);
    CHECK("head NULL after free", head == NULL);
}

/* ═════════════════════════════════════════
   INSERT STRING
   ═════════════════════════════════════════ */
void test_insertStringNode() {
    printf("\n--- insertStringNode ---\n");
    Node *head = NULL;

    // happy path
    insertStringNode(&head, "hello");
    CHECK("insert into empty list", head != NULL);
    CHECK("head data correct", strcmp(head->data.node_string, "hello") == 0);
    CHECK("head kind is STRING", head->kind == STRING);
    CHECK("head->prev is NULL", head->prev == NULL);

    // deep copy — mutating source string shouldn't affect list
    char src[] = "world";
    insertStringNode(&head, src);
    src[0] = 'X';
    CHECK("deep copy: mutation doesn't affect node", strcmp(head->next->data.node_string, "world") == 0);

    // prev/next links
    CHECK("second->prev points to head", head->next->prev == head);
    CHECK("second->next is NULL", head->next->next == NULL);

    // empty string edge case
    insertStringNode(&head, "");
    CHECK("empty string inserted", strcmp(head->next->next->data.node_string, "") == 0);

    free_list(&head);
    CHECK("head NULL after free", head == NULL);
}

/* ═════════════════════════════════════════
   MIXED LIST
   ═════════════════════════════════════════ */
void test_mixed_list() {
    printf("\n--- mixed list ---\n");
    Node *head = NULL;

    insertIntNode(&head, 1);
    insertStringNode(&head, "two");
    insertIntNode(&head, 3);
    insertStringNode(&head, "four");

    CHECK("node1 kind INTEGER", head->kind == INTEGER);
    CHECK("node2 kind STRING",  head->next->kind == STRING);
    CHECK("node3 kind INTEGER", head->next->next->kind == INTEGER);
    CHECK("node4 kind STRING",  head->next->next->next->kind == STRING);

    // prev links in mixed list
    CHECK("node2->prev = node1", head->next->prev == head);
    CHECK("node3->prev = node2", head->next->next->prev == head->next);

    free_list(&head);
}

/* ═════════════════════════════════════════
   DELETE INT
   ═════════════════════════════════════════ */
void test_deleteIntNode() {
    printf("\n--- deleteIntNode ---\n");
    Node *head = NULL;
    insertIntNode(&head, 10);
    insertIntNode(&head, 20);
    insertIntNode(&head, 30);

    // miss
    CHECK("returns -1 on miss", deleteIntNode(&head, 99) == -1);

    // delete middle
    CHECK("returns 0 on hit", deleteIntNode(&head, 20) == 0);
    CHECK("middle deleted: head->next is 30", head->next->data.node_int == 30);
    CHECK("middle deleted: next->prev = head", head->next->prev == head);

    // delete head
    deleteIntNode(&head, 10);
    CHECK("head deleted: new head is 30", head->data.node_int == 30);
    CHECK("new head->prev is NULL", head->prev == NULL);

    // delete tail (only node left)
    deleteIntNode(&head, 30);
    CHECK("list empty after deleting last node", head == NULL);

    // miss on empty list
    CHECK("miss on empty list returns -1", deleteIntNode(&head, 10) == -1);

    free_list(&head);
}

void test_deleteIntNode_mixed() {
    printf("\n--- deleteIntNode on mixed list ---\n");
    Node *head = NULL;
    insertStringNode(&head, "hello");
    insertIntNode(&head, 42);
    insertStringNode(&head, "world");

    // should not delete string node even if data matches by accident
    CHECK("skips STRING nodes", deleteIntNode(&head, 42) == 0);
    CHECK("string nodes intact", strcmp(head->data.node_string, "hello") == 0);
    CHECK("second string intact", strcmp(head->next->data.node_string, "world") == 0);

    free_list(&head);
}

/* ═════════════════════════════════════════
   DELETE STRING
   ═════════════════════════════════════════ */
void test_deleteStringNode() {
    printf("\n--- deleteStringNode ---\n");
    Node *head = NULL;
    insertStringNode(&head, "apple");
    insertStringNode(&head, "banana");
    insertStringNode(&head, "cherry");

    // miss
    CHECK("returns -1 on miss", deleteStringNode(&head, "mango") == -1);

    // delete middle
    CHECK("returns 0 on hit", deleteStringNode(&head, "banana") == 0);
    CHECK("middle deleted: head->next is cherry", strcmp(head->next->data.node_string, "cherry") == 0);
    CHECK("middle deleted: next->prev = head", head->next->prev == head);

    // delete head
    deleteStringNode(&head, "apple");
    CHECK("head deleted: new head is cherry", strcmp(head->data.node_string, "cherry") == 0);
    CHECK("new head->prev is NULL", head->prev == NULL);

    // delete tail (only node left)
    deleteStringNode(&head, "cherry");
    CHECK("list empty after deleting last node", head == NULL);

    // miss on empty list
    CHECK("miss on empty list returns -1", deleteStringNode(&head, "apple") == -1);

    free_list(&head);
}

void test_deleteStringNode_mixed() {
    printf("\n--- deleteStringNode on mixed list ---\n");
    Node *head = NULL;
    insertIntNode(&head, 10);
    insertStringNode(&head, "target");
    insertIntNode(&head, 20);

    CHECK("skips INTEGER nodes", deleteStringNode(&head, "target") == 0);
    CHECK("int nodes intact after delete", head->data.node_int == 10);
    CHECK("second int intact", head->next->data.node_int == 20);

    free_list(&head);
}

/* ═════════════════════════════════════════
   SEARCH INT
   ═════════════════════════════════════════ */
void test_searchInt() {
    printf("\n--- searchInt ---\n");
    Node *head = NULL;
    insertIntNode(&head, 10);
    insertIntNode(&head, 20);
    insertIntNode(&head, 30);

    // happy path
    Node *found = searchInt(head, 20);
    CHECK("search hit returns non-NULL", found != NULL);
    CHECK("search hit returns correct node", found->data.node_int == 20);

    // head and tail
    CHECK("search head", searchInt(head, 10) == head);
    CHECK("search tail", searchInt(head, 30)->data.node_int == 30);

    // miss
    CHECK("search miss returns NULL", searchInt(head, 99) == NULL);

    // empty list
    Node *empty = NULL;
    CHECK("search empty list returns NULL", searchInt(empty, 10) == NULL);

    free_list(&head);
}

/* ═════════════════════════════════════════
   SEARCH STRING
   ═════════════════════════════════════════ */
void test_searchString() {
    printf("\n--- searchString ---\n");
    Node *head = NULL;
    insertStringNode(&head, "apple");
    insertStringNode(&head, "banana");
    insertStringNode(&head, "cherry");

    // happy path
    Node *found = searchString(head, "banana");
    CHECK("search hit returns non-NULL", found != NULL);
    CHECK("search hit returns correct node", strcmp(found->data.node_string, "banana") == 0);

    // head and tail
    CHECK("search head", searchString(head, "apple") == head);
    CHECK("search tail", strcmp(searchString(head, "cherry")->data.node_string, "cherry") == 0);

    // miss
    CHECK("search miss returns NULL", searchString(head, "mango") == NULL);

    // empty list
    Node *empty = NULL;
    CHECK("search empty list returns NULL", searchString(empty, "apple") == NULL);

    // empty string
    insertStringNode(&head, "");
    CHECK("search empty string", searchString(head, "") != NULL);

    free_list(&head);
}

/* ═════════════════════════════════════════
   STRESS TEST
   ═════════════════════════════════════════ */
void test_stress_insert_delete() {
    printf("\n--- stress: insert + delete 1000 nodes ---\n");
    Node *head = NULL;

    // insert 1000 ints
    for (int i = 0; i < 1000; i++)
        insertIntNode(&head, i);

    // delete all even numbers
    for (int i = 0; i < 1000; i += 2)
        deleteIntNode(&head, i);

    // verify only odd numbers remain, prev links intact
    Node *curr = head;
    int ok = 1;
    while (curr != NULL) {
        if (curr->data.node_int % 2 == 0) { ok = 0; break; }
        if (curr->next != NULL && curr->next->prev != curr) { ok = 0; break; }
        curr = curr->next;
    }
    CHECK("only odd numbers remain, prev links intact", ok);

    free_list(&head);
    CHECK("head NULL after stress free", head == NULL);
}

void test_stress_mixed() {
    printf("\n--- stress: mixed list 500 nodes ---\n");
    Node *head = NULL;

    for (int i = 0; i < 250; i++) {
        insertIntNode(&head, i);
        insertStringNode(&head, "x");
    }

    // delete all strings
    while (deleteStringNode(&head, "x") == 0);

    // verify only ints remain
    Node *curr = head;
    int ok = 1;
    while (curr != NULL) {
        if (curr->kind != INTEGER) { ok = 0; break; }
        curr = curr->next;
    }
    CHECK("only ints remain after deleting all strings", ok);

    free_list(&head);
}

/* ═════════════════════════════════════════
   FREE LIST
   ═════════════════════════════════════════ */
void test_free_list() {
    printf("\n--- free_list ---\n");
    Node *head = NULL;

    // free empty list
    free_list(&head);
    CHECK("free empty list is safe", head == NULL);

    // free single node
    insertIntNode(&head, 1);
    free_list(&head);
    CHECK("free single int node", head == NULL);

    // free single string node
    insertStringNode(&head, "hello");
    free_list(&head);
    CHECK("free single string node", head == NULL);

    // double free safe
    free_list(&head);
    CHECK("double free safe", head == NULL);

    // free mixed list
    insertIntNode(&head, 1);
    insertStringNode(&head, "two");
    insertIntNode(&head, 3);
    free_list(&head);
    CHECK("free mixed list", head == NULL);
}

/* ═════════════════════════════════════════
   MAIN
   ═════════════════════════════════════════ */
int main() {
    printf("=== tagged union list tests ===\n");

    test_insertIntNode();
    test_insertStringNode();
    test_mixed_list();
    test_deleteIntNode();
    test_deleteIntNode_mixed();
    test_deleteStringNode();
    test_deleteStringNode_mixed();
    test_searchInt();
    test_searchString();
    test_stress_insert_delete();
    test_stress_mixed();
    test_free_list();

    printf("\ndone\n");
    return 0;
}