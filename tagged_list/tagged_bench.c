#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tagged_list.h"

#define N      1000
#define BATCH  1000
#define LIST_N 100000
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
    printf("  %-42s  p50=%8ld ns   p95=%8ld ns   p99=%8ld ns\n",
           label, pct(times, n, 0.50), pct(times, n, 0.95), pct(times, n, 0.99));
}


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

/* Append to a stable 100-node list — steady-state tail-walk cost.
   Keep length fixed by pairing each insert with a delete. */
void bench_insertInt_tail100(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 100; j++) insertIntNode(&head, j);

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            insertIntNode(&head, 999);
            deleteIntNode(&head, 999);
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


/* Head delete — O(1), allocation-dominated.  Small list is intentional. */
void bench_deleteInt_head(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 10; j++) insertIntNode(&head, j);

        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++) {
            deleteIntNode(&head, 0);
            insertIntNode(&head, 0);   /* restore head value; goes to tail but cost matches */
        }
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("deleteInt head hit (10 nodes)", times, N);
}

/*
 * Tail delete at 100k nodes — exercises the full O(n) walk to find the
 * last element, comparable to intrusive bench_search_hit (tail, 100k).
 * Rebuild the list each outer iteration so the delete target stays at tail.
 */
void bench_deleteInt_tail(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < LIST_N; j++) insertIntNode(&head, j);

        /* Time a single delete-tail.  BATCH=1 here: rebuilding 100k nodes
           per sample is too expensive; one timed op per sample is sufficient
           because LIST_N >> clock resolution. */
        long t0 = now_ns();
        deleteIntNode(&head, LIST_N - 1);   /* target is last inserted value */
        times[i] = now_ns() - t0;

        free_list(&head);
    }
    print_stats("deleteInt tail hit (100k nodes)", times, N);
}

/*
 * Miss case — walks all LIST_N nodes before returning 0.
 * List is built once outside the timed BATCH loop and reused,
 * so we measure pure traversal cost without alloc noise.
 */
void bench_deleteInt_miss(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < LIST_N; j++) insertIntNode(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            deleteIntNode(&head, -1);   /* guaranteed miss */
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("deleteInt miss (100k nodes)", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   DELETE STRING
   Same size rationale as delete int:
   - head/middle: small list, strcmp is the cost being measured
   - miss: LIST_N, full strcmp-per-node scan
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
            insertStringNode(&head, "apple");
        }
        times[i] = (now_ns() - t0) / BATCH;
        free_list(&head);
    }
    print_stats("deleteString head hit (3 nodes)", times, N);
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
    print_stats("deleteString middle hit (3 nodes)", times, N);
}

/*
 * String delete miss at LIST_N.  Build a list of "nodeXXXXX" strings,
 * search for a value that cannot exist.
 */
