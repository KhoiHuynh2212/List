#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "list_vp.h"


#define N      1000
#define BATCH  1000
#define LIST_N 100000
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
    printf("  %-42s  p50=%8ld ns   p95=%8ld ns   p99=%8ld ns\n",
           label, pct(times, n, 0.50), pct(times, n, 0.95), pct(times, n, 0.99));
}

/* ════════════════════════════════════════════════════════════════════
   INSERT — allocation-dominated; small list is intentional.
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

/* Append to stable 100-node list, keep length fixed. */
void bench_insertEnd_tail100(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 100; j++) insertEnd(&head, &j, sizeof(int));

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            insertEnd(&head, &j, sizeof(int));
            deleteNode(&head, &j, cmp_int);
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
   - head: O(1), small list, isolates pointer-unlink + free cost
   - tail: O(n), LIST_N, matches intrusive search-hit (tail)
   - miss: O(n), LIST_N, matches intrusive search-miss
   ════════════════════════════════════════════════════════════════════ */

void bench_deleteNode_head(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        int zero = 0;
        for (int j = 0; j < 10; j++) insertEnd(&head, &j, sizeof(int));

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            deleteNode(&head, &zero, cmp_int);
            insertHead(&head, &zero, sizeof(int));
        }
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("deleteNode head hit (10 nodes)", times, N);
}

/*
 * Tail delete on LIST_N nodes — one op per sample (rebuilding 100k
 * nodes per op would swamp the signal).
 */
void bench_deleteNode_tail(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        int target = LIST_N - 1;
        for (int j = 0; j < LIST_N; j++) insertEnd(&head, &j, sizeof(int));

        long t0 = now_ns();
        deleteNode(&head, &target, cmp_int);
        times[i] = now_ns() - t0;

        free_list(&head);
    }
    print_stats("deleteNode tail hit (100k nodes)", times, N);
}

/* Miss — full scan, list reused across all N samples. */
void bench_deleteNode_miss(void) {
    long times[N];
    Node *head = NULL;
    int miss = -1;
    for (int j = 0; j < LIST_N; j++) insertEnd(&head, &j, sizeof(int));

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            deleteNode(&head, &miss, cmp_int);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("deleteNode miss (100k nodes)", times, N);
    free_list(&head);
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
    print_stats("deleteNode string hit (3 nodes)", times, N);
}

/*
 * String delete miss at LIST_N.  Build a list of short unique strings,
 * search for a value that cannot exist.
 */
void bench_deleteNode_string_miss(void) {
    long times[N];
    Node *head = NULL;
    char buf[16];
    for (int j = 0; j < LIST_N; j++) {
        snprintf(buf, sizeof(buf), "n%d", j);
        insertEnd(&head, buf, strlen(buf) + 1);
    }

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            deleteNode(&head, "zzz_miss", cmp_string);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("deleteNode string miss (100k nodes)", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   SEARCH — LIST_N for all cases; list built once, reused.
   ════════════════════════════════════════════════════════════════════ */

void bench_searchNode_head(void) {
    long times[N];
    Node *head = NULL;
    int zero = 0;
    for (int j = 0; j < LIST_N; j++) insertEnd(&head, &j, sizeof(int));

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchNode(head, &zero, cmp_int);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchNode head hit (100k nodes)", times, N);
    free_list(&head);
}

void bench_searchNode_tail(void) {
    long times[N];
    Node *head = NULL;
    int tail_val = LIST_N - 1;
    for (int j = 0; j < LIST_N; j++) insertEnd(&head, &j, sizeof(int));

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchNode(head, &tail_val, cmp_int);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchNode tail hit (100k nodes)", times, N);
    free_list(&head);
}

void bench_searchNode_miss(void) {
    long times[N];
    Node *head = NULL;
    int miss = -1;
    for (int j = 0; j < LIST_N; j++) insertEnd(&head, &j, sizeof(int));

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchNode(head, &miss, cmp_int);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchNode miss (100k nodes)", times, N);
    free_list(&head);
}

void bench_searchNode_string_tail(void) {
    long times[N];
    Node *head = NULL;
    char buf[16];
    char tail_val[16];
    for (int j = 0; j < LIST_N; j++) {
        snprintf(buf, sizeof(buf), "n%d", j);
        insertEnd(&head, buf, strlen(buf) + 1);
    }
    snprintf(tail_val, sizeof(tail_val), "n%d", LIST_N - 1);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchNode(head, tail_val, cmp_string);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchNode string tail (100k nodes)", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   REVERSE — LIST_N nodes, one reverse per sample.
   ════════════════════════════════════════════════════════════════════ */

void bench_reverse(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < LIST_N; j++) insertEnd(&head, &j, sizeof(int));

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        reverse(&head);
        times[i] = now_ns() - t0;
    }
    print_stats("reverse (100k nodes)", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   FREE — LIST_N nodes, rebuild each sample, one free_list per sample.
   ════════════════════════════════════════════════════════════════════ */

void bench_free_int(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < LIST_N; j++) insertEnd(&head, &j, sizeof(int));
        long t0 = now_ns();
        free_list(&head);
        times[i] = now_ns() - t0;
    }
    print_stats("free_list int (100k nodes)", times, N);
}

void bench_free_string(void) {
    long times[N];
    char buf[16];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < LIST_N; j++) {
            snprintf(buf, sizeof(buf), "n%d", j);
            insertEnd(&head, buf, strlen(buf) + 1);
        }
        long t0 = now_ns();
        free_list(&head);
        times[i] = now_ns() - t0;
    }
    print_stats("free_list string (100k nodes)", times, N);
}

/* ════════════════════════════════════════════════════════════════════
   MAIN
   ════════════════════════════════════════════════════════════════════ */
int main(void) {
    printf("=== void* list benchmark"
           "  (N=%d samples, BATCH=%d, LIST_N=%d) ===\n\n",
           N, BATCH, LIST_N);

    printf("-- insert (allocation-dominated, small list) --\n");
    bench_insertEnd_empty();
    bench_insertEnd_tail100();
    bench_insertHead_empty();
    bench_insertEnd_string_short();
    bench_insertEnd_string_long();

    printf("\n-- delete int --\n");
    bench_deleteNode_head();        /* O(1), small list  */
    bench_deleteNode_tail();        /* O(n), 100k nodes  */
    bench_deleteNode_miss();        /* O(n), 100k nodes  */

    printf("\n-- delete string --\n");
    bench_deleteNode_string_hit();
    bench_deleteNode_string_miss(); /* 100k nodes */

    printf("\n-- search int (100k nodes) --\n");
    bench_searchNode_head();
    bench_searchNode_tail();
    bench_searchNode_miss();

    printf("\n-- search string (100k nodes) --\n");
    bench_searchNode_string_tail();

    printf("\n-- reverse (100k nodes) --\n");
    bench_reverse();

    printf("\n-- free (100k nodes) --\n");
    bench_free_int();
    bench_free_string();

    printf("\ndone\n");
    return 0;
}