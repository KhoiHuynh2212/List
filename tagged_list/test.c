
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

/* ── CHECK macro ─────────────────────────────────────────────────────── */
static int g_pass = 0, g_fail = 0;

#define CHECK(name, cond) do {                          \
    if (cond) {                                         \
        printf("  PASS  %s\n", name);                   \
        g_pass++;                                       \
    } else {                                            \
        printf("  FAIL  %s  [line %d]\n", name, __LINE__); \
        g_fail++;                                       \
    }                                                   \
} while (0)

/* ── Ring integrity helpers ──────────────────────────────────────────── */

/*
 * ring_len_forward: count nodes by walking ->next until we return to head.
 * Returns -1 if head is NULL.
 * Caps at (max_steps) to avoid hanging on a broken ring — any real list
 * in these tests is << 10 000 nodes.
 */
static int ring_len_forward(Node *head, int max_steps) {
    if (head == NULL) return -1;
    int n = 0;
    Node *cur = head;
    do {
        cur = cur->next;
        n++;
        if (n > max_steps) return -2; /* broken ring */
    } while (cur != head);
    return n;
}

/* Same walk via ->prev. */
static int ring_len_backward(Node *head, int max_steps) {
    if (head == NULL) return -1;
    int n = 0;
    Node *cur = head;
    do {
        cur = cur->prev;
        n++;
        if (n > max_steps) return -2;
    } while (cur != head);
    return n;
}

/*
 * ring_links_ok: verify X->next->prev == X for every node.
 * Returns 1 if all good, 0 on first violation.
 */
static int ring_links_ok(Node *head) {
    if (head == NULL) return 1;
    Node *cur = head;
    do {
        if (cur->next->prev != cur) return 0;
        if (cur->prev->next != cur) return 0;
        cur = cur->next;
    } while (cur != head);
    return 1;
}

/* Tail of a circular list == head->prev. */
static Node *tail(Node *head) {
    return (head == NULL) ? NULL : head->prev;
}

/* ═══════════════════════════════════════════════════════════════════════
   SECTION 1 — Structural invariants after insert
   ═══════════════════════════════════════════════════════════════════════ */

void test_ring_single_node() {
    printf("\n[ring] single node\n");
    Node *head = NULL;
    insertIntNode(&head, 42);

    CHECK("head not NULL",          head != NULL);
    CHECK("head->next == head",     head->next == head);
    CHECK("head->prev == head",     head->prev == head);
    CHECK("forward length == 1",    ring_len_forward(head, 10) == 1);
    CHECK("backward length == 1",   ring_len_backward(head, 10) == 1);
    CHECK("bidirectional links ok", ring_links_ok(head));

    free_list(&head);
    CHECK("head NULL after free",   head == NULL);
}

void test_ring_two_nodes() {
    printf("\n[ring] two nodes\n");
    Node *head = NULL;
    insertIntNode(&head, 1);
    insertIntNode(&head, 2);

    Node *t = tail(head);
    CHECK("forward length == 2",    ring_len_forward(head, 10) == 2);
    CHECK("backward length == 2",   ring_len_backward(head, 10) == 2);
    CHECK("head->next is tail",     head->next == t);
    CHECK("tail->next is head",     t->next == head);
    CHECK("head->prev is tail",     head->prev == t);
    CHECK("tail->prev is head",     t->prev == head);
    CHECK("bidirectional links ok", ring_links_ok(head));

    free_list(&head);
}

void test_ring_many_nodes() {
    printf("\n[ring] 50-node ring integrity\n");
    Node *head = NULL;
    for (int i = 0; i < 50; i++)
        insertIntNode(&head, i);

    CHECK("forward length == 50",   ring_len_forward(head, 200) == 50);
    CHECK("backward length == 50",  ring_len_backward(head, 200) == 50);
    CHECK("tail->next == head",     tail(head)->next == head);
    CHECK("head->prev == tail",     head->prev == tail(head));
    CHECK("all bidirectional links",ring_links_ok(head));

    free_list(&head);
}

void test_ring_mixed_types() {
    printf("\n[ring] ring integrity across mixed types\n");
    Node *head = NULL;
    insertIntNode(&head, 1);
    insertStringNode(&head, "two");
    insertFloatNode(&head, 3.0f);
    insertDoubleNode(&head, 4.0);
    insertStringNode(&head, "five");

    CHECK("forward length == 5",    ring_len_forward(head, 20) == 5);
    CHECK("backward length == 5",   ring_len_backward(head, 20) == 5);
    CHECK("tail->next == head",     tail(head)->next == head);
    CHECK("all bidirectional links",ring_links_ok(head));

    free_list(&head);
}

