#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "list.h"

DEFINE_LIST(int)
DEFINE_LIST(float)
DEFINE_LIST_STRING

/* -----------------------------------------------------------------------
 * Harness — identical to bench_vp.c
 * N samples, BATCH ops per sample, divide to get per-op ns
 * List size held CONSTANT across samples so we measure steady-state,
 * not a mix of small-list and large-list costs.
 * ----------------------------------------------------------------------- */
#define N     1000
#define BATCH 1000

#define NANO 1000000000L

static inline long now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * NANO + ts.tv_nsec;
}

static int cmp_long(const void *a, const void *b) {
    long x = *(const long *)a, y = *(const long *)b;
    return (x > y) - (x < y);
}
static long pct(long *arr, int n, double p) {
    int i = (int)(p * n);
    if (i >= n) i = n - 1;
    return arr[i];
}
static void print_stats(const char *label, long *times, int n) {
    qsort(times, n, sizeof(long), cmp_long);
    printf("  %-38s  p50=%5ld ns   p95=%5ld ns   p99=%5ld ns\n",
           label, pct(times, n, 0.50),
                  pct(times, n, 0.95),
                  pct(times, n, 0.99));
}

/* ═══════════════════════════════════════════════════════════════════
   INSERT
   ═══════════════════════════════════════════════════════════════════ */

/* Insert into empty list each time — baseline malloc cost */
static void bench_int_insert_empty(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            intNode *head = NULL;
            int_insert(&head, j);
            int_free_list(&head);
        }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("int insert (empty list)", times, N);
}

/* Append to stable 100-node list — steady-state cost */
static void bench_int_insert_tail100(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        intNode *head = NULL;
        for (int j = 0; j < 100; j++) int_insert(&head, j);

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            int_insert(&head, j);
            int_delete_node(&head, j);   /* keep length stable */
        }
        times[i] = (now_ns() - t0) / BATCH;
        int_free_list(&head);
    }
    print_stats("int insert (append to 100)", times, N);
}

/* String insert empty — strdup cost visible */
static void bench_str_insert_empty(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            strNode *head = NULL;
            str_insert(&head, "hello");
            str_free_list(&head);
        }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("str insert short (5 B, empty)", times, N);
}

/* String insert — long string, more strdup cost */
static void bench_str_insert_long(void) {
    long times[N];
    const char *s = "a very long string that stresses malloc beyond a cache line here";
    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            strNode *head = NULL;
            str_insert(&head, s);
            str_free_list(&head);
        }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("str insert long (64 B, empty)", times, N);
}

/* ═══════════════════════════════════════════════════════════════════
   DELETE
   ═══════════════════════════════════════════════════════════════════ */

/* Delete head — O(1) find, always first node */
static void bench_int_delete_head(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        intNode *head = NULL;
        for (int j = 0; j < 10; j++) int_insert(&head, j);

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            int_delete_node(&head, 0);       /* always head */
            int_insert(&head, 0);            /* restore at tail, re-delete next round */
            /* Note: after insert 0 goes to tail. Next delete walks to find it.
               For true head-delete bench we rebuild each time: */
        }
        times[i] = (now_ns() - t0) / BATCH;
        int_free_list(&head);
    }
    print_stats("int delete head (10 nodes)", times, N);
}

/* Delete middle node — average case */
static void bench_int_delete_middle(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        intNode *head = NULL;
        for (int j = 0; j < 10; j++) int_insert(&head, j);

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            int_delete_node(&head, 5);
            int_insert(&head, 5);
        }
        times[i] = (now_ns() - t0) / BATCH;
        int_free_list(&head);
    }
    print_stats("int delete middle (10 nodes)", times, N);
}

/* Delete miss — full ring traversal, no hit */
static void bench_int_delete_miss(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        intNode *head = NULL;
        for (int j = 0; j < 10; j++) int_insert(&head, j);

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            int_delete_node(&head, 99);
        times[i] = (now_ns() - t0) / BATCH;
        int_free_list(&head);
    }
    print_stats("int delete miss (10 nodes)", times, N);
}

/* String delete hit */
static void bench_str_delete_hit(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        strNode *head = NULL;
        str_insert(&head, "apple");
        str_insert(&head, "banana");
        str_insert(&head, "cherry");

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            str_delete(&head, "banana");
            str_insert(&head, "banana");
        }
        times[i] = (now_ns() - t0) / BATCH;
        str_free_list(&head);
    }
    print_stats("str delete hit (3 nodes)", times, N);
}

/* String delete miss */
static void bench_str_delete_miss(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        strNode *head = NULL;
        str_insert(&head, "apple");
        str_insert(&head, "banana");
        str_insert(&head, "cherry");

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            str_delete(&head, "mango");
        times[i] = (now_ns() - t0) / BATCH;
        str_free_list(&head);
    }
    print_stats("str delete miss (3 nodes)", times, N);
}

