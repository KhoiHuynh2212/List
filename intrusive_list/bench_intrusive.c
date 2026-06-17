// bench_intrusive_v2.c
//
// Side-by-side benchmark: the intrusive list (ilist.h) backed by two
// allocation strategies —
//   (A) arena    — bump allocator, one malloc upfront, no per-node free
//   (B) malloc   — classic malloc-per-node, matching tagged_list/void_list's
//                  allocation pattern, so the *list* implementation is held
//                  constant and only the *allocator* varies
//
// This isolates the allocator's contribution to your p50/p95/p99 numbers.
// If insert is faster under arena but search/swap/splice are identical,
// that confirms those operations are allocator-independent (pure pointer
// topology), which they should be — only insert/destroy touch the allocator.
//
// Also includes the standalone arena_grow isolation benchmark from before.
//
// gcc -O2 -std=c11 bench_intrusive_v2.c -o bench_intrusive_v2

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "arena.h"
#include "instrusive_list.h"

#define N 100000
#define RUNS 1000

typedef struct {
    int value;
    list link;
} Node;

#define ALIGNED_NODE_SIZE \
    ((sizeof(Node) + ARENA_ALIGN - 1) & ~(ARENA_ALIGN - 1))

static inline Node* entry(list* n) {
    return list_entry(n, Node, link);
}

// ─── timing helpers ───

static inline uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static int cmp_u64(const void* a, const void* b) {
    uint64_t x = *(const uint64_t*)a, y = *(const uint64_t*)b;
    return (x > y) - (x < y);
}

static void print_percentiles(const char* label, uint64_t* samples, size_t n) {
    qsort(samples, n, sizeof(uint64_t), cmp_u64);
    uint64_t p50 = samples[(size_t)(n * 0.50)];
    uint64_t p95 = samples[(size_t)(n * 0.95)];
    uint64_t p99 = samples[(size_t)(n * 0.99)];
    printf("%-28s p50=%8llu ns   p95=%8llu ns   p99=%8llu ns\n",
           label, (unsigned long long)p50, (unsigned long long)p95, (unsigned long long)p99);
}

// ─── malloc-backed node helpers ───
// Mirrors what tagged_list/void_list did: one malloc per node, one free
// per node. Used only to build the comparison lists below — destruction
// loop frees each node individually since there's no arena to bulk-free.

static Node* node_alloc_malloc(void) {
    Node* n = malloc(sizeof(Node));
    return n;
}

static void free_all_malloc(list* head) {
    list* pos;
    list* tmp;
    list_for_each_safe(pos, tmp, head) {
        list_unlink(pos);
        free(entry(pos));
    }
}

// =============================================================
// ARENA-BACKED BENCHMARKS
// =============================================================

static void bench_insert_arena(uint64_t* samples) {
    for (int r = 0; r < RUNS; r++) {
        arena a = arena_create((size_t)N * ALIGNED_NODE_SIZE + 4096);
        list head = LIST_INIT(head);

        uint64_t t0 = now_ns();
        for (int i = 0; i < N; i++) {
            Node* nd = arena_alloc(&a, sizeof(Node));
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&head, &nd->link);
        }
        uint64_t t1 = now_ns();

        samples[r] = (t1 - t0) / N;
        arena_destroy(&a);
    }
}

static void bench_search_hit_arena(uint64_t* samples) {
    for (int r = 0; r < RUNS; r++) {
        arena a = arena_create((size_t)N * ALIGNED_NODE_SIZE + 4096);
        list head = LIST_INIT(head);
        for (int i = 0; i < N; i++) {
            Node* nd = arena_alloc(&a, sizeof(Node));
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&head, &nd->link);
        }
        int target = N - 1;

        uint64_t t0 = now_ns();
        Node* found = NULL;
        list* pos;
        list_for_each(pos, &head) {
            Node* nd = entry(pos);
            if (nd->value == target) { found = nd; break; }
        }
        uint64_t t1 = now_ns();

        (void)found;
        samples[r] = (t1 - t0);
        arena_destroy(&a);
    }
}

static void bench_search_miss_arena(uint64_t* samples) {
    for (int r = 0; r < RUNS; r++) {
        arena a = arena_create((size_t)N * ALIGNED_NODE_SIZE + 4096);
        list head = LIST_INIT(head);
        for (int i = 0; i < N; i++) {
            Node* nd = arena_alloc(&a, sizeof(Node));
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&head, &nd->link);
        }
        int target = -1;

        uint64_t t0 = now_ns();
        Node* found = NULL;
        list* pos;
        list_for_each(pos, &head) {
            Node* nd = entry(pos);
            if (nd->value == target) { found = nd; break; }
        }
        uint64_t t1 = now_ns();

        (void)found;
        samples[r] = (t1 - t0);
        arena_destroy(&a);
    }
}

