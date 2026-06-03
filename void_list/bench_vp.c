
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "list_vp.h"

#define N      1000    /* latency samples                */
#define BATCH  1000    /* ops per sample                 */
#define NANO   1000000000L

/* ── comparators ─────────────────────────────────────────────────────── */
static int cmp_int   (void *a, void *b) { return *(int*)a == *(int*)b; }
static int cmp_string(void *a, void *b) { return strcmp((char*)a, (char*)b) == 0; }

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
    int i = (int)(p * n); if (i >= n) i = n - 1; return arr[i];
}
static void print_stats(const char *label, long *times, int n) {
    qsort(times, n, sizeof(long), cmp_long);
    printf("  %-36s  p50=%5ld ns   p95=%5ld ns   p99=%5ld ns\n",
           label, pct(times, n, 0.50), pct(times, n, 0.95), pct(times, n, 0.99));
}

/* ════════════════════════════════════════════════════════════════════
   INSERT
   ════════════════════════════════════════════════════════════════════ */

void bench_insertEnd_empty(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            Node *head = NULL;
            insertEnd(&head, &j, sizeof(int));
            free_list(&head);
        }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("insertEnd (empty list)", times, N);
}

/* append to a stable 100-node list — O(1) tail via head->prev */
void bench_insertEnd_tail100(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 100; j++) insertEnd(&head, &j, sizeof(int));

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            insertEnd(&head, &j, sizeof(int));
            deleteNode(&head, &j, cmp_int);   /* keep length stable */
        }
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("insertEnd (append to 100)", times, N);
}

void bench_insertHead_empty(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            Node *head = NULL;
            insertHead(&head, &j, sizeof(int));
            free_list(&head);
        }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("insertHead (empty list)", times, N);
}

void bench_insertEnd_string_short(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            Node *head = NULL;
            insertEnd(&head, "hello", 6);
            free_list(&head);
        }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("insertEnd string short (5 B)", times, N);
}

void bench_insertEnd_string_long(void) {
    long times[N];
    const char *str = "a very long string that is much bigger than a cache line and stresses malloc";
    size_t len = strlen(str) + 1;
    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            Node *head = NULL;
            insertEnd(&head, (void*)str, len);
            free_list(&head);
        }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("insertEnd string long (77 B)", times, N);
}

/* ════════════════════════════════════════════════════════════════════
   DELETE
   ════════════════════════════════════════════════════════════════════ */

void bench_deleteNode_head(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 10; j++) insertEnd(&head, &j, sizeof(int));

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            int v = 0;
            deleteNode(&head, &v, cmp_int);
            insertHead(&head, &v, sizeof(int));   /* restore */
        }
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("deleteNode head hit (10 nodes)", times, N);
}

void bench_deleteNode_middle(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 10; j++) insertEnd(&head, &j, sizeof(int));

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            int v = 5;
            deleteNode(&head, &v, cmp_int);
            insertEnd(&head, &v, sizeof(int));    /* restore at tail */
        }
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("deleteNode middle hit (10 nodes)", times, N);
}

void bench_deleteNode_miss(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 10; j++) insertEnd(&head, &j, sizeof(int));

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            int v = 99;
            deleteNode(&head, &v, cmp_int);   /* full ring traversal, no hit */
        }
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("deleteNode miss (10 nodes)", times, N);
}

void bench_deleteNode_string_hit(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        insertEnd(&head, "apple",  6);
        insertEnd(&head, "banana", 7);
        insertEnd(&head, "cherry", 7);

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            deleteNode(&head, "banana", cmp_string);
            insertEnd(&head, "banana", 7);
        }
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("deleteNode string hit", times, N);
}

void bench_deleteNode_string_miss(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        insertEnd(&head, "apple",  6);
        insertEnd(&head, "banana", 7);
        insertEnd(&head, "cherry", 7);

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            deleteNode(&head, "mango", cmp_string);
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("deleteNode string miss", times, N);
}

/* ════════════════════════════════════════════════════════════════════
   SEARCH
   ════════════════════════════════════════════════════════════════════ */

void bench_searchNode_head(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < 10; j++) insertEnd(&head, &j, sizeof(int));

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) { int v = 0; searchNode(head, &v, cmp_int); }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchNode head hit (10 nodes)", times, N);
    free_list(&head);
}

void bench_searchNode_tail(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < 10; j++) insertEnd(&head, &j, sizeof(int));

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) { int v = 9; searchNode(head, &v, cmp_int); }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchNode tail hit (10 nodes)", times, N);
    free_list(&head);
}

void bench_searchNode_miss(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < 10; j++) insertEnd(&head, &j, sizeof(int));

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) { int v = 99; searchNode(head, &v, cmp_int); }
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchNode miss (10 nodes)", times, N);
    free_list(&head);
}

void bench_searchNode_string(void) {
    long times[N];
    Node *head = NULL;
    insertEnd(&head, "apple",  6);
    insertEnd(&head, "banana", 7);
    insertEnd(&head, "cherry", 7);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) searchNode(head, "cherry", cmp_string);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchNode string tail hit", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   REVERSE
   ════════════════════════════════════════════════════════════════════ */

void bench_reverse_10(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < 10; j++) insertEnd(&head, &j, sizeof(int));

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) reverse(&head);  /* even iters cancel */
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("reverse (10 nodes)", times, N);
    free_list(&head);
}

void bench_reverse_1000(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < 1000; j++) insertEnd(&head, &j, sizeof(int));

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) reverse(&head);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("reverse (1000 nodes)", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   FREE  (deallocation cost at scale — 2 frees per node vs 1 for tagged)
   ════════════════════════════════════════════════════════════════════ */

void bench_free_int_1000(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 1000; j++) insertEnd(&head, &j, sizeof(int));
        long t0 = now_ns();
        free_list(&head);
        times[i] = now_ns() - t0;   /* one free per sample is correct here */
    }
    print_stats("free_list 1000 ints", times, N);
}

void bench_free_string_1000(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 1000; j++) insertEnd(&head, "bench", 6);
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
    printf("=== void* list benchmark  (N=%d samples, BATCH=%d ops/sample) ===\n\n", N, BATCH);

    printf("-- insert --\n");
    bench_insertEnd_empty();
    bench_insertEnd_tail100();
    bench_insertHead_empty();
    bench_insertEnd_string_short();
    bench_insertEnd_string_long();

    printf("\n-- delete --\n");
    bench_deleteNode_head();
    bench_deleteNode_middle();
    bench_deleteNode_miss();
    bench_deleteNode_string_hit();
    bench_deleteNode_string_miss();

    printf("\n-- search --\n");
    bench_searchNode_head();
    bench_searchNode_tail();
    bench_searchNode_miss();
    bench_searchNode_string();

    printf("\n-- reverse --\n");
    bench_reverse_10();
    bench_reverse_1000();

    printf("\n-- free --\n");
    bench_free_int_1000();
    bench_free_string_1000();

    printf("\ndone\n");
    return 0;
}