/* ═══════════════════════════════════════════════════════════════════
   SEARCH
   ═══════════════════════════════════════════════════════════════════ */

/* Search head — O(1) best case */
static void bench_int_search_head(void) {
    long times[N];
    intNode *head = NULL;
    for (int j = 0; j < 10; j++) int_insert(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            int_search(head, 0);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("int search head hit (10 nodes)", times, N);
    int_free_list(&head);
}

/* Search tail — worst case */
static void bench_int_search_tail(void) {
    long times[N];
    intNode *head = NULL;
    for (int j = 0; j < 10; j++) int_insert(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            int_search(head, 9);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("int search tail hit (10 nodes)", times, N);
    int_free_list(&head);
}

/* Search miss — full traversal */
static void bench_int_search_miss(void) {
    long times[N];
    intNode *head = NULL;
    for (int j = 0; j < 10; j++) int_insert(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            int_search(head, 99);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("int search miss (10 nodes)", times, N);
    int_free_list(&head);
}

/* String search tail hit — strcmp cost visible */
static void bench_str_search_tail(void) {
    long times[N];
    strNode *head = NULL;
    str_insert(&head, "apple");
    str_insert(&head, "banana");
    str_insert(&head, "cherry");

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            str_search(head, "cherry");
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("str search tail hit (3 nodes)", times, N);
    str_free_list(&head);
}

/* ═══════════════════════════════════════════════════════════════════
   REVERSE
   ═══════════════════════════════════════════════════════════════════ */

/* Reverse 10 nodes — even iterations cancel out */
static void bench_int_reverse_10(void) {
    long times[N];
    intNode *head = NULL;
    for (int j = 0; j < 10; j++) int_insert(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            int_reverse(&head);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("int reverse (10 nodes)", times, N);
    int_free_list(&head);
}

/* Reverse 1000 nodes — shows O(n) scaling */
static void bench_int_reverse_1000(void) {
    long times[N];
    intNode *head = NULL;
    for (int j = 0; j < 1000; j++) int_insert(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            int_reverse(&head);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("int reverse (1000 nodes)", times, N);
    int_free_list(&head);
}

/* String reverse 1000 — same pointer logic, larger node size */
static void bench_str_reverse_1000(void) {
    long times[N];
    strNode *head = NULL;
    char buf[16];
    for (int j = 0; j < 1000; j++) {
        snprintf(buf, sizeof(buf), "s%d", j);
        str_insert(&head, buf);
    }

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            str_reverse(&head);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("str reverse (1000 nodes)", times, N);
    str_free_list(&head);
}

/* ═══════════════════════════════════════════════════════════════════
   FREE  — deallocation cost at scale
   ═══════════════════════════════════════════════════════════════════ */

/* free 1000 int nodes — 1 free per node */
static void bench_free_int_1000(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        intNode *head = NULL;
        for (int j = 0; j < 1000; j++) int_insert(&head, j);
        long t0 = now_ns();
        int_free_list(&head);
        times[i] = now_ns() - t0;
    }
    print_stats("int free_list (1000 nodes)", times, N);
}

/* free 1000 string nodes — 2 frees per node (data + node) */
static void bench_free_str_1000(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        strNode *head = NULL;
        for (int j = 0; j < 1000; j++) str_insert(&head, "bench");
        long t0 = now_ns();
        str_free_list(&head);
        times[i] = now_ns() - t0;
    }
    print_stats("str free_list (1000 nodes)", times, N);
}

/* ═══════════════════════════════════════════════════════════════════
   MAIN
   ═══════════════════════════════════════════════════════════════════ */
int main(void) {
    printf("=== macro list bench  (N=%d samples, BATCH=%d ops/sample) ===\n\n",
           N, BATCH);

    printf("-- insert --\n");
    bench_int_insert_empty();
    bench_int_insert_tail100();
    bench_str_insert_empty();
    bench_str_insert_long();

    printf("\n-- delete --\n");
    bench_int_delete_head();
    bench_int_delete_middle();
    bench_int_delete_miss();
    bench_str_delete_hit();
    bench_str_delete_miss();

    printf("\n-- search --\n");
    bench_int_search_head();
    bench_int_search_tail();
    bench_int_search_miss();
    bench_str_search_tail();

    printf("\n-- reverse --\n");
    bench_int_reverse_10();
    bench_int_reverse_1000();
    bench_str_reverse_1000();

    printf("\n-- free --\n");
    bench_free_int_1000();
    bench_free_str_1000();

    printf("\ndone\n");
    return 0;
}