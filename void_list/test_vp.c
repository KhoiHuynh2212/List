
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list_vp.h"

/* ── CHECK macro ─────────────────────────────────────────────────────── */
static int g_pass = 0, g_fail = 0;

#define CHECK(name, cond) do {                              \
    if (cond) {                                             \
        printf("  PASS  %s\n", name);                       \
        g_pass++;                                           \
    } else {                                                \
        printf("  FAIL  %s  [line %d]\n", name, __LINE__); \
        g_fail++;                                           \
    }                                                       \
} while (0)

/* ── comparators ─────────────────────────────────────────────────────── */
/* non-zero = MATCH */
static int cmp_int   (void *a, void *b) { return *(int*)a == *(int*)b; }
static int cmp_string(void *a, void *b) { return strcmp((char*)a, (char*)b) == 0; }

/* ── ring integrity helpers ──────────────────────────────────────────── */
static int ring_len_fwd(Node *head, int cap) {
    if (!head) return -1;
    int n = 0;
    Node *c = head;
    do { c = c->next; if (++n > cap) return -2; } while (c != head);
    return n;
}
static int ring_len_bwd(Node *head, int cap) {
    if (!head) return -1;
    int n = 0;
    Node *c = head;
    do { c = c->prev; if (++n > cap) return -2; } while (c != head);
    return n;
}
static int ring_links_ok(Node *head) {
    if (!head) return 1;
    Node *c = head;
    do {
        if (c->next->prev != c) return 0;
        if (c->prev->next != c) return 0;
        c = c->next;
    } while (c != head);
    return 1;
}
static Node *tail(Node *head) { return head ? head->prev : NULL; }

/* ═══════════════════════════════════════════════════════════════════════
   SECTION 1 — Ring integrity
   ═══════════════════════════════════════════════════════════════════════ */

void test_ring_single(void) {
    printf("\n[ring] single node\n");
    Node *head = NULL;
    int v = 42;
    insertEnd(&head, &v, sizeof(int));

    CHECK("head not NULL",           head != NULL);
    CHECK("head->next == head",      head->next == head);
    CHECK("head->prev == head",      head->prev == head);
    CHECK("forward length == 1",     ring_len_fwd(head, 10) == 1);
    CHECK("backward length == 1",    ring_len_bwd(head, 10) == 1);
    CHECK("bidirectional links ok",  ring_links_ok(head));

    free_list(&head);
    CHECK("head NULL after free",    head == NULL);
}

void test_ring_two_nodes(void) {
    printf("\n[ring] two nodes\n");
    Node *head = NULL;
    int a = 1, b = 2;
    insertEnd(&head, &a, sizeof(int));
    insertEnd(&head, &b, sizeof(int));

    Node *t = tail(head);
    CHECK("forward length == 2",     ring_len_fwd(head, 10) == 2);
    CHECK("backward length == 2",    ring_len_bwd(head, 10) == 2);
    CHECK("head->next is tail",      head->next == t);
    CHECK("tail->next is head",      t->next == head);
    CHECK("head->prev is tail",      head->prev == t);
    CHECK("tail->prev is head",      t->prev == head);
    CHECK("bidirectional links ok",  ring_links_ok(head));

    free_list(&head);
}

void test_ring_many(void) {
    printf("\n[ring] 50-node ring\n");
    Node *head = NULL;
    for (int i = 0; i < 50; i++) insertEnd(&head, &i, sizeof(int));

    CHECK("forward length == 50",    ring_len_fwd(head, 200) == 50);
    CHECK("backward length == 50",   ring_len_bwd(head, 200) == 50);
    CHECK("tail->next == head",      tail(head)->next == head);
    CHECK("head->prev == tail",      head->prev == tail(head));
    CHECK("all bidirectional links", ring_links_ok(head));

    free_list(&head);
}