static void bench_unlink_head_arena(uint64_t* samples) {
    for (int r = 0; r < RUNS; r++) {
        arena a = arena_create((size_t)N * ALIGNED_NODE_SIZE + 4096);
        list head = LIST_INIT(head);
        for (int i = 0; i < N; i++) {
            Node* nd = arena_alloc(&a, sizeof(Node));
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&head, &nd->link);
        }

        uint64_t t0 = now_ns();
        list* popped = list_pop_front(&head);
        uint64_t t1 = now_ns();

        (void)popped;
        samples[r] = (t1 - t0);
        arena_destroy(&a);
    }
}

static void bench_swap_arena(uint64_t* samples) {
    for (int r = 0; r < RUNS; r++) {
        arena a = arena_create((size_t)N * ALIGNED_NODE_SIZE + 4096);
        list head = LIST_INIT(head);
        Node* nodes[N];
        for (int i = 0; i < N; i++) {
            Node* nd = arena_alloc(&a, sizeof(Node));
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&head, &nd->link);
            nodes[i] = nd;
        }

        uint64_t t0 = now_ns();
        list_swap(&nodes[10]->link, &nodes[N - 10]->link);
        uint64_t t1 = now_ns();

        samples[r] = (t1 - t0);
        arena_destroy(&a);
    }
}

static void bench_splice_arena(uint64_t* samples) {
    for (int r = 0; r < RUNS; r++) {
        arena a = arena_create((size_t)N * ALIGNED_NODE_SIZE + 4096);
        list target = LIST_INIT(target);
        list source = LIST_INIT(source);
        for (int i = 0; i < N / 2; i++) {
            Node* nd = arena_alloc(&a, sizeof(Node));
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&target, &nd->link);
        }
        for (int i = 0; i < N / 2; i++) {
            Node* nd = arena_alloc(&a, sizeof(Node));
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&source, &nd->link);
        }

        uint64_t t0 = now_ns();
        list_splice(&target, &source);
        uint64_t t1 = now_ns();

        samples[r] = (t1 - t0);
        arena_destroy(&a);
    }
}

static void bench_length_arena(uint64_t* samples) {
    // list_length is O(n) — this benchmarks the full walk-and-count cost.
    for (int r = 0; r < RUNS; r++) {
        arena a = arena_create((size_t)N * ALIGNED_NODE_SIZE + 4096);
        list head = LIST_INIT(head);
        for (int i = 0; i < N; i++) {
            Node* nd = arena_alloc(&a, sizeof(Node));
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&head, &nd->link);
        }

        uint64_t t0 = now_ns();
        size_t len = list_length(&head);
        uint64_t t1 = now_ns();

        (void)len;
        samples[r] = (t1 - t0);
        arena_destroy(&a);
    }
}

// =============================================================
// MALLOC-BACKED BENCHMARKS (same list ops, classic per-node alloc)
// =============================================================

static void bench_insert_malloc(uint64_t* samples) {
    for (int r = 0; r < RUNS; r++) {
        list head = LIST_INIT(head);

        uint64_t t0 = now_ns();
        for (int i = 0; i < N; i++) {
            Node* nd = node_alloc_malloc();
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&head, &nd->link);
        }
        uint64_t t1 = now_ns();

        samples[r] = (t1 - t0) / N;
        free_all_malloc(&head);
    }
}

static void bench_search_hit_malloc(uint64_t* samples) {
    for (int r = 0; r < RUNS; r++) {
        list head = LIST_INIT(head);
        for (int i = 0; i < N; i++) {
            Node* nd = node_alloc_malloc();
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&head, &nd->link);
        }
        int target = N - 1;

        uint64_t t0 = now_ns();
        Node* found = NULL;
        list* pos;
        list_for_each(pos, &head) {
            Node* nd = entry(pos);
            if (nd->value == target) { found = nd; break; }
        }
        uint64_t t1 = now_ns();

        (void)found;
        samples[r] = (t1 - t0);
        free_all_malloc(&head);
    }
}

static void bench_search_miss_malloc(uint64_t* samples) {
    for (int r = 0; r < RUNS; r++) {
        list head = LIST_INIT(head);
        for (int i = 0; i < N; i++) {
            Node* nd = node_alloc_malloc();
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&head, &nd->link);
        }
        int target = -1;

        uint64_t t0 = now_ns();
        Node* found = NULL;
        list* pos;
        list_for_each(pos, &head) {
            Node* nd = entry(pos);
            if (nd->value == target) { found = nd; break; }
        }
        uint64_t t1 = now_ns();

        (void)found;
        samples[r] = (t1 - t0);
        free_all_malloc(&head);
    }
}

