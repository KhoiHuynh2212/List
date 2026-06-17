// test-embedded.c
//
// Exercises the intrusive list against a realistic embedded struct,
// covering list_entry() NULL-safety, incremental linking, safe-iterator
// removal, and the less commonly tested operations (pop, replace,
// splice-then-reuse, length tracking through a full lifecycle).

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "instrusive_list.h"

typedef struct {
    int id;
    char name[16];
    list link;
} Entry;

static Entry make_entry(int id, const char* name) {
    Entry e;
    e.id = id;
    strncpy(e.name, name, sizeof(e.name) - 1);
    e.name[sizeof(e.name) - 1] = '\0';
    list_init(&e.link);
    return e;
}

static void print_list(const char* label, list* head) {
    printf("%-28s: ", label);
    list* pos;
    list_for_each(pos, head) {
        Entry* e = list_entry(pos, Entry, link);
        printf("%s(%d) ", e->name, e->id);
    }
    printf("\n");
}

static void test_list_entry_null_safety(void) {
    printf("-- list_entry() / list_entry_offset() NULL safety --\n");

    /* verify c_list_entry() works as expected (even with NULL) */
    assert(list_entry_offset(NULL, 0) == NULL);
    assert(list_entry_offset(NULL, offsetof(Entry, link)) == NULL);
    assert(list_entry(NULL, Entry, link) == NULL);

    Entry e = make_entry(1, "alpha");
    Entry* recovered = list_entry(&e.link, Entry, link);
    assert(recovered == &e);
    assert(recovered->id == 1);
    assert(strcmp(recovered->name, "alpha") == 0);

    printf("  passed\n\n");
}

static void test_link_two_entries(void) {
    printf("-- link 2 entries and verify list state --\n");

    list head = LIST_INIT(head);
    assert(list_is_empty(&head) == true);
    assert(list_length(&head) == 0);

    Entry a = make_entry(1, "alpha");
    Entry b = make_entry(2, "beta");

    list_add_tail(&head, &a.link);
    assert(list_is_empty(&head) == false);
    assert(list_length(&head) == 1);
    assert(list_is_first(&a.link, &head) == true);
    assert(list_is_last(&a.link, &head) == true); // only element: first AND last

    list_add_tail(&head, &b.link);
    assert(list_length(&head) == 2);
    assert(list_is_first(&a.link, &head) == true);
    assert(list_is_last(&a.link, &head) == false);
    assert(list_is_first(&b.link, &head) == false);
    assert(list_is_last(&b.link, &head) == true);

    print_list("after linking alpha, beta", &head);

    // tear down before next test reuses these stack entries' memory pattern
    list_unlink(&a.link);
    list_unlink(&b.link);
    assert(list_is_empty(&head));

    printf("  passed\n\n");
}

static void test_link_two_more_entries(void) {
    printf("-- link 2 more entries (4 total) --\n");

    list head = LIST_INIT(head);
    Entry a = make_entry(1, "alpha");
    Entry b = make_entry(2, "beta");
    Entry c = make_entry(3, "gamma");
    Entry d = make_entry(4, "delta");

    list_add_tail(&head, &a.link);
    list_add_tail(&head, &b.link);
    assert(list_length(&head) == 2);

    /* link 2 more entries */
    list_add_tail(&head, &c.link);
    list_add_tail(&head, &d.link);
    assert(list_length(&head) == 4);

    print_list("after linking all 4", &head);

    // verify exact order via direct traversal, not just count
    list* pos = head.next;
    assert(list_entry(pos, Entry, link)->id == 1); pos = pos->next;
    assert(list_entry(pos, Entry, link)->id == 2); pos = pos->next;
    assert(list_entry(pos, Entry, link)->id == 3); pos = pos->next;
    assert(list_entry(pos, Entry, link)->id == 4); pos = pos->next;
    assert(pos == &head);

    // first/last accessors on the full 4-element list
    assert(list_is_first(&a.link, &head) == true);
    assert(list_is_last(&d.link, &head) == true);
    assert(list_is_first(&b.link, &head) == false);
    assert(list_is_last(&c.link, &head) == false);

    printf("  passed\n\n");
}

static void test_remove_via_safe_iterator(void) {
    printf("-- remove via safe iterator --\n");

    list head = LIST_INIT(head);
    Entry a = make_entry(1, "alpha");
    Entry b = make_entry(2, "beta");
    Entry c = make_entry(3, "gamma");
    Entry d = make_entry(4, "delta");

    list_add_tail(&head, &a.link);
    list_add_tail(&head, &b.link);
    list_add_tail(&head, &c.link);
    list_add_tail(&head, &d.link);
    assert(list_length(&head) == 4);

    /* remove via safe iterator */
    // remove every entry with an even id (beta=2, delta=4)
    list* pos;
    list* tmp;
    list_for_each_safe(pos, tmp, &head) {
        Entry* e = list_entry(pos, Entry, link);
        if (e->id % 2 == 0) {
            list_unlink(pos);
        }
    }

    assert(list_length(&head) == 2);
    assert(list_is_linked(&b.link) == false);
    assert(list_is_linked(&d.link) == false);
    assert(list_is_linked(&a.link) == true);
    assert(list_is_linked(&c.link) == true);

    print_list("after removing even ids", &head);

    // confirm remaining order: alpha -> gamma
    list* p = head.next;
    assert(list_entry(p, Entry, link)->id == 1); p = p->next;
    assert(list_entry(p, Entry, link)->id == 3); p = p->next;
    assert(p == &head);

    // edge case: safe iterator removing the head and the tail simultaneously
    // in a separate 3-element list, leaving only the middle element
    list head2 = LIST_INIT(head2);
    Entry x = make_entry(10, "x");
    Entry y = make_entry(20, "y");
    Entry z = make_entry(30, "z");
    list_add_tail(&head2, &x.link);
    list_add_tail(&head2, &y.link);
    list_add_tail(&head2, &z.link);

    list_for_each_safe(pos, tmp, &head2) {
        Entry* e = list_entry(pos, Entry, link);
        if (e->id != 20) {
            list_unlink(pos);
        }
    }
    assert(list_length(&head2) == 1);
    assert(list_entry(head2.next, Entry, link)->id == 20);
    assert(list_is_linked(&x.link) == false);
    assert(list_is_linked(&z.link) == false);

    printf("  passed\n\n");
}