void test_ring_insertHead(void) {
    printf("\n[ring] insertHead ring integrity\n");
    Node *head = NULL;
    int a = 1, b = 2, c = 3;
    insertHead(&head, &a, sizeof(int));
    insertHead(&head, &b, sizeof(int));
    insertHead(&head, &c, sizeof(int));   /* head should be 3->2->1-> (ring) */

    CHECK("forward length == 3",     ring_len_fwd(head, 20) == 3);
    CHECK("backward length == 3",    ring_len_bwd(head, 20) == 3);
    CHECK("head data == 3",          *(int*)head->data == 3);
    CHECK("tail data == 1",          *(int*)tail(head)->data == 1);
    CHECK("tail->next == head",      tail(head)->next == head);
    CHECK("bidirectional links ok",  ring_links_ok(head));

    free_list(&head);
}

/* ═══════════════════════════════════════════════════════════════════════
   SECTION 2 — Node invariants
   ═══════════════════════════════════════════════════════════════════════ */

void test_invariants_int(void) {
    printf("\n[invariants] int deep copy\n");
    Node *head = NULL;
    int v = 100;
    insertEnd(&head, &v, sizeof(int));
    v = 0;                                 /* mutate source */

    CHECK("data unaffected by source mutation", *(int*)head->data == 100);
    CHECK("node ptr differs from stack var",    head->data != &v);
    CHECK("size field correct",                 head->size == sizeof(int));

    free_list(&head);
}

void test_invariants_string(void) {
    printf("\n[invariants] string deep copy\n");
    Node *head = NULL;
    char src[] = "mutable";
    insertEnd(&head, src, strlen(src) + 1);
    src[0] = 'X';                          /* mutate source */

    CHECK("string unaffected by src mutation",
          strcmp((char*)head->data, "mutable") == 0);
    CHECK("node stores own copy, not src ptr",  head->data != src);
    CHECK("size field == strlen+1",             head->size == strlen("mutable") + 1);

    free_list(&head);
}

void test_invariants_order(void) {
    printf("\n[invariants] insertion order preserved\n");
    Node *head = NULL;
    int vals[] = {10, 20, 30, 40};
    for (int i = 0; i < 4; i++) insertEnd(&head, &vals[i], sizeof(int));

    Node *cur = head;
    CHECK("node 0 == 10", *(int*)cur->data == 10); cur = cur->next;
    CHECK("node 1 == 20", *(int*)cur->data == 20); cur = cur->next;
    CHECK("node 2 == 30", *(int*)cur->data == 30); cur = cur->next;
    CHECK("node 3 == 40", *(int*)cur->data == 40);

    free_list(&head);
}

/* ═══════════════════════════════════════════════════════════════════════
   SECTION 3 — Edge cases
   ═══════════════════════════════════════════════════════════════════════ */

void test_edge_free_null(void) {
    printf("\n[edge] free_list on NULL\n");
    Node *head = NULL;
    free_list(&head);
    CHECK("head still NULL",     head == NULL);
    free_list(&head);            /* double free — must not crash */
    CHECK("double free safe",    head == NULL);
    free_list(NULL);             /* NULL pointer to head — must not crash */
    CHECK("free_list(NULL) safe", 1);
}

void test_edge_delete_empty(void) {
    printf("\n[edge] delete / search on empty list\n");
    Node *head = NULL;
    int v = 5;
    CHECK("deleteNode empty -> -1",  deleteNode(&head, &v, cmp_int) == -1);
    CHECK("searchNode empty -> NULL", searchNode(head, &v, cmp_int) == NULL);
    CHECK("count empty == 0",         count(head) == 0);
}

void test_edge_null_args(void) {
    printf("\n[edge] NULL argument guards\n");
    Node *head = NULL;
    int v = 1;
    insertEnd(&head, &v, sizeof(int));

    CHECK("deleteNode NULL head -> -1",  deleteNode(NULL, &v, cmp_int) == -1);
    CHECK("deleteNode NULL key -> -1",   deleteNode(&head, NULL, cmp_int) == -1);
    CHECK("deleteNode NULL cmp -> -1",   deleteNode(&head, &v, NULL) == -1);
    CHECK("searchNode NULL head -> NULL",searchNode(NULL, &v, cmp_int) == NULL);
    CHECK("searchNode NULL data -> NULL",searchNode(head, NULL, cmp_int) == NULL);
    CHECK("searchNode NULL cmp -> NULL", searchNode(head, &v, NULL) == NULL);

    free_list(&head);
}