/* ═══════════════════════════════════════════════════════════════════════
   SECTION 2 — Node invariants (kind + data correctness)
   ═══════════════════════════════════════════════════════════════════════ */

void test_node_invariants_int() {
    printf("\n[invariants] insertIntNode\n");
    Node *head = NULL;
    insertIntNode(&head, 100);

    CHECK("kind == INTEGER",        head->kind == INTEGER);
    CHECK("data == 100",            head->data.node_int == 100);

    /* deep-copy semantics: int is passed by value, always safe */
    int x = 999;
    insertIntNode(&head, x);
    x = 0;
    CHECK("int value independent of source var", head->next->data.node_int == 999);

    free_list(&head);
}

void test_node_invariants_string() {
    printf("\n[invariants] insertStringNode deep copy\n");
    Node *head = NULL;
    char src[] = "mutable";
    insertStringNode(&head, src);

    /* mutate source — node must still hold original */
    src[0] = 'X';
    CHECK("kind == STRING",         head->kind == STRING);
    CHECK("deep copy: node unaffected by src mutation",
          strcmp(head->data.node_string, "mutable") == 0);

    /* pointer inside node must differ from src */
    CHECK("node stores its own copy, not src ptr",
          head->data.node_string != src);

    free_list(&head);
}

void test_node_invariants_float() {
    printf("\n[invariants] insertFloatNode\n");
    Node *head = NULL;
    insertFloatNode(&head, 1.5f);

    CHECK("kind == FLOAT",          head->kind == FLOAT);
    CHECK("data == 1.5f",           head->data.node_float == 1.5f);
    CHECK("ring self-pointer",      head->next == head && head->prev == head);

    free_list(&head);
}

void test_node_invariants_double() {
    printf("\n[invariants] insertDoubleNode\n");
    Node *head = NULL;
    insertDoubleNode(&head, 3.141592653589793);

    CHECK("kind == DOUBLE",         head->kind == DOUBLE);
    CHECK("data correct",           head->data.node_double == 3.141592653589793);
    CHECK("ring self-pointer",      head->next == head && head->prev == head);

    free_list(&head);
}

void test_node_invariants_empty_string() {
    printf("\n[invariants] empty string node\n");
    Node *head = NULL;
    insertStringNode(&head, "");

    CHECK("kind == STRING",         head->kind == STRING);
    CHECK("data is empty string",   strcmp(head->data.node_string, "") == 0);
    CHECK("ring self-pointer",      head->next == head);

    free_list(&head);
}

/* ═══════════════════════════════════════════════════════════════════════
   SECTION 3 — Edge cases and boundary states
   ═══════════════════════════════════════════════════════════════════════ */

void test_free_null_list() {
    printf("\n[edge] free_list on NULL head\n");
    Node *head = NULL;
    free_list(&head); /* must not crash */
    CHECK("head still NULL", head == NULL);
    free_list(&head); /* double-free of NULL — must not crash */
    CHECK("double free safe", head == NULL);
}

void test_delete_from_empty() {
    printf("\n[edge] delete from empty list\n");
    Node *head = NULL;
    CHECK("deleteIntNode empty -> -1",    deleteIntNode(&head, 5)    == -1);
    CHECK("deleteStringNode empty -> -1", deleteStringNode(&head, "x") == -1);
    CHECK("deleteFloatNode empty -> -1",  deleteFloatNode(&head, 1.0f) == -1);
    CHECK("deleteDoubleNode empty -> -1", deleteDoubleNode(&head, 1.0) == -1);
}

void test_delete_only_node() {
    printf("\n[edge] delete only node -> empty list\n");
    Node *head = NULL;

    insertIntNode(&head, 7);
    CHECK("int delete returns 0",   deleteIntNode(&head, 7) == 0);
    CHECK("head is NULL after",     head == NULL);

    insertStringNode(&head, "solo");
    CHECK("string delete returns 0",deleteStringNode(&head, "solo") == 0);
    CHECK("head is NULL after",     head == NULL);

    insertFloatNode(&head, 2.0f);
    CHECK("float delete returns 0", deleteFloatNode(&head, 2.0f) == 0);
    CHECK("head is NULL after",     head == NULL);

    insertDoubleNode(&head, 9.9);
    CHECK("double delete returns 0",deleteDoubleNode(&head, 9.9) == 0);
    CHECK("head is NULL after",     head == NULL);
}

