#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "list.h"

DEFINE_LIST(int)
DEFINE_LIST(float)
DEFINE_LIST_STRING

#define ITERATIONS 100000
#define RUNS       1000

static long ns_diff(struct timespec s, struct timespec e) {
    return (e.tv_sec - s.tv_sec) * 1000000000L + (e.tv_nsec - s.tv_nsec);
}
static int cmp_long(const void *a, const void *b) {
    long x = *(long*)a, y = *(long*)b;
    return (x > y) - (x < y);
}
static void print_stats(const char *label, long *s, int n) {
    qsort(s, n, sizeof(long), cmp_long);
    printf("%-38s  p50=%6ld ns  p95=%6ld ns  p99=%6ld ns\n",
           label, s[n/2], s[(int)(n*0.95)], s[(int)(n*0.99)]);
}
/* -----------------------------------------------------------------------
 * int benchmarks
 * ----------------------------------------------------------------------- */
static void bench_int_insert(void) {
    long samples[RUNS];
    for (int r = 0; r < RUNS; r++) {
        intNode *head = NULL;
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        for (int i = 0; i < ITERATIONS; i++)
            int_insert(&head, i);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        samples[r] = ns_diff(t0, t1) / ITERATIONS;
        int_free_list(&head);
    }
    print_stats("int insert", samples, RUNS);
}

static void bench_int_search_hit(void) {
    intNode *head = NULL;
    for (int i = 0; i < ITERATIONS; i++)
        int_insert(&head, i);

    long samples[RUNS];
    for (int r = 0; r < RUNS; r++) {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        int_search(head, ITERATIONS - 1);   /* worst case — tail */
        clock_gettime(CLOCK_MONOTONIC, &t1);
        samples[r] = ns_diff(t0, t1);
    }
    print_stats("int search hit (tail/worst)", samples, RUNS);
    int_free_list(&head);
}

static void bench_int_search_miss(void) {
    intNode *head = NULL;
    for (int i = 0; i < ITERATIONS; i++)
        int_insert(&head, i);

    long samples[RUNS];
    for (int r = 0; r < RUNS; r++) {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        int_search(head, -1);   /* miss — full traversal */
        clock_gettime(CLOCK_MONOTONIC, &t1);
        samples[r] = ns_diff(t0, t1);
    }
    print_stats("int search miss (full scan)", samples, RUNS);
    int_free_list(&head);
}

static void bench_int_delete_head(void) {
    long samples[RUNS];
    for (int r = 0; r < RUNS; r++) {
        intNode *head = NULL;
        for (int i = 0; i < ITERATIONS; i++)
            int_insert(&head, i);
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        for (int i = 0; i < ITERATIONS; i++)
            int_delete_node(&head, i);   /* always deletes head — O(1) find */
        clock_gettime(CLOCK_MONOTONIC, &t1);
        samples[r] = ns_diff(t0, t1) / ITERATIONS;
    }
    print_stats("int delete head (best case)", samples, RUNS);
}

static void bench_int_reverse(void) {
    intNode *head = NULL;
    for (int i = 0; i < ITERATIONS; i++)
        int_insert(&head, i);

    long samples[RUNS];
    for (int r = 0; r < RUNS; r++) {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        int_reverse(&head);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        samples[r] = ns_diff(t0, t1);
    }
    print_stats("int reverse", samples, RUNS);
    int_free_list(&head);
}

/* -----------------------------------------------------------------------
 * string benchmarks
 * ----------------------------------------------------------------------- */
static void bench_str_insert(void) {
    char words[ITERATIONS][16];
    for (int i = 0; i < ITERATIONS; i++)
        snprintf(words[i], sizeof(words[i]), "str%d", i);

    long samples[RUNS];
    for (int r = 0; r < RUNS; r++) {
        strNode *head = NULL;
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        for (int i = 0; i < ITERATIONS; i++)
            str_insert(&head, words[i]);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        samples[r] = ns_diff(t0, t1) / ITERATIONS;
        str_free_list(&head);
    }
    print_stats("str insert (strdup per node)", samples, RUNS);
}

static void bench_str_search_hit(void) {
    char words[ITERATIONS][16];
    strNode *head = NULL;
    for (int i = 0; i < ITERATIONS; i++) {
        snprintf(words[i], sizeof(words[i]), "str%d", i);
        str_insert(&head, words[i]);
    }

    char target[16];
    snprintf(target, sizeof(target), "str%d", ITERATIONS - 1);

    long samples[RUNS];
    for (int r = 0; r < RUNS; r++) {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        str_search(head, target);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        samples[r] = ns_diff(t0, t1);
    }
    print_stats("str search hit (tail/worst)", samples, RUNS);
    str_free_list(&head);
}

static void bench_str_search_miss(void) {
    char words[ITERATIONS][16];
    strNode *head = NULL;
    for (int i = 0; i < ITERATIONS; i++) {
        snprintf(words[i], sizeof(words[i]), "str%d", i);
        str_insert(&head, words[i]);
    }

    long samples[RUNS];
    for (int r = 0; r < RUNS; r++) {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        str_search(head, "NOTHERE");
        clock_gettime(CLOCK_MONOTONIC, &t1);
        samples[r] = ns_diff(t0, t1);
    }
    print_stats("str search miss (full scan)", samples, RUNS);
    str_free_list(&head);
}

static void bench_str_delete(void) {
    char words[ITERATIONS][16];
    for (int i = 0; i < ITERATIONS; i++)
        snprintf(words[i], sizeof(words[i]), "str%d", i);

    long samples[RUNS];
    for (int r = 0; r < RUNS; r++) {
        strNode *head = NULL;
        for (int i = 0; i < ITERATIONS; i++)
            str_insert(&head, words[i]);
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        for (int i = 0; i < ITERATIONS; i++)
            str_delete(&head, words[i]);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        samples[r] = ns_diff(t0, t1) / ITERATIONS;
    }
    print_stats("str delete (sequential)", samples, RUNS);
}

static void bench_str_reverse(void) {
    strNode *head = NULL;
    char words[ITERATIONS][16];
    for (int i = 0; i < ITERATIONS; i++) {
        snprintf(words[i], sizeof(words[i]), "str%d", i);
        str_insert(&head, words[i]);
    }

    long samples[RUNS];
    for (int r = 0; r < RUNS; r++) {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        str_reverse(&head);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        samples[r] = ns_diff(t0, t1);
    }
    print_stats("str reverse", samples, RUNS);
    str_free_list(&head);
}

int main(void) {
    printf("=== macro list bench  (%d iters, %d runs) ===\n\n",
           ITERATIONS, RUNS);

    bench_int_insert();
    bench_int_search_hit();
    bench_int_search_miss();
    bench_int_delete_head();
    bench_int_reverse();
    printf("\n");
    bench_str_insert();
    bench_str_search_hit();
    bench_str_search_miss();
    bench_str_delete();
    bench_str_reverse();

    printf("\ndone\n");
    return 0;
}