void test_edge_delete_only_node(void) {
    printf("\n[edge] delete only node -> empty list\n");
    Node *head = NULL;
    int v = 7;
    insertEnd(&head, &v, sizeof(int));

    CHECK("delete returns 0",     deleteNode(&head, &v, cmp_int) == 0);
    CHECK("head is NULL after",   head == NULL);
}

void test_edge_delete_head(void) {
    printf("\n[edge] delete head — ring stays valid\n");
    Node *head = NULL;
    int a = 10, b = 20, c = 30;
    insertEnd(&head, &a, sizeof(int));
    insertEnd(&head, &b, sizeof(int));
    insertEnd(&head, &c, sizeof(int));

    CHECK("delete head returns 0",    deleteNode(&head, &a, cmp_int) == 0);
    CHECK("new head data == 20",      *(int*)head->data == 20);
    CHECK("forward length == 2",      ring_len_fwd(head, 20) == 2);
    CHECK("backward length == 2",     ring_len_bwd(head, 20) == 2);
    CHECK("bidirectional links ok",   ring_links_ok(head));
    CHECK("tail->next == head",       tail(head)->next == head);

    free_list(&head);
}

void test_edge_delete_tail(void) {
    printf("\n[edge] delete tail — ring stays valid\n");
    Node *head = NULL;
    int a = 10, b = 20, c = 30;
    insertEnd(&head, &a, sizeof(int));
    insertEnd(&head, &b, sizeof(int));
    insertEnd(&head, &c, sizeof(int));

    CHECK("delete tail returns 0",    deleteNode(&head, &c, cmp_int) == 0);
    CHECK("head data unchanged",      *(int*)head->data == 10);
    CHECK("forward length == 2",      ring_len_fwd(head, 20) == 2);
    CHECK("backward length == 2",     ring_len_bwd(head, 20) == 2);
    CHECK("bidirectional links ok",   ring_links_ok(head));
    CHECK("new tail data == 20",      *(int*)tail(head)->data == 20);

    free_list(&head);
}

void test_edge_delete_middle(void) {
    printf("\n[edge] delete middle — ring stays valid\n");
    Node *head = NULL;
    int a = 1, b = 2, c = 3;
    insertEnd(&head, &a, sizeof(int));
    insertEnd(&head, &b, sizeof(int));
    insertEnd(&head, &c, sizeof(int));

    CHECK("delete middle returns 0",  deleteNode(&head, &b, cmp_int) == 0);
    CHECK("forward length == 2",      ring_len_fwd(head, 20) == 2);
    CHECK("backward length == 2",     ring_len_bwd(head, 20) == 2);
    CHECK("bidirectional links ok",   ring_links_ok(head));
    CHECK("head->next->data == 3",    *(int*)head->next->data == 3);
    CHECK("3->prev == head",          head->next->prev == head);

    free_list(&head);
}

void test_edge_delete_miss(void) {
    printf("\n[edge] delete miss — list unchanged\n");
    Node *head = NULL;
    int a = 1, b = 2;
    insertEnd(&head, &a, sizeof(int));
    insertEnd(&head, &b, sizeof(int));

    int miss = 99;
    CHECK("delete miss returns -1",   deleteNode(&head, &miss, cmp_int) == -1);
    CHECK("forward length still 2",   ring_len_fwd(head, 20) == 2);
    CHECK("links still ok",           ring_links_ok(head));

    free_list(&head);
}

void test_edge_insertMid(void) {
    printf("\n[edge] list_insertMid\n");
    Node *head = NULL;
    int a = 1, b = 3;
    insertEnd(&head, &a, sizeof(int));
    insertEnd(&head, &b, sizeof(int));

    int mid = 2;
    list_insertMid(head, &mid, sizeof(int));   /* insert after head: 1->2->3 */

    CHECK("forward length == 3",      ring_len_fwd(head, 20) == 3);
    CHECK("backward length == 3",     ring_len_bwd(head, 20) == 3);
    CHECK("bidirectional links ok",   ring_links_ok(head));
    CHECK("mid node data == 2",       *(int*)head->next->data == 2);
    CHECK("mid->next data == 3",      *(int*)head->next->next->data == 3);

    free_list(&head);
}

