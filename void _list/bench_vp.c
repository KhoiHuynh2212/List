
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "list_vp.h"

#define N          10000
#define NANO       1000000000L

/* ─────────────────────────────────────────
   COMPARATORS
   ───────────────────────────────────────── */
int cmp_int(void *a, void *b)    { return *(int*)a == *(int*)b; }
int cmp_string(void *a, void *b) { return strcmp((char*)a, (char*)b) == 0; }

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
   BENCHMARKS
   ───────────────────────────────────────── */
void bench_insertEnd() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        long t0 = now_ns();
        insertEnd(&head, &i, sizeof(int));
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("insertEnd", times, N);
}

void bench_insertHead() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        long t0 = now_ns();
        insertHead(&head, &i, sizeof(int));
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("insertHead", times, N);
}

void bench_deleteNode_hit() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        int vals[10];
        for (int j = 0; j < 10; j++) {
            vals[j] = j;
            insertEnd(&head, &vals[j], sizeof(int));
        }
        int target = 5;  // middle hit
        long t0 = now_ns();
        deleteNode(&head, &target, cmp_int);
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("deleteNode (hit)", times, N);
}

void bench_deleteNode_miss() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        int vals[10];
        for (int j = 0; j < 10; j++) {
            vals[j] = j;
            insertEnd(&head, &vals[j], sizeof(int));
        }
        int target = 99;  // miss
        long t0 = now_ns();
        deleteNode(&head, &target, cmp_int);
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("deleteNode (miss)", times, N);
}

void bench_searchNode_head() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        int vals[10];
        for (int j = 0; j < 10; j++) {
            vals[j] = j;
            insertEnd(&head, &vals[j], sizeof(int));
        }
        int target = 0;  // head hit
        long t0 = now_ns();
        searchNode(head, &target, cmp_int);
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("searchNode (head)", times, N);
}

void bench_searchNode_tail() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        int vals[10];
        for (int j = 0; j < 10; j++) {
            vals[j] = j;
            insertEnd(&head, &vals[j], sizeof(int));
        }
        int target = 9;  // tail hit
        long t0 = now_ns();
        searchNode(head, &target, cmp_int);
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("searchNode (tail)", times, N);
}

void bench_searchNode_miss() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        int vals[10];
        for (int j = 0; j < 10; j++) {
            vals[j] = j;
            insertEnd(&head, &vals[j], sizeof(int));
        }
        int target = 99;  // miss
        long t0 = now_ns();
        searchNode(head, &target, cmp_int);
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("searchNode (miss)", times, N);
}

void bench_reverse() {
    long times[N];
    for (int i = 0; i < N; i++) {
        Node *head = NULL;
        int vals[10];
        for (int j = 0; j < 10; j++) {
            vals[j] = j;
            insertEnd(&head, &vals[j], sizeof(int));
        }
        long t0 = now_ns();
        reverse(&head);
        times[i] = now_ns() - t0;
        free_list(&head);
    }
    print_stats("reverse (10 nodes)", times, N);
}

/* ─────────────────────────────────────────
   MAIN
   ───────────────────────────────────────── */
int main() {
    printf("=== void* list benchmark (N=%d) ===\n\n", N);
    bench_insertEnd();
    bench_insertHead();
    bench_deleteNode_hit();
    bench_deleteNode_miss();
    bench_searchNode_head();
    bench_searchNode_tail();
    bench_searchNode_miss();
    bench_reverse();
    printf("\ndone\n");
    return 0;
}