static void test_pop_front_back(void) {
    printf("-- list_pop_front / list_pop_back --\n");

    list head = LIST_INIT(head);

    // pop on an empty list must return NULL, not crash
    assert(list_pop_front(&head) == NULL);
    assert(list_pop_back(&head) == NULL);

    Entry a = make_entry(1, "alpha");
    Entry b = make_entry(2, "beta");
    Entry c = make_entry(3, "gamma");
    list_add_tail(&head, &a.link);
    list_add_tail(&head, &b.link);
    list_add_tail(&head, &c.link);

    list* front = list_pop_front(&head);
    assert(list_entry(front, Entry, link)->id == 1);
    assert(list_length(&head) == 2);
    assert(list_is_linked(&a.link) == false); // popped node is reinitialized

    list* back = list_pop_back(&head);
    assert(list_entry(back, Entry, link)->id == 3);
    assert(list_length(&head) == 1);
    assert(list_is_linked(&c.link) == false);

    // only beta remains
    assert(list_entry(head.next, Entry, link)->id == 2);

    printf("  passed\n\n");
}

static void test_replace_and_replace_init(void) {
    printf("-- list_replace / list_replace_init --\n");

    list head = LIST_INIT(head);
    Entry a = make_entry(1, "alpha");
    Entry b = make_entry(2, "beta");
    Entry c = make_entry(3, "gamma");
    list_add_tail(&head, &a.link);
    list_add_tail(&head, &b.link);
    list_add_tail(&head, &c.link);

    // list_replace: swap beta out for a fresh node, old node keeps stale links
    Entry repl = make_entry(99, "replacement");
    list_replace(&b.link, &repl.link);
    assert(list_length(&head) == 3); // size unchanged
    assert(list_entry(head.next->next, Entry, link)->id == 99); // middle slot
    assert(list_is_linked(&b.link) == true); // old node NOT reinitialized — still "linked" per its stale pointers

    // list_replace_init: same swap, but old node becomes properly detached
    Entry repl2 = make_entry(100, "replacement2");
    list_replace_init(&repl.link, &repl2.link);
    assert(list_length(&head) == 3);
    assert(list_entry(head.next->next, Entry, link)->id == 100);
    assert(list_is_linked(&repl.link) == false); // this time, properly detached

    printf("  passed\n\n");
}

static void test_splice_then_reuse(void) {
    printf("-- splice, then reuse the drained source list --\n");

    list target = LIST_INIT(target);
    list source = LIST_INIT(source);

    Entry t1 = make_entry(1, "t1");
    Entry s1 = make_entry(10, "s1");
    Entry s2 = make_entry(20, "s2");

    list_add_tail(&target, &t1.link);
    list_add_tail(&source, &s1.link);
    list_add_tail(&source, &s2.link);

    list_splice(&target, &source);
    assert(list_length(&target) == 3);
    assert(list_is_empty(&source) == true);

    // source must be a valid, reusable empty anchor after splice —
    // not just "logically empty" but actually self-linked
    assert(source.next == &source);
    assert(source.prev == &source);

    Entry s3 = make_entry(30, "s3");
    list_add_tail(&source, &s3.link); // reuse source as a fresh list
    assert(list_length(&source) == 1);
    assert(list_entry(source.next, Entry, link)->id == 30);

    // target is unaffected by source's reuse
    assert(list_length(&target) == 3);

    printf("  passed\n\n");
}

static void test_full_lifecycle(void) {
    printf("-- full lifecycle: build, mutate, drain --\n");

    list head = LIST_INIT(head);
    enum { N = 6 };
    Entry entries[N];
    char namebuf[N][8];

    for (int i = 0; i < N; i++) {
        snprintf(namebuf[i], sizeof(namebuf[i]), "e%d", i);
        entries[i] = make_entry(i, namebuf[i]);
        list_add_tail(&head, &entries[i].link);
    }
    assert(list_length(&head) == N);

    // drain entirely via list_pop_front, verify order matches insertion order
    for (int i = 0; i < N; i++) {
        list* popped = list_pop_front(&head);
        assert(popped != NULL);
        assert(list_entry(popped, Entry, link)->id == i);
    }
    assert(list_is_empty(&head) == true);
    assert(list_pop_front(&head) == NULL); // confirm fully drained

    printf("  passed\n\n");
}

int main(void) {
    test_list_entry_null_safety();
    test_link_two_entries();
    test_link_two_more_entries();
    test_remove_via_safe_iterator();
    test_pop_front_back();
    test_replace_and_replace_init();
    test_splice_then_reuse();
    test_full_lifecycle();

    printf("ALL TESTS PASSED\n");
    return 0;
}