void bench_deleteString_miss(void) {
    long times[N];
    Node *head = NULL;
    char buf[16];
    for (int j = 0; j < LIST_N; j++) {
        snprintf(buf, sizeof(buf), "n%d", j);
        insertStringNode(&head, buf);
    }

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            deleteStringNode(&head, "zzz_miss");
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("deleteString miss (100k nodes)", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   SEARCH INT — LIST_N for all cases; comparable to intrusive bench.
   List built once, reused across all N samples.
   ════════════════════════════════════════════════════════════════════ */

void bench_searchInt_head(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < LIST_N; j++) insertIntNode(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchInt(head, 0);   /* head — exits on first node */
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchInt head hit (100k nodes)", times, N);
    free_list(&head);
}

void bench_searchInt_tail(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < LIST_N; j++) insertIntNode(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchInt(head, LIST_N - 1);   /* tail — full walk */
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchInt tail hit (100k nodes)", times, N);
    free_list(&head);
}

void bench_searchInt_miss(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < LIST_N; j++) insertIntNode(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchInt(head, -1);   /* miss — full walk */
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchInt miss (100k nodes)", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   SEARCH STRING — same scale as int search.
   ════════════════════════════════════════════════════════════════════ */

void bench_searchString_head(void) {
    long times[N];
    Node *head = NULL;
    char buf[16];
    for (int j = 0; j < LIST_N; j++) {
        snprintf(buf, sizeof(buf), "n%d", j);
        insertStringNode(&head, buf);
    }

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchString(head, "n0");   /* head */
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchString head hit (100k nodes)", times, N);
    free_list(&head);
}

void bench_searchString_tail(void) {
    long times[N];
    Node *head = NULL;
    char buf[16];
    char tail_val[16];
    for (int j = 0; j < LIST_N; j++) {
        snprintf(buf, sizeof(buf), "n%d", j);
        insertStringNode(&head, buf);
    }
    snprintf(tail_val, sizeof(tail_val), "n%d", LIST_N - 1);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchString(head, tail_val);   /* tail — worst case */
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchString tail hit (100k nodes)", times, N);
    free_list(&head);
}

void bench_searchString_miss(void) {
    long times[N];
    Node *head = NULL;
    char buf[16];
    for (int j = 0; j < LIST_N; j++) {
        snprintf(buf, sizeof(buf), "n%d", j);
        insertStringNode(&head, buf);
    }

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        for (int j = 0; j < BATCH; j++)
            searchString(head, "zzz_miss");
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchString miss (100k nodes)", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   SEARCH FLOAT / DOUBLE — small list; these types don't appear in the
   other three implementations so no cross-implementation comparison is
   needed.  Kept to document the tagged union's type-dispatch overhead.
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
            searchFloat(head, 3.0f);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchFloat tail hit (3 nodes)", times, N);
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
            searchDouble(head, 3.3);
        times[i] = (now_ns() - t0) / BATCH;
    }
    print_stats("searchDouble tail hit (3 nodes)", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   REVERSE — LIST_N nodes, time one reverse per sample.
   Even number of samples cancels out, so the list order alternates
   but the node count stays constant.
   ════════════════════════════════════════════════════════════════════ */

void bench_reverse(void) {
    long times[N];
    Node *head = NULL;
    for (int j = 0; j < LIST_N; j++) insertIntNode(&head, j);

    for (int i = 0; i < N; i++) {
        long t0 = now_ns();
        reverse(&head);
        times[i] = now_ns() - t0;   /* one reverse per sample; no BATCH divide */
    }
    print_stats("reverse (100k nodes)", times, N);
    free_list(&head);
}

/* ════════════════════════════════════════════════════════════════════
   FREE — LIST_N nodes, rebuild each sample, time one free_list call.
   No BATCH loop: we can't reuse the list after freeing it, and one
   free of LIST_N nodes is well above clock resolution.
   ════════════════════════════════════════════════════════════════════ */

void bench_free_int(void) {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < LIST_N; j++) insertIntNode(&head, j);
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
            insertStringNode(&head, buf);
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
    printf("=== tagged union list benchmark"
           "  (N=%d samples, BATCH=%d, LIST_N=%d) ===\n\n",
           N, BATCH, LIST_N);

    printf("-- insert (allocation-dominated, small list) --\n");
    bench_insertInt_empty();
    bench_insertInt_tail100();
    bench_insertString_short();
    bench_insertString_long();
    bench_insertFloat();
    bench_insertDouble();

    printf("\n-- delete int --\n");
    bench_deleteInt_head();         /* O(1) find, small list               */
    bench_deleteInt_tail();         /* O(n) find, 100k nodes, 1 op/sample  */
    bench_deleteInt_miss();         /* O(n) full scan, 100k nodes           */

    printf("\n-- delete string --\n");
    bench_deleteString_head();
    bench_deleteString_middle();
    bench_deleteString_miss();      /* 100k nodes */

    printf("\n-- search int (100k nodes) --\n");
    bench_searchInt_head();
    bench_searchInt_tail();
    bench_searchInt_miss();

    printf("\n-- search string (100k nodes) --\n");
    bench_searchString_head();
    bench_searchString_tail();
    bench_searchString_miss();

    printf("\n-- search float / double (tagged-union specific, 3 nodes) --\n");
    bench_searchFloat();
    bench_searchDouble();

    printf("\n-- reverse (100k nodes) --\n");
    bench_reverse();

    printf("\n-- free (100k nodes) --\n");
    bench_free_int();
    bench_free_string();

    printf("\ndone\n");
    return 0;
}