CONTEXT: Systems programming internship prep, Summer 2026 → Winter internship goal.

## Current Project: libds (C data structures library)
Phase 1 of 4. Building 3 implementations of a linked list to benchmark against each other:
1. Tagged union ✓ → 2. void* ✓ → 3. Macro-generic + arena allocator ← YOU ARE HERE

---

## Tagged Union List — DONE
File: tagged_list/list.h + list.c
- insertIntNode, insertStringNode
- display (switch inside loop, handles mixed types)
- free_list (checks kind == STRING before freeing node_string)
- deleteIntNode, deleteStringNode (doubly linked, curr->prev rewiring)
- searchInt, searchString (return Node*, NULL on miss)
- test.c: full correctness suite, all pass, 0 Valgrind errors
- bench.c: p50/p95/p99 harness

### Tagged Union Benchmark Results (N=10000, -O2)
```
insertIntNode              p50=18ns  p95=19ns  p99=19ns
insertStringNode (short)   p50=22ns  p95=22ns  p99=23ns
insertStringNode (long)    p50=29ns  p95=31ns  p99=31ns
deleteIntNode (head hit)   p50=25ns  p95=28ns  p99=29ns
deleteIntNode (tail hit)   p50=30ns  p95=31ns  p99=32ns
deleteIntNode (miss)       p50=22ns  p95=23ns  p99=23ns
deleteStringNode (hit)     p50=22ns  p95=23ns  p99=23ns
deleteStringNode (miss)    p50=18ns  p95=20ns  p99=20ns
searchInt (head hit)       p50=14ns  p95=16ns  p99=16ns
searchInt (tail hit)       p50=27ns  p95=28ns  p99=29ns
searchInt (miss)           p50=24ns  p95=25ns  p99=26ns
searchString (tail hit)    p50=19ns  p95=19ns  p99=20ns
searchString (miss)        p50=17ns  p95=17ns  p99=18ns
```

---

## void* List — DONE
File: void_list/list_vp.h + list_vp.c
- insertEnd, insertHead
- deleteNode (comparator callback: int (*cmp)(void*, void*))
- searchNode (comparator callback)
- reverse (track last node visited for new head)
- free_list (always free(data), no kind check needed)
- test_vp.c: full correctness suite, all pass, 0 Valgrind errors
- bench_vp.c: same p50/p95/p99 harness

### void* Benchmark Results (N=10000, -O2)
```
insertEnd                  p50=21ns  p95=22ns  p99=22ns
insertHead                 p50=35ns  p95=37ns  p99=37ns
deleteNode (hit)           p50=67ns  p95=77ns  p99=78ns
deleteNode (miss)          p50=23ns  p95=45ns  p99=47ns
searchNode (head)          p50=16ns  p95=20ns  p99=20ns
searchNode (tail)          p50=18ns  p95=19ns  p99=20ns
searchNode (miss)          p50=22ns  p95=30ns  p99=37ns
reverse (10 nodes)         p50=30ns  p95=35ns  p99=38ns
```

### void* vs Tagged Union — Key Findings
- deleteNode hit is 2-3x slower (67ns vs 25-30ns) — function pointer indirect call overhead
- deleteNode miss p99 spikes to 47ns vs tagged 23ns — 10 cmp callback calls stack up
- searchNode tail is faster (18ns vs 27ns) — no kind check before compare
- insertEnd comparable (21ns vs 18ns) — same malloc cost
- insertHead 35ns — extra prev pointer write causes cache miss

### Design Tradeoffs Learned
- void* genericity costs ~2x on delete due to function pointer indirection
- Indirect call hurts branch predictor — CPU can't predict function pointer target
- Tagged union pays kind check on every search node — extra branch
- void* free_list simpler — always free(data), no type check needed
- Linus Node** technique compensates for no prev in singly linked list
- Doubly linked list: curr->prev replaces need for Node** traversal

---

## Tooling
- Makefile at root: make test_tagged, make valgrind_tagged, make bench_tagged, same for _vp
- Return convention: 0 on hit, -1 on miss
- _POSIX_C_SOURCE 199309L needed for CLOCK_MONOTONIC on Linux

---

## Testing Knowledge Gained
- CHECK macro with do{}while(0) to avoid dangling else bug
- Four test categories: happy path, boundaries, miss case, after-effects
- Valgrind workflow: compile with -g → valgrind --leak-check=full --track-origins=yes
- free_list(&head) required at end of every test or Valgrind reports leak
- Makefile indentation must be tabs not spaces

---

## Phase 1 Sequence
1. Tagged union list ✓
2. Benchmark tagged list ✓
3. void* list ✓
4. Benchmark void* list ✓
5. Macro-generic list + arena allocator ← NEXT
6. Design doc with measured comparisons

## Next Immediate Action
Start macro-generic list — same API, eliminate function pointer overhead via macros,
add arena allocator, benchmark all three side by side.

## Phase 2: kv store (epoll, robin hood hashmap, WAL, arena allocator)
## Target: Winter 2026 internship, systems/infra teams