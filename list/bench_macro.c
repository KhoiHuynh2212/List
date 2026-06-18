#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "list.h"

DEFINE_LIST(int)
DEFINE_LIST(float)
DEFINE_LIST_STRING

#define N      1000
#define BATCH  1000
#define LIST_N 100000
#define NANO   1000000000L

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
    printf("  %-42s  p50=%8ld ns   p95=%8ld ns   p99=%8ld ns\n",
           label, pct(times, n, 0.50), pct(times, n, 0.95), pct(times, n, 0.99));
}

/* ════════════════════════════════════════════════════════════════════
   INSERT — allocation-dominated; small list is intentional.
   ════════════════════════════════════════════════════════════════════ */

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

/* Append to stable 100-node list, keep length fixed. */
static void bench_int_insert_tail100(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        intNode *head = NULL;
        for (int j = 0; j < 100; j++) int_insert(&head, j);

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            int_insert(&head, j);
            int_delete_node(&head, j);
        }
        times[i] = (now_ns() - t0) / BATCH;
        int_free_list(&head);
    }
    print_stats("int insert (append to 100)", times, N);
}

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

/* ════════════════════════════════════════════════════════════════════
   DELETE
   - head: O(1), small list
   - tail: O(n), LIST_N, one op per sample
   - miss: O(n), LIST_N, list reused
   ════════════════════════════════════════════════════════════════════ */

static void bench_int_delete_head(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        intNode *head = NULL;
        for (int j = 0; j < 10; j++) int_insert(&head, j);

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            int_delete_node(&head, 0);
            int_insert(&head, 0);
        }
        times[i] = (now_ns() - t0) / BATCH;
        int_free_list(&head);
    }
    print_stats("int delete head (10 nodes)", times, N);
}

/*
 * Tail delete on LIST_N nodes.  Rebuild per sample; one op per sample is
 * sufficient because LIST_N >> clock resolution (~1 µs).
 */
static void bench_int_delete_tail(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        intNode *head = NULL;
        for (int j = 0; j < LIST_N; j++) int_insert(&head, j);

        long t0 = now_ns();
        int_delete_node(&head, LIST_N - 1);
        times[i] = now_ns() - t0;

        int_free_list(&head);
    }
    print_stats("int delete tail (100k nodes)", times, N);
}

/* Miss — full scan, list reused. */
static void bench_int_delete_miss(void) {
    long times[N];
    intNode *head = NULL;
    for (int j = 0; j < LIST_N; j++) int_insert(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            int_delete_node(&head, -1);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("int delete miss (100k nodes)", times, N);
    int_free_list(&head);
}

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

static void bench_str_delete_miss(void) {
    long times[N];
    strNode *head = NULL;
    char buf[16];
    for (int j = 0; j < LIST_N; j++) {
        snprintf(buf, sizeof(buf), "n%d", j);
        str_insert(&head, buf);
    }

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            str_delete(&head, "zzz_miss");
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("str delete miss (100k nodes)", times, N);
    str_free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   SEARCH — LIST_N for all cases; list built once, reused.
   ════════════════════════════════════════════════════════════════════ */

static void bench_int_search_head(void) {
    long times[N];
    intNode *head = NULL;
    for (int j = 0; j < LIST_N; j++) int_insert(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            int_search(head, 0);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("int search head hit (100k nodes)", times, N);
    int_free_list(&head);
}

static void bench_int_search_tail(void) {
    long times[N];
    intNode *head = NULL;
    for (int j = 0; j < LIST_N; j++) int_insert(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            int_search(head, LIST_N - 1);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("int search tail hit (100k nodes)", times, N);
    int_free_list(&head);
}

static void bench_int_search_miss(void) {
    long times[N];
    intNode *head = NULL;
    for (int j = 0; j < LIST_N; j++) int_insert(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            int_search(head, -1);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("int search miss (100k nodes)", times, N);
    int_free_list(&head);
}

static void bench_str_search_tail(void) {
    long times[N];
    strNode *head = NULL;
    char buf[16];
    char tail_val[16];
    for (int j = 0; j < LIST_N; j++) {
        snprintf(buf, sizeof(buf), "n%d", j);
        str_insert(&head, buf);
    }
    snprintf(tail_val, sizeof(tail_val), "n%d", LIST_N - 1);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            str_search(head, tail_val);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("str search tail hit (100k nodes)", times, N);
    str_free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   REVERSE — LIST_N nodes, one reverse per sample.
   ════════════════════════════════════════════════════════════════════ */

static void bench_int_reverse(void) {
    long times[N];
    intNode *head = NULL;
    for (int j = 0; j < LIST_N; j++) int_insert(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        int_reverse(&head);
        times[i] = now_ns() - t0;
    }
    print_stats("int reverse (100k nodes)", times, N);
    int_free_list(&head);
}

static void bench_str_reverse(void) {
    long times[N];
    strNode *head = NULL;
    char buf[16];
    for (int j = 0; j < LIST_N; j++) {
        snprintf(buf, sizeof(buf), "n%d", j);
        str_insert(&head, buf);
    }

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        str_reverse(&head);
        times[i] = now_ns() - t0;
    }
    print_stats("str reverse (100k nodes)", times, N);
    str_free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   FREE — LIST_N nodes, rebuild each sample, one free_list per sample.
   ════════════════════════════════════════════════════════════════════ */

static void bench_free_int(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        intNode *head = NULL;
        for (int j = 0; j < LIST_N; j++) int_insert(&head, j);
        long t0 = now_ns();
        int_free_list(&head);
        times[i] = now_ns() - t0;
    }
    print_stats("int free_list (100k nodes)", times, N);
}

static void bench_free_str(void) {
    long times[N];
    char buf[16];
    for (int i = 0; i < N; i++) {
        strNode *head = NULL;
        for (int j = 0; j < LIST_N; j++) {
            snprintf(buf, sizeof(buf), "n%d", j);
            str_insert(&head, buf);
        }
        long t0 = now_ns();
        str_free_list(&head);
        times[i] = now_ns() - t0;
    }
    print_stats("str free_list (100k nodes)", times, N);
}

/* ════════════════════════════════════════════════════════════════════
   MAIN
   ════════════════════════════════════════════════════════════════════ */
int main(void) {
    printf("=== macro list benchmark"
           "  (N=%d samples, BATCH=%d, LIST_N=%d) ===\n\n",
           N, BATCH, LIST_N);

    printf("-- insert (allocation-dominated, small list) --\n");
    bench_int_insert_empty();
    bench_int_insert_tail100();
    bench_str_insert_empty();
    bench_str_insert_long();

    printf("\n-- delete int --\n");
    bench_int_delete_head();        /* O(1), small list  */
    bench_int_delete_tail();        /* O(n), 100k nodes  */
    bench_int_delete_miss();        /* O(n), 100k nodes  */

    printf("\n-- delete string --\n");
    bench_str_delete_hit();
    bench_str_delete_miss();        /* 100k nodes */

    printf("\n-- search int (100k nodes) --\n");
    bench_int_search_head();
    bench_int_search_tail();
    bench_int_search_miss();

    printf("\n-- search string (100k nodes) --\n");
    bench_str_search_tail();

    printf("\n-- reverse (100k nodes) --\n");
    bench_int_reverse();
    bench_str_reverse();

    printf("\n-- free (100k nodes) --\n");
    bench_free_int();
    bench_free_str();

    printf("\ndone\n");
    return 0;
}