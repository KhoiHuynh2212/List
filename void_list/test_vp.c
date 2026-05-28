#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list_vp.h"

/* ─────────────────────────────────────────
   CHECK MACRO
   ───────────────────────────────────────── */
#define CHECK(name, condition) do { \
    if (condition) \
        printf("PASS %s\n", name); \
    else \
        printf("FAIL %s\n", name); \
} while(0)

/* ─────────────────────────────────────────
   COMPARATORS
   ───────────────────────────────────────── */
int cmp_int(void *a, void *b)    { return *(int*)a == *(int*)b; }
int cmp_string(void *a, void *b) { return strcmp((char*)a, (char*)b) == 0; }

/* ─────────────────────────────────────────
   INSERT END
   ───────────────────────────────────────── */
void test_insertEnd() {
    printf("\n--- insertEnd ---\n");
    Node *head = NULL;

    // happy path: insert into empty list
    int a = 10;
    insertEnd(&head, &a, sizeof(int));
    CHECK("insert into empty list", head != NULL);
    CHECK("head data correct", *(int*)head->data == 10);
    CHECK("head->prev is NULL", head->prev == NULL);
    CHECK("head->next is NULL", head->next == NULL);

    // happy path: insert second node
    int b = 20;
    insertEnd(&head, &b, sizeof(int));
    CHECK("second node appended", head->next != NULL);
    CHECK("second node data correct", *(int*)head->next->data == 20);
    CHECK("second node prev points to head", head->next->prev == head);
    CHECK("second node next is NULL", head->next->next == NULL);

    // happy path: insert third node
    int c = 30;
    insertEnd(&head, &c, sizeof(int));
    CHECK("third node data correct", *(int*)head->next->next->data == 30);

    // after-effect: original value change doesn't affect list (deep copy)
    int x = 99;
    insertEnd(&head, &x, sizeof(int));
    x = 0;
    CHECK("deep copy: mutation doesn't affect node", *(int*)head->next->next->next->data == 99);

    free_list(&head);
}

/* ─────────────────────────────────────────
   INSERT HEAD
   ───────────────────────────────────────── */
void test_insertHead() {
    printf("\n--- insertHead ---\n");
    Node *head = NULL;

    // happy path: insert into empty list
    int a = 10;
    insertHead(&head, &a, sizeof(int));
    CHECK("insert into empty list", head != NULL);
    CHECK("head data correct", *(int*)head->data == 10);
    CHECK("head->prev is NULL", head->prev == NULL);

    // happy path: insert second at head
    int b = 20;
    insertHead(&head, &b, sizeof(int));
    CHECK("new head data correct", *(int*)head->data == 20);
    CHECK("new head->next data correct", *(int*)head->next->data == 10);
    CHECK("new head->prev is NULL", head->prev == NULL);
    CHECK("old head->prev points to new head", head->next->prev == head);

    // happy path: insert third at head
    int c = 30;
    insertHead(&head, &c, sizeof(int));
    CHECK("third head data correct", *(int*)head->data == 30);

    free_list(&head);
}

/* ─────────────────────────────────────────
   DELETE
   ───────────────────────────────────────── */
void test_deleteNode() {
    printf("\n--- deleteNode ---\n");
    Node *head = NULL;
    int a = 10, b = 20, c = 30;
    insertEnd(&head, &a, sizeof(int));
    insertEnd(&head, &b, sizeof(int));
    insertEnd(&head, &c, sizeof(int));

    // miss case
    int miss = 99;
    CHECK("returns -1 on miss", deleteNode(&head, &miss, cmp_int) == -1);

    // delete middle
    CHECK("returns 0 on hit", deleteNode(&head, &b, cmp_int) == 0);
    CHECK("middle deleted: head->next is 30", *(int*)head->next->data == 30);
    CHECK("middle deleted: next->prev points to head", head->next->prev == head);

    // delete head
    deleteNode(&head, &a, cmp_int);
    CHECK("head deleted: new head is 30", *(int*)head->data == 30);
    CHECK("new head->prev is NULL", head->prev == NULL);

    // delete tail (only node left)
    deleteNode(&head, &c, cmp_int);
    CHECK("tail deleted: list is empty", head == NULL);

    // miss on empty list
    CHECK("returns -1 on empty list", deleteNode(&head, &a, cmp_int) == -1);

    free_list(&head);
}

void test_deleteNode_string() {
    printf("\n--- deleteNode string ---\n");
    Node *head = NULL;
    insertEnd(&head, "apple",  strlen("apple")  + 1);
    insertEnd(&head, "banana", strlen("banana") + 1);
    insertEnd(&head, "cherry", strlen("cherry") + 1);

    CHECK("delete middle string", deleteNode(&head, "banana", cmp_string) == 0);
    CHECK("miss string", deleteNode(&head, "mango", cmp_string) == -1);
    CHECK("delete head string", deleteNode(&head, "apple", cmp_string) == 0);
    CHECK("new head is cherry", strcmp((char*)head->data, "cherry") == 0);

    free_list(&head);
}