void test_delete_head_ring_fix() {
    printf("\n[edge] delete head — ring must stay valid\n");
    Node *head = NULL;
    insertIntNode(&head, 10);
    insertIntNode(&head, 20);
    insertIntNode(&head, 30);

    deleteIntNode(&head, 10);      /* remove head */

    CHECK("new head data == 20",      head->data.node_int == 20);
    CHECK("forward length still 2",   ring_len_forward(head, 20) == 2);
    CHECK("backward length still 2",  ring_len_backward(head, 20) == 2);
    CHECK("bidirectional links ok",   ring_links_ok(head));
    CHECK("new head->prev is tail",   head->prev == tail(head));
    CHECK("tail->next is new head",   tail(head)->next == head);

    free_list(&head);
}

void test_delete_tail_ring_fix() {
    printf("\n[edge] delete tail — ring must stay valid\n");
    Node *head = NULL;
    insertIntNode(&head, 10);
    insertIntNode(&head, 20);
    insertIntNode(&head, 30);

    deleteIntNode(&head, 30); /* remove tail */

    CHECK("head data unchanged",      head->data.node_int == 10);
    CHECK("forward length == 2",      ring_len_forward(head, 20) == 2);
    CHECK("backward length == 2",     ring_len_backward(head, 20) == 2);
    CHECK("bidirectional links ok",   ring_links_ok(head));
    CHECK("new tail data == 20",      tail(head)->data.node_int == 20);
    CHECK("new tail->next == head",   tail(head)->next == head);

    free_list(&head);
}

void test_delete_middle_ring_fix() {
    printf("\n[edge] delete middle — ring must stay valid\n");
    Node *head = NULL;
    insertIntNode(&head, 1);
    insertIntNode(&head, 2);
    insertIntNode(&head, 3);

    deleteIntNode(&head, 2);

    CHECK("forward length == 2",      ring_len_forward(head, 20) == 2);
    CHECK("backward length == 2",     ring_len_backward(head, 20) == 2);
    CHECK("bidirectional links ok",   ring_links_ok(head));
    CHECK("head->next->data == 3",    head->next->data.node_int == 3);
    CHECK("3->prev == head",          head->next->prev == head);

    free_list(&head);
}

void test_delete_miss() {
    printf("\n[edge] delete miss — list unchanged\n");
    Node *head = NULL;
    insertIntNode(&head, 1);
    insertIntNode(&head, 2);

    int r = deleteIntNode(&head, 99);
    CHECK("returns -1",               r == -1);
    CHECK("forward length still 2",   ring_len_forward(head, 20) == 2);
    CHECK("bidirectional links ok",   ring_links_ok(head));

    free_list(&head);
}

void test_delete_type_safety() {
    printf("\n[edge] delete skips wrong type\n");
    Node *head = NULL;
    insertStringNode(&head, "hello");
    insertIntNode(&head, 42);
    insertStringNode(&head, "world");

    /* deleteIntNode must skip STRING nodes even in a mixed list */
    CHECK("deleteIntNode skips strings, hits int",
          deleteIntNode(&head, 42) == 0);
    CHECK("both string nodes survive",
          ring_len_forward(head, 20) == 2);
    CHECK("head kind is STRING",
          head->kind == STRING);
    CHECK("tail kind is STRING",
          tail(head)->kind == STRING);
    CHECK("ring still valid",         ring_links_ok(head));

    /* deleteStringNode must skip INTEGER nodes */
    insertIntNode(&head, 100);
    CHECK("deleteStringNode skips ints, hits string",
          deleteStringNode(&head, "hello") == 0);
    CHECK("int node survives",
          searchInt(head, 100) != NULL);
    CHECK("ring still valid",         ring_links_ok(head));

    free_list(&head);
}

void test_search_null() {
    printf("\n[edge] search on NULL head\n");
    Node *head = NULL;
    CHECK("searchInt NULL -> NULL",    searchInt(head, 5)    == NULL);
    CHECK("searchString NULL -> NULL", searchString(head, "x") == NULL);
    CHECK("searchFloat NULL -> NULL",  searchFloat(head, 1.0f) == NULL);
    CHECK("searchDouble NULL -> NULL", searchDouble(head, 1.0)  == NULL);
}

