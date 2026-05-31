/*
 * bench.c — p50/p95/p99 latency benchmark for circular doubly-linked tagged-union list
 *
 * WHY BATCHING:
 *   clock_gettime(CLOCK_MONOTONIC) has a hardware/OS tick floor — on many
 *   machines (VMs, WSL, some kernels) that floor is ~1-2 µs, meaning any
 *   single operation faster than the clock tick reads as a fixed constant.
 *   Fix: time BATCH operations in one clock sandwich, divide by BATCH.
 *   Each reported sample = average latency over BATCH ops.
 *
 * Compile (optimised — matches real usage):
 *   gcc -I. -O2 -o bench bench.c list.c
 * Compile (no-optimise — raw instruction cost, less compiler magic):
 *   gcc -I. -O0 -o bench bench.c list.c
 */

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "list.h"

#define N      1000    /* outer iterations — number of latency samples    */
#define BATCH  1000    /* ops per sample — amortises clock resolution      */
#define NANO   1000000000L

/* ── timer ─────────────────────────────────────────────────────────── */
static inline long now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * NANO + ts.tv_nsec;
}

/* ── stats ─────────────────────────────────────────────────────────── */
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
    printf("  %-34s  p50=%5ld ns   p95=%5ld ns   p99=%5ld ns\n",
           label, pct(times, n, 0.50), pct(times, n, 0.95), pct(times, n, 0.99));
}

/* ════════════════════════════════════════════════════════════════════
   INSERT
   ════════════════════════════════════════════════════════════════════ */

/* single insert into empty list each time — pure alloc + pointer wiring */
void bench_insertInt_empty(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            Node *head = NULL;
            insertIntNode(&head, j);
            free_list(&head);
        }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("insertInt (empty list)", times, N);
}

/* append to a pre-built 100-node list — exercises O(1) tail via head->prev */
void bench_insertInt_tail100(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        /* build the 100-node base outside the timed region */
        Node *head = NULL;
        for (int j = 0; j < 100; j++) insertIntNode(&head, j);

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            insertIntNode(&head, 999);
            deleteIntNode(&head, 999);   /* keep list length stable */
        }
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("insertInt (append to 100)", times, N);
}

void bench_insertString_short(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            Node *head = NULL;
            insertStringNode(&head, "hello");
            free_list(&head);
        }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("insertString short (5 B)", times, N);
}

void bench_insertString_long(void) {
    long times[N];
    const char *str = "a very long string that is much bigger than a cache line and will stress malloc";
    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            Node *head = NULL;
            insertStringNode(&head, (char *)str);
            free_list(&head);
        }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("insertString long (80 B)", times, N);
}

void bench_insertFloat(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            Node *head = NULL;
            insertFloatNode(&head, 3.14f);
            free_list(&head);
        }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("insertFloat (empty list)", times, N);
}

void bench_insertDouble(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            Node *head = NULL;
            insertDoubleNode(&head, 3.14159265358979);
            free_list(&head);
        }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("insertDouble (empty list)", times, N);
}

/* ════════════════════════════════════════════════════════════════════
   DELETE INT
   ════════════════════════════════════════════════════════════════════ */

void bench_deleteInt_head(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 10; j++) insertIntNode(&head, j);

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            deleteIntNode(&head, 0);       /* hit at head */
            insertIntNode(&head, 0);       /* restore — keep list stable */
        }
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("deleteInt head hit (10 nodes)", times, N);
}

void bench_deleteInt_tail(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 10; j++) insertIntNode(&head, j);

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            deleteIntNode(&head, 9);       /* hit at tail */
            insertIntNode(&head, 9);       /* restore */
        }
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("deleteInt tail hit (10 nodes)", times, N);
}

void bench_deleteInt_miss(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 10; j++) insertIntNode(&head, j);

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            deleteIntNode(&head, 99);      /* miss — full ring traversal */
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("deleteInt miss (10 nodes)", times, N);
}

/* ════════════════════════════════════════════════════════════════════
   DELETE STRING
   ════════════════════════════════════════════════════════════════════ */

void bench_deleteString_head(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        insertStringNode(&head, "apple");
        insertStringNode(&head, "banana");
        insertStringNode(&head, "cherry");

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            deleteStringNode(&head, "apple");
            insertStringNode(&head, "apple");  /* restore at tail, position shifts but cost is same */
        }
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("deleteString head hit", times, N);
}

void bench_deleteString_middle(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        insertStringNode(&head, "apple");
        insertStringNode(&head, "banana");
        insertStringNode(&head, "cherry");

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            deleteStringNode(&head, "banana");
            insertStringNode(&head, "banana");
        }
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("deleteString middle hit", times, N);
}

void bench_deleteString_miss(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        insertStringNode(&head, "apple");
        insertStringNode(&head, "banana");
        insertStringNode(&head, "cherry");

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            deleteStringNode(&head, "mango");  /* miss */
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("deleteString miss", times, N);
}