/* ─────────────────────────────────────────
   SEARCH
   ───────────────────────────────────────── */
void test_searchNode() {
    printf("\n--- searchNode ---\n");
    Node *head = NULL;
    int a = 10, b = 20, c = 30;
    insertEnd(&head, &a, sizeof(int));
    insertEnd(&head, &b, sizeof(int));
    insertEnd(&head, &c, sizeof(int));

    // happy path
    Node *found = searchNode(head, &b, cmp_int);
    CHECK("search hit returns non-NULL", found != NULL);
    CHECK("search hit returns correct node", *(int*)found->data == 20);

    // search head
    Node *foundHead = searchNode(head, &a, cmp_int);
    CHECK("search head", *(int*)foundHead->data == 10);

    // search tail
    Node *foundTail = searchNode(head, &c, cmp_int);
    CHECK("search tail", *(int*)foundTail->data == 30);

    // miss case
    int miss = 99;
    CHECK("search miss returns NULL", searchNode(head, &miss, cmp_int) == NULL);

    // empty list
    Node *empty = NULL;
    CHECK("search empty list returns NULL", searchNode(empty, &a, cmp_int) == NULL);

    free_list(&head);
}

/* ─────────────────────────────────────────
   REVERSE
   ───────────────────────────────────────── */
void test_reverse() {
    printf("\n--- reverse ---\n");

    // single node
    Node *head = NULL;
    int a = 10;
    insertEnd(&head, &a, sizeof(int));
    reverse(&head);
    CHECK("single node reverse: head unchanged", *(int*)head->data == 10);
    free_list(&head);

    // two nodes
    head = NULL;
    int x = 1, y = 2;
    insertEnd(&head, &x, sizeof(int));
    insertEnd(&head, &y, sizeof(int));
    reverse(&head);
    CHECK("two nodes: new head is 2", *(int*)head->data == 2);
    CHECK("two nodes: second is 1", *(int*)head->next->data == 1);
    CHECK("two nodes: head->prev is NULL", head->prev == NULL);
    CHECK("two nodes: tail->next is NULL", head->next->next == NULL);
    free_list(&head);

    // three nodes
    head = NULL;
    int a2 = 10, b2 = 20, c2 = 30;
    insertEnd(&head, &a2, sizeof(int));
    insertEnd(&head, &b2, sizeof(int));
    insertEnd(&head, &c2, sizeof(int));
    reverse(&head);
    CHECK("three nodes: new head is 30", *(int*)head->data == 30);
    CHECK("three nodes: middle is 20", *(int*)head->next->data == 20);
    CHECK("three nodes: tail is 10", *(int*)head->next->next->data == 10);
    CHECK("three nodes: head->prev NULL", head->prev == NULL);
    CHECK("three nodes: tail->next NULL", head->next->next->next == NULL);
    CHECK("three nodes: prev links correct", head->next->prev == head);
    free_list(&head);

    // empty list
    head = NULL;
    reverse(&head);
    CHECK("empty list reverse: head still NULL", head == NULL);
}

/* ─────────────────────────────────────────
   COUNT
   ───────────────────────────────────────── */
void test_count() {
    printf("\n--- count ---\n");
    Node *head = NULL;

    CHECK("empty list count is 0", count(head) == 0);

    int a = 1, b = 2, c = 3;
    insertEnd(&head, &a, sizeof(int));
    CHECK("count after 1 insert", count(head) == 1);

    insertEnd(&head, &b, sizeof(int));
    insertEnd(&head, &c, sizeof(int));
    CHECK("count after 3 inserts", count(head) == 3);

    deleteNode(&head, &b, cmp_int);
    CHECK("count after delete", count(head) == 2);

    free_list(&head);
    CHECK("count after free", count(head) == 0);
}

/* ─────────────────────────────────────────
   FREE LIST
   ───────────────────────────────────────── */
void test_free_list() {
    printf("\n--- free_list ---\n");
    Node *head = NULL;
    int a = 1, b = 2;
    insertEnd(&head, &a, sizeof(int));
    insertEnd(&head, &b, sizeof(int));

    free_list(&head);
    CHECK("head is NULL after free", head == NULL);

    // double free should be safe
    free_list(&head);
    CHECK("double free safe", head == NULL);
}

/* ─────────────────────────────────────────
   MAIN
   ───────────────────────────────────────── */
int main() {
    printf("=== void* doubly linked list tests ===\n");
    test_insertEnd();
    test_insertHead();
    test_deleteNode();
    test_deleteNode_string();
    test_searchNode();
    test_reverse();
    test_count();
    test_free_list();
    printf("\ndone\n");
    return 0;
}