/* ═══════════════════════════════════════════════════════════════════════
   SECTION 4 — Traverse & match (standard functional tests)
   ═══════════════════════════════════════════════════════════════════════ */

void test_search_int() {
    printf("\n[traverse] searchInt\n");
    Node *head = NULL;
    insertIntNode(&head, 10);
    insertIntNode(&head, 20);
    insertIntNode(&head, 30);

    Node *found = searchInt(head, 20);
    CHECK("hit: non-NULL",            found != NULL);
    CHECK("hit: correct data",        found->data.node_int == 20);
    CHECK("hit head",                 searchInt(head, 10) == head);
    CHECK("hit tail",                 searchInt(head, 30) == tail(head));
    CHECK("miss: NULL",               searchInt(head, 99) == NULL);

    free_list(&head);
}

void test_search_string() {
    printf("\n[traverse] searchString\n");
    Node *head = NULL;
    insertStringNode(&head, "alpha");
    insertStringNode(&head, "beta");
    insertStringNode(&head, "gamma");

    Node *found = searchString(head, "beta");
    CHECK("hit: non-NULL",            found != NULL);
    CHECK("hit: correct data",        strcmp(found->data.node_string, "beta") == 0);
    CHECK("hit head",                 searchString(head, "alpha") == head);
    CHECK("hit tail",                 searchString(head, "gamma") == tail(head));
    CHECK("miss: NULL",               searchString(head, "delta") == NULL);

    free_list(&head);
}

void test_search_float() {
    printf("\n[traverse] searchFloat\n");
    Node *head = NULL;
    insertFloatNode(&head, 1.0f);
    insertFloatNode(&head, 2.5f);
    insertFloatNode(&head, 3.75f);

    Node *found = searchFloat(head, 2.5f);
    CHECK("hit: non-NULL",            found != NULL);
    CHECK("hit: correct data",        found->data.node_float == 2.5f);
    CHECK("miss: NULL",               searchFloat(head, 9.9f) == NULL);

    free_list(&head);
}

void test_search_double() {
    printf("\n[traverse] searchDouble\n");
    Node *head = NULL;
    insertDoubleNode(&head, 1.1);
    insertDoubleNode(&head, 2.2);
    insertDoubleNode(&head, 3.3);

    Node *found = searchDouble(head, 2.2);
    CHECK("hit: non-NULL",            found != NULL);
    CHECK("hit: correct data",        found->data.node_double == 2.2);
    CHECK("miss: NULL",               searchDouble(head, 9.9) == NULL);

    free_list(&head);
}

void test_search_after_delete() {
    printf("\n[traverse] search after delete\n");
    Node *head = NULL;
    insertIntNode(&head, 5);
    insertIntNode(&head, 10);
    insertIntNode(&head, 15);

    deleteIntNode(&head, 10);
    CHECK("deleted node no longer found", searchInt(head, 10) == NULL);
    CHECK("remaining nodes found",        searchInt(head, 5)  != NULL);
    CHECK("remaining nodes found",        searchInt(head, 15) != NULL);

    free_list(&head);
}

/* ═══════════════════════════════════════════════════════════════════════
   SECTION 5 — Stress test (ring stays intact at scale)
   ═══════════════════════════════════════════════════════════════════════ */

void test_stress_ring() {
    printf("\n[stress] 1 000-node ring integrity after mass delete\n");
    Node *head = NULL;

    for (int i = 0; i < 1000; i++)
        insertIntNode(&head, i);

    CHECK("1000-node forward ring",   ring_len_forward(head, 2000)  == 1000);
    CHECK("1000-node backward ring",  ring_len_backward(head, 2000) == 1000);
    CHECK("all links ok before del",  ring_links_ok(head));

    /* delete all even nodes */
    for (int i = 0; i < 1000; i += 2)
        deleteIntNode(&head, i);

    int fwd = ring_len_forward(head, 2000);
    int bwd = ring_len_backward(head, 2000);
    CHECK("500 nodes remain (forward)",  fwd == 500);
    CHECK("500 nodes remain (backward)", bwd == 500);
    CHECK("all links ok after del",      ring_links_ok(head));
    CHECK("tail->next == head",          tail(head)->next == head);

    /* verify only odds survive */
    int ok = 1;
    Node *cur = head;
    do {
        if (cur->data.node_int % 2 == 0) { ok = 0; break; }
        cur = cur->next;
    } while (cur != head);
    CHECK("only odd values remain", ok);

    free_list(&head);
    CHECK("head NULL after stress free", head == NULL);
}