static void bench_unlink_head_malloc(uint64_t* samples) {
    for (int r = 0; r < RUNS; r++) {
        list head = LIST_INIT(head);
        for (int i = 0; i < N; i++) {
            Node* nd = node_alloc_malloc();
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&head, &nd->link);
        }

        uint64_t t0 = now_ns();
        list* popped = list_pop_front(&head);
        uint64_t t1 = now_ns();

        free(entry(popped));   // malloc path: must free what was unlinked
        samples[r] = (t1 - t0);
        free_all_malloc(&head);
    }
}

static void bench_swap_malloc(uint64_t* samples) {
    for (int r = 0; r < RUNS; r++) {
        list head = LIST_INIT(head);
        Node* nodes[N];
        for (int i = 0; i < N; i++) {
            Node* nd = node_alloc_malloc();
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&head, &nd->link);
            nodes[i] = nd;
        }

        uint64_t t0 = now_ns();
        list_swap(&nodes[10]->link, &nodes[N - 10]->link);
        uint64_t t1 = now_ns();

        samples[r] = (t1 - t0);
        free_all_malloc(&head);
    }
}

static void bench_splice_malloc(uint64_t* samples) {
    for (int r = 0; r < RUNS; r++) {
        list target = LIST_INIT(target);
        list source = LIST_INIT(source);
        for (int i = 0; i < N / 2; i++) {
            Node* nd = node_alloc_malloc();
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&target, &nd->link);
        }
        for (int i = 0; i < N / 2; i++) {
            Node* nd = node_alloc_malloc();
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&source, &nd->link);
        }

        uint64_t t0 = now_ns();
        list_splice(&target, &source);
        uint64_t t1 = now_ns();

        samples[r] = (t1 - t0);
        free_all_malloc(&target);   // source was merged into target — one free pass
    }
}

static void bench_length_malloc(uint64_t* samples) {
    for (int r = 0; r < RUNS; r++) {
        list head = LIST_INIT(head);
        for (int i = 0; i < N; i++) {
            Node* nd = node_alloc_malloc();
            nd->value = i;
            list_init(&nd->link);
            list_add_tail(&head, &nd->link);
        }

        uint64_t t0 = now_ns();
        size_t len = list_length(&head);
        uint64_t t1 = now_ns();

        (void)len;
        samples[r] = (t1 - t0);
        free_all_malloc(&head);
    }
}

// =============================================================
// arena_grow isolation (unchanged from earlier session)
// =============================================================

static void bench_arena_grow(uint64_t* samples) {
    size_t undersized = (size_t)(N / 100) * ALIGNED_NODE_SIZE;
    if (undersized < ALIGNED_NODE_SIZE) undersized = ALIGNED_NODE_SIZE;

    for (int r = 0; r < RUNS; r++) {
        arena a = arena_create(undersized);

        uint64_t t0 = now_ns();
        for (int i = 0; i < N; i++) {
            Node* nd = arena_alloc(&a, sizeof(Node));
            nd->value = i;
        }
        uint64_t t1 = now_ns();

        samples[r] = (t1 - t0) / N;
        arena_destroy(&a);
    }
}

int main(void) {
    uint64_t* samples = malloc(sizeof(uint64_t) * RUNS);
    if (!samples) { fprintf(stderr, "malloc failed\n"); return 1; }

    printf("Intrusive List: Arena vs Malloc (N=%d nodes, %d runs)\n\n", N, RUNS);

    printf("=== ARENA-BACKED ===\n");
    bench_insert_arena(samples);       print_percentiles("insert (tail)", samples, RUNS);
    bench_search_hit_arena(samples);   print_percentiles("search hit (tail)", samples, RUNS);
    bench_search_miss_arena(samples);  print_percentiles("search miss (full scan)", samples, RUNS);
    bench_unlink_head_arena(samples);  print_percentiles("unlink head", samples, RUNS);
    bench_swap_arena(samples);         print_percentiles("swap (far apart)", samples, RUNS);
    bench_splice_arena(samples);       print_percentiles("splice (50k+50k)", samples, RUNS);
    bench_length_arena(samples);       print_percentiles("list_length (full walk)", samples, RUNS);

    printf("\n=== MALLOC-BACKED ===\n");
    bench_insert_malloc(samples);      print_percentiles("insert (tail)", samples, RUNS);
    bench_search_hit_malloc(samples);  print_percentiles("search hit (tail)", samples, RUNS);
    bench_search_miss_malloc(samples); print_percentiles("search miss (full scan)", samples, RUNS);
    bench_unlink_head_malloc(samples); print_percentiles("unlink head", samples, RUNS);
    bench_swap_malloc(samples);        print_percentiles("swap (far apart)", samples, RUNS);
    bench_splice_malloc(samples);      print_percentiles("splice (50k+50k)", samples, RUNS);
    bench_length_malloc(samples);      print_percentiles("list_length (full walk)", samples, RUNS);

    printf("\n=== ARENA GROWTH ISOLATION ===\n");
    bench_arena_grow(samples);         print_percentiles("insert w/ growth (undersized)", samples, RUNS);

    free(samples);
    return 0;
}