/* ════════════════════════════════════════════════════════════════════
   SEARCH INT
   ════════════════════════════════════════════════════════════════════ */

void bench_searchInt_head(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < 10; j++) insertIntNode(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchInt(head, 0);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchInt head hit (10 nodes)", times, N);
    free_list(&head);
}

void bench_searchInt_tail(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < 10; j++) insertIntNode(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchInt(head, 9);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchInt tail hit (10 nodes)", times, N);
    free_list(&head);
}

void bench_searchInt_miss(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < 10; j++) insertIntNode(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchInt(head, 99);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchInt miss (10 nodes)", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   SEARCH STRING
   ════════════════════════════════════════════════════════════════════ */

void bench_searchString_head(void) {
    long times[N];
    Node *head = NULL;
    insertStringNode(&head, "apple");
    insertStringNode(&head, "banana");
    insertStringNode(&head, "cherry");

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchString(head, "apple");
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchString head hit", times, N);
    free_list(&head);
}

void bench_searchString_tail(void) {
    long times[N];
    Node *head = NULL;
    insertStringNode(&head, "apple");
    insertStringNode(&head, "banana");
    insertStringNode(&head, "cherry");

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchString(head, "cherry");
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchString tail hit", times, N);
    free_list(&head);
}

void bench_searchString_miss(void) {
    long times[N];
    Node *head = NULL;
    insertStringNode(&head, "apple");
    insertStringNode(&head, "banana");
    insertStringNode(&head, "cherry");

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchString(head, "mango");
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchString miss", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   SEARCH FLOAT / DOUBLE
   ════════════════════════════════════════════════════════════════════ */

void bench_searchFloat(void) {
    long times[N];
    Node *head = NULL;
    insertFloatNode(&head, 1.0f);
    insertFloatNode(&head, 2.0f);
    insertFloatNode(&head, 3.0f);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchFloat(head, 3.0f);   /* tail hit — worst case */
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchFloat tail hit", times, N);
    free_list(&head);
}

void bench_searchDouble(void) {
    long times[N];
    Node *head = NULL;
    insertDoubleNode(&head, 1.1);
    insertDoubleNode(&head, 2.2);
    insertDoubleNode(&head, 3.3);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchDouble(head, 3.3);   /* tail hit */
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchDouble tail hit", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   REVERSE
   ════════════════════════════════════════════════════════════════════ */

void bench_reverse_10(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < 10; j++) insertIntNode(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            reverse(&head);            /* even iterations cancel out */
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("reverse (10 nodes)", times, N);
    free_list(&head);
}

void bench_reverse_1000(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < 1000; j++) insertIntNode(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            reverse(&head);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("reverse (1000 nodes)", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   FREE LIST  (deallocation cost at scale — outer loop rebuilds each time)
   ════════════════════════════════════════════════════════════════════ */

void bench_free_int_1000(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 1000; j++) insertIntNode(&head, j);
        long t0 = now_ns();
        free_list(&head);
        times[i] = now_ns() - t0;     /* no divide — one free per sample is correct */
    }
    print_stats("free_list 1000 ints", times, N);
}

void bench_free_string_1000(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 1000; j++) insertStringNode(&head, "bench");
        long t0 = now_ns();
        free_list(&head);
        times[i] = now_ns() - t0;
    }
    print_stats("free_list 1000 strings", times, N);
}

/* ════════════════════════════════════════════════════════════════════
   MAIN
   ════════════════════════════════════════════════════════════════════ */
int main(void) {
    printf("=== tagged union list benchmark  (N=%d samples, BATCH=%d ops/sample) ===\n\n", N, BATCH);

    printf("-- insert --\n");
    bench_insertInt_empty();
    bench_insertInt_tail100();
    bench_insertString_short();
    bench_insertString_long();
    bench_insertFloat();
    bench_insertDouble();

    printf("\n-- delete int --\n");
    bench_deleteInt_head();
    bench_deleteInt_tail();
    bench_deleteInt_miss();

    printf("\n-- delete string --\n");
    bench_deleteString_head();
    bench_deleteString_middle();
    bench_deleteString_miss();

    printf("\n-- search int --\n");
    bench_searchInt_head();
    bench_searchInt_tail();
    bench_searchInt_miss();

    printf("\n-- search string --\n");
    bench_searchString_head();
    bench_searchString_tail();
    bench_searchString_miss();

    printf("\n-- search float / double --\n");
    bench_searchFloat();
    bench_searchDouble();

    printf("\n-- reverse --\n");
    bench_reverse_10();
    bench_reverse_1000();

    printf("\n-- free --\n");
    bench_free_int_1000();
    bench_free_string_1000();

    printf("\ndone\n");
    return 0;
}