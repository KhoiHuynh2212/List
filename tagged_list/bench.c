#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include<list.h> 


#define N          10000
#define NANO       1000000000L 


/* ─────────────────────────────────────────
   TIMER
   ───────────────────────────────────────── */
static inline long now_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * NANO + ts.tv_nsec;
}

/* ─────────────────────────────────────────
   PERCENTILE HELPER
   ───────────────────────────────────────── */
static int cmp_long(const void *a, const void *b) {
    long x = *(long*)a, y = *(long*)b;
    return (x > y) - (x < y);
}

static long percentile(long *arr, int n, double p) {
    int idx = (int)(p * n);
    if (idx >= n) idx = n - 1;
    return arr[idx];
}

static void print_stats(const char *label, long *times, int n) {
    qsort(times, n, sizeof(long), cmp_long);
    printf("%-25s  p50=%4ld ns  p95=%4ld ns  p99=%4ld ns\n",
        label,
        percentile(times, n, 0.50),
        percentile(times, n, 0.95),
        percentile(times, n, 0.99));
} 


/* ─────────────────────────────────────────
   INSERT
   ───────────────────────────────────────── */
void bench_insertInt() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        long t0 = now_ns(); 
        insertIntNode(&head, i); 
        times[i] = now_ns() - t0;
        free_list(&head); 
    } 
    print_stats("insertIntNode", times, N); 
} 
 
void bench_insertString_short() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        char str[] = "hello";
        long t0 = now_ns();
        insertStringNode(&head, str);
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("insertStringNode (short)", times, N);
}
 
void bench_insertString_long() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        char str[] = "a very long string that is much bigger than a cache line and will stress the copy";
        long t0 = now_ns();
        insertStringNode(&head, str);
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("insertStringNode (long)", times, N);
}
 
/* ─────────────────────────────────────────
   DELETE INT
   ───────────────────────────────────────── */
void bench_deleteInt_hit_head() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 10; j++)
            insertIntNode(&head, j);
        long t0 = now_ns();
        deleteIntNode(&head, 0);       // hit at head
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("deleteIntNode (head hit)", times, N);
}
 
void bench_deleteInt_hit_tail() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 10; j++)
            insertIntNode(&head, j);
        long t0 = now_ns();
        deleteIntNode(&head, 9);       // hit at tail
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("deleteIntNode (tail hit)", times, N);
}
 
void bench_deleteInt_miss() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 10; j++)
            insertIntNode(&head, j);
        long t0 = now_ns();
        deleteIntNode(&head, 99);      // miss — full traversal
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("deleteIntNode (miss)", times, N);
}
 
/* ─────────────────────────────────────────
   DELETE STRING
   ───────────────────────────────────────── */
void bench_deleteString_hit() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        insertStringNode(&head, "apple");
        insertStringNode(&head, "banana");
        insertStringNode(&head, "cherry");
        long t0 = now_ns();
        deleteStringNode(&head, "banana");   // middle hit
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("deleteStringNode (hit)", times, N);
}
 
void bench_deleteString_miss() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        insertStringNode(&head, "apple");
        insertStringNode(&head, "banana");
        insertStringNode(&head, "cherry");
        long t0 = now_ns();
        deleteStringNode(&head, "mango");    // miss
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("deleteStringNode (miss)", times, N);
}
 
/* ─────────────────────────────────────────
   SEARCH INT
   ───────────────────────────────────────── */
void bench_searchInt_head() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 10; j++)
            insertIntNode(&head, j);
        long t0 = now_ns();
        searchInt(head, 0);            // hit at head
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("searchInt (head hit)", times, N);
}
 
void bench_searchInt_tail() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 10; j++)
            insertIntNode(&head, j);
        long t0 = now_ns();
        searchInt(head, 9);            // hit at tail
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("searchInt (tail hit)", times, N);
}
 
void bench_searchInt_miss() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        for (int j = 0; j < 10; j++)
            insertIntNode(&head, j);
        long t0 = now_ns();
        searchInt(head, 99);           // miss
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("searchInt (miss)", times, N);
}
 
/* ─────────────────────────────────────────
   SEARCH STRING
   ───────────────────────────────────────── */
void bench_searchString_hit() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        insertStringNode(&head, "apple");
        insertStringNode(&head, "banana");
        insertStringNode(&head, "cherry");
        long t0 = now_ns();
        searchString(head, "cherry");  // tail hit — worst case
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("searchString (tail hit)", times, N);
}
 
void bench_searchString_miss() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        insertStringNode(&head, "apple");
        insertStringNode(&head, "banana");
        insertStringNode(&head, "cherry");
        long t0 = now_ns();
        searchString(head, "mango");   // miss
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("searchString (miss)", times, N);
}
 
/* ─────────────────────────────────────────
   MAIN
   ───────────────────────────────────────── */
int main() {
    printf("=== tagged union list benchmark (N=%d) ===\n\n", N);
 
    printf("-- insert --\n");
    bench_insertInt();
    bench_insertString_short();
    bench_insertString_long();
 
    printf("\n-- delete --\n");
    bench_deleteInt_hit_head();
    bench_deleteInt_hit_tail();
    bench_deleteInt_miss();
    bench_deleteString_hit();
    bench_deleteString_miss();
 
    printf("\n-- search --\n");
    bench_searchInt_head();
    bench_searchInt_tail();
    bench_searchInt_miss();
    bench_searchString_hit();
    bench_searchString_miss();
 
    printf("\ndone\n");
    return 0;
}