void test_stress_mixed_ring() {
    printf("\n[stress] 500-node mixed ring, delete all strings\n");
    Node *head = NULL;

    for (int i = 0; i < 250; i++) {
        insertIntNode(&head, i);
        insertStringNode(&head, "tag");
    }

    CHECK("500 nodes before purge",   ring_len_forward(head, 1000) == 500);
    CHECK("links ok before purge",    ring_links_ok(head));

    while (deleteStringNode(&head, "tag") == 0);

    CHECK("250 nodes after purge",    ring_len_forward(head, 1000) == 250);
    CHECK("backward ring matches",    ring_len_backward(head, 1000) == 250);
    CHECK("all links ok after purge", ring_links_ok(head));

    int only_int = 1;
    Node *cur = head;
    do {
        if (cur->kind != INTEGER) { only_int = 0; break; }
        cur = cur->next;
    } while (cur != head);
    CHECK("only INTEGER nodes remain", only_int);

    free_list(&head);
}

/* ═══════════════════════════════════════════════════════════════════════
   SECTION 6 — reverse()
   ═══════════════════════════════════════════════════════════════════════ */

void test_reverse() {
    printf("\n[reverse] ring integrity + order\n");
    Node *head = NULL;
    insertIntNode(&head, 1);
    insertIntNode(&head, 2);
    insertIntNode(&head, 3);

    reverse(&head);

    CHECK("forward length still 3",   ring_len_forward(head, 20) == 3);
    CHECK("backward length still 3",  ring_len_backward(head, 20) == 3);
    CHECK("bidirectional links ok",   ring_links_ok(head));
    CHECK("new head is 3",            head->data.node_int == 3);
    CHECK("middle is 2",              head->next->data.node_int == 2);
    CHECK("tail is 1",                tail(head)->data.node_int == 1);
    CHECK("tail->next == head",       tail(head)->next == head);

    /* reverse again => original order */
    reverse(&head);
    CHECK("double-reverse: head is 1",  head->data.node_int == 1);
    CHECK("double-reverse: tail is 3",  tail(head)->data.node_int == 3);
    CHECK("double-reverse: links ok",   ring_links_ok(head));

    free_list(&head);
}

void test_reverse_single() {
    printf("\n[reverse] single-node noop\n");
    Node *head = NULL;
    insertIntNode(&head, 42);
    reverse(&head);
    CHECK("head unchanged",           head->data.node_int == 42);
    CHECK("head->next == head",       head->next == head);
    CHECK("head->prev == head",       head->prev == head);
    free_list(&head);
}

/* ═══════════════════════════════════════════════════════════════════════
   MAIN
   ═══════════════════════════════════════════════════════════════════════ */

int main(void) {
    printf("=== circular doubly-linked list — test suite ===\n");

    /* Section 1: Circular structure / ring integrity */
    test_ring_single_node();
    test_ring_two_nodes();
    test_ring_many_nodes();
    test_ring_mixed_types();

    /* Section 2: Node invariants */
    test_node_invariants_int();
    test_node_invariants_string();
    test_node_invariants_float();
    test_node_invariants_double();
    test_node_invariants_empty_string();

    /* Section 3: Edge cases */
    test_free_null_list();
    test_delete_from_empty();
    test_delete_only_node();
    test_delete_head_ring_fix();
    test_delete_tail_ring_fix();
    test_delete_middle_ring_fix();
    test_delete_miss();
    test_delete_type_safety();
    test_search_null();

    /* Section 4: Standard traversal / search */
    test_search_int();
    test_search_string();
    test_search_float();
    test_search_double();
    test_search_after_delete();

    /* Section 5: Stress */
    test_stress_ring();
    test_stress_mixed_ring();

    /* Section 6: reverse() */
    test_reverse();
    test_reverse_single();

    printf("\n═══════════════════════════════\n");
    printf("  PASSED: %d\n", g_pass);
    printf("  FAILED: %d\n", g_fail);
    printf("═══════════════════════════════\n");

    return g_fail > 0 ? 1 : 0;
}