/* ═══════════════════════════════════════════════════════════════════════
   SECTION 4 — Traverse & match
   ═══════════════════════════════════════════════════════════════════════ */

void test_search_int(void) {
    printf("\n[traverse] searchNode int\n");
    Node *head = NULL;
    int a = 10, b = 20, c = 30;
    insertEnd(&head, &a, sizeof(int));
    insertEnd(&head, &b, sizeof(int));
    insertEnd(&head, &c, sizeof(int));

    Node *found = searchNode(head, &b, cmp_int);
    CHECK("hit: non-NULL",           found != NULL);
    CHECK("hit: correct data",       *(int*)found->data == 20);
    CHECK("hit head",                searchNode(head, &a, cmp_int) == head);
    CHECK("hit tail",                searchNode(head, &c, cmp_int) == tail(head));

    int miss = 99;
    CHECK("miss: NULL",              searchNode(head, &miss, cmp_int) == NULL);

    free_list(&head);
}

void test_search_string(void) {
    printf("\n[traverse] searchNode string\n");
    Node *head = NULL;
    insertEnd(&head, "alpha",  6);
    insertEnd(&head, "beta",   5);
    insertEnd(&head, "gamma",  6);

    Node *found = searchNode(head, "beta", cmp_string);
    CHECK("hit: non-NULL",           found != NULL);
    CHECK("hit: correct data",       strcmp((char*)found->data, "beta") == 0);
    CHECK("hit head",                searchNode(head, "alpha", cmp_string) == head);
    CHECK("hit tail",                searchNode(head, "gamma", cmp_string) == tail(head));
    CHECK("miss: NULL",              searchNode(head, "delta", cmp_string) == NULL);

    free_list(&head);
}

void test_delete_string(void) {
    printf("\n[traverse] deleteNode string\n");
    Node *head = NULL;
    insertEnd(&head, "apple",  6);
    insertEnd(&head, "banana", 7);
    insertEnd(&head, "cherry", 7);

    CHECK("delete middle -> 0",      deleteNode(&head, "banana", cmp_string) == 0);
    CHECK("ring length == 2",        ring_len_fwd(head, 20) == 2);
    CHECK("links ok",                ring_links_ok(head));
    CHECK("miss -> -1",              deleteNode(&head, "mango", cmp_string) == -1);
    CHECK("delete head -> 0",        deleteNode(&head, "apple", cmp_string) == 0);
    CHECK("new head is cherry",      strcmp((char*)head->data, "cherry") == 0);

    free_list(&head);
}

void test_search_after_delete(void) {
    printf("\n[traverse] search after delete\n");
    Node *head = NULL;
    int a = 5, b = 10, c = 15;
    insertEnd(&head, &a, sizeof(int));
    insertEnd(&head, &b, sizeof(int));
    insertEnd(&head, &c, sizeof(int));

    deleteNode(&head, &b, cmp_int);
    CHECK("deleted node not found",  searchNode(head, &b, cmp_int) == NULL);
    CHECK("other nodes still found", searchNode(head, &a, cmp_int) != NULL);
    CHECK("other nodes still found", searchNode(head, &c, cmp_int) != NULL);

    free_list(&head);
}

void test_count(void) {
    printf("\n[traverse] count\n");
    Node *head = NULL;
    CHECK("empty == 0",   count(head) == 0);

    int a = 1, b = 2, c = 3;
    insertEnd(&head, &a, sizeof(int));
    CHECK("after 1 insert", count(head) == 1);
    insertEnd(&head, &b, sizeof(int));
    insertEnd(&head, &c, sizeof(int));
    CHECK("after 3 inserts", count(head) == 3);

    deleteNode(&head, &b, cmp_int);
    CHECK("after delete", count(head) == 2);

    free_list(&head);
    CHECK("after free", count(head) == 0);
}

void test_reverse(void) {
    printf("\n[traverse] reverse\n");

    /* single node — noop */
    Node *head = NULL;
    int a = 42;
    insertEnd(&head, &a, sizeof(int));
    reverse(&head);
    CHECK("single: data unchanged",      *(int*)head->data == 42);
    CHECK("single: head->next == head",  head->next == head);
    CHECK("single: head->prev == head",  head->prev == head);
    free_list(&head);

    /* three nodes */
    head = NULL;
    int v1 = 1, v2 = 2, v3 = 3;
    insertEnd(&head, &v1, sizeof(int));
    insertEnd(&head, &v2, sizeof(int));
    insertEnd(&head, &v3, sizeof(int));
    reverse(&head);

    CHECK("3-node: forward length == 3",    ring_len_fwd(head, 20) == 3);
    CHECK("3-node: backward length == 3",   ring_len_bwd(head, 20) == 3);
    CHECK("3-node: bidirectional links ok", ring_links_ok(head));
    CHECK("3-node: new head == 3",          *(int*)head->data == 3);
    CHECK("3-node: middle == 2",            *(int*)head->next->data == 2);
    CHECK("3-node: tail == 1",              *(int*)tail(head)->data == 1);
    CHECK("3-node: tail->next == head",     tail(head)->next == head);

    /* double reverse restores order */
    reverse(&head);
    CHECK("double-reverse: head == 1",      *(int*)head->data == 1);
    CHECK("double-reverse: tail == 3",      *(int*)tail(head)->data == 3);
    CHECK("double-reverse: links ok",       ring_links_ok(head));

    free_list(&head);

    /* empty list */
    head = NULL;
    reverse(&head);
    CHECK("empty: head still NULL",  head == NULL);
}

/* ═══════════════════════════════════════════════════════════════════════
   SECTION 5 — Stress
   ═══════════════════════════════════════════════════════════════════════ */

void test_stress(void) {
    printf("\n[stress] 1000-node ring — mass delete\n");
    Node *head = NULL;
    for (int i = 0; i < 1000; i++) insertEnd(&head, &i, sizeof(int));

    CHECK("1000-node forward ring",  ring_len_fwd(head, 2000) == 1000);
    CHECK("1000-node backward ring", ring_len_bwd(head, 2000) == 1000);
    CHECK("all links ok before del", ring_links_ok(head));

    /* delete all even values */
    for (int i = 0; i < 1000; i += 2) deleteNode(&head, &i, cmp_int);

    CHECK("500 remain (forward)",    ring_len_fwd(head, 2000) == 500);
    CHECK("500 remain (backward)",   ring_len_bwd(head, 2000) == 500);
    CHECK("all links ok after del",  ring_links_ok(head));
    CHECK("tail->next == head",      tail(head)->next == head);

    int ok = 1;
    Node *cur = head;
    do {
        if (*(int*)cur->data % 2 == 0) { ok = 0; break; }
        cur = cur->next;
    } while (cur != head);
    CHECK("only odd values remain",  ok);

    free_list(&head);
    CHECK("head NULL after stress free", head == NULL);
}

/* ═══════════════════════════════════════════════════════════════════════
   MAIN
   ═══════════════════════════════════════════════════════════════════════ */
int main(void) {
    printf("=== void* circular list — test suite ===\n");

    test_ring_single();
    test_ring_two_nodes();
    test_ring_many();
    test_ring_insertHead();

    test_invariants_int();
    test_invariants_string();
    test_invariants_order();

    test_edge_free_null();
    test_edge_delete_empty();
    test_edge_null_args();
    test_edge_delete_only_node();
    test_edge_delete_head();
    test_edge_delete_tail();
    test_edge_delete_middle();
    test_edge_delete_miss();
    test_edge_insertMid();

    test_search_int();
    test_search_string();
    test_delete_string();
    test_search_after_delete();
    test_count();
    test_reverse();

    test_stress();

    printf("\n═══════════════════════════════\n");
    printf("  PASSED: %d\n", g_pass);
    printf("  FAILED: %d\n", g_fail);
    printf("═══════════════════════════════\n");
    return g_fail > 0 ? 1 : 0;
}