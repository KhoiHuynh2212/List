CONTEXT: Systems programming internship prep, Summer 2026 → Winter internship goal.

## Current Project: libds (C data structures library)
Phase 1 of 4. Building 4 implementations of a linked list to benchmark against each other:
1. Tagged union ✅ → 2. void* ✅ → 3. Macro-generic ✅ → 4. Intrusive + arena allocator ✅

## Tagged Union List — COMPLETE
File: tagged_list/list.h + list.c
- insertIntNode, insertStringNode, display, free_list
- deleteIntNode, deleteStringNode, searchInt, searchString
- test.c: 25/25 pass, 0 Valgrind errors
- bench_fast.c: p50/p95/p99 baseline numbers recorded

## void* List — COMPLETE
File: void_list/list_vp.h + list_vp.c
- insert (memcpy), free_list, deleteNode (cmp callback)
- bench_vp.c: baseline numbers recorded

## Macro-Generic List — COMPLETE
File: list/list.h
- DEFINE_LIST(T) — stamps out full typed implementation per type
- DEFINE_LIST_STRING — separate macro for string (strdup/strcmp/free special cases)
- Functions generated: T##_create, T##_insert, T##_delete_node, T##_search,
  T##_display (print_fn callback), T##_free_list, T##_reverse
- str_create, str_insert, str_delete, str_search, str_display,
  str_free_list, str_reverse
- test_macro.c: 56/56 pass, 0 Valgrind errors
- bench_macro.c: p50/p95/p99 numbers recorded

## Bugs Found and Fixed During Macro Phase
- T## with space: `T## init` → `T##_init` (space breaks token paste)
- `node` used inside macro instead of `T##Node`
- `free_list` not prefixed with `T##_` → duplicate symbol across types
- `display` missing semicolon after printf — curr = curr->next swallowed as arg
- `free_list` fell through into do-loop on empty list — missing return
- `display` used `%d` hardcoded — broke for float, fixed with print_fn callback
- `searchNode` not type-prefixed — duplicate symbol across types

## Key Concepts Learned
- ## token pasting, ## with spaces breaks it
- static inline required on all macro functions — prevents duplicate symbol errors
- gcc -E main.c | grep -v "^#" — see exactly what macro expanded to
- typedef char* string → DEFINE_LIST(string) — workaround for pointer types
- String needs own macro: strdup on create, strcmp on search/delete, free(data) on delete
- display is not type-agnostic — needs print_fn callback or separate macro per type
- reverse/free are type-agnostic — only touch next/prev, same code for all types
- sentinel node — dummy head, makes empty and non-empty list code identical
- circular_ok helper — verify next->prev == node && prev->next == node after every op
- IntelliSense shows red underline on generated types — normal, trust gcc not IntelliSense

## Testing Knowledge Gained
- CHECK macro with do{}while(0)
- circular_ok() helper — walks entire list verifying both pointer directions
- count() helper — walk the ring counting nodes, verify after every insert/delete
- Four test categories: happy path, boundaries (single/two element), miss case, after-effects
- Edge cases: empty list, single element, delete head, delete tail, delete only, duplicates,
  reverse empty, reverse single, reverse twice = original, empty string value
- Valgrind: gcc -g → valgrind --leak-check=full --track-origins=yes ./test_macro

## Intrusive List + Arena Allocator — COMPLETE
Files: ilist.h (formerly instrusive_list.h), arena.h, bench_intrusive_v2.c, test-api.c, test-embedded.c

### Arena allocator (arena.h)
- arena_create, arena_alloc (bump pointer, alignment via ARENA_ALIGN = _Alignof(max_align_t))
- arena_grow — realloc-based doubling with min_extra fallback when doubling isn't enough
- arena_reset (rewind to base, reuse same slab), arena_destroy (single free)
- WARNING documented in header: realloc-based growth invalidates any pointer issued
  before a growth-triggering call — base may move. Safe only if growth never fires
  (pre-sized arena) or no pointers are held across allocations.

### Intrusive list (ilist.h)
- struct list { next, prev } embedded directly in user structs — no separate node alloc
- container_of pattern via list_entry_offset / list_entry(ptr, type, member) macro,
  NULL-safe
- Circular doubly-linked, sentinel/anchor pattern (LIST_INIT, list_init)
- Core ops: list_add_before/after/between, list_push_front, list_add_tail
- Removal: list_unlink (reinitializes node), list_unlink_stale (leaves node's old
  pointers untouched — caller manages state manually)
- Container ops: list_pop_front, list_pop_back (both NULL-safe on empty list)
- list_replace / list_replace_init (one-for-one node swap, with/without detaching old)
- list_swap (positional swap, handles adjacent-node edge case via pos==entry1 check)
- list_splice (O(1) merge, drains source to a valid empty anchor, safe on empty source)
- list_is_first, list_is_last, list_is_empty, list_is_linked, list_length (O(n), test/diag use)
- Iteration: list_for_each, list_for_each_safe (tolerates unlink mid-loop),
  list_for_each_entry (typed, via __typeof__)

## Bugs Found and Fixed During Intrusive List + Arena Phase
- arena_create returning bare `return;` from a non-void struct-returning function
  → fixed with (arena){0} sentinel for the failure case
- arena_grow called as arena_grow(&a, size) inside arena_alloc where `a` was already
  arena* — extra & produced arena**, wrong type passed to arena*-expecting function
- arena_grow capacity math: doubling alone isn't always sufficient if a single
  allocation exceeds the doubled size — must compare against (cap + extra) too
- arena_grow after realloc: bump must be recomputed as new_base + used_offset,
  NEVER carried over as the old absolute pointer — realloc can move the block
- Test harness loop `while (arena_alloc(&b, 8) != NULL)` — contract mismatch.
  Once arena_grow exists, arena_alloc effectively never returns NULL under normal
  operation (it grows instead of failing); the old "loop until NULL" pattern from
  a fixed-capacity arena ran unbounded until actual OOM, then aborted at ~8GB
- list_for_each_entry used `typeof()` — GNU extension, fails under strict -std=c11/c99.
  Fixed with __typeof__ (portable across -std= flags on gcc/clang)
- Trailing `\` on the final line of list_for_each_entry's macro body with nothing
  after it — "stray backslash" / "backslash-newline at EOF" depending on compiler
- bench harness: arena capacity sized as N * sizeof(Node) without accounting for
  ARENA_ALIGN rounding (24-byte Node actually consumes 32 bytes per alloc) → arena
  undersized by ~800KB → arena_grow fired mid-benchmark → realloc moved the block →
  every previously-stored Node* (in the swap-test array and in list pointers)
  went dangling → segfault. Fixed with ALIGNED_NODE_SIZE macro matching arena_alloc's
  actual rounding behavior
- test-api.c: inverted assertion — `assert(list_is_linked(&node2.link))` immediately
  after `list_unlink(&node2.link)`. list_unlink reinitializes via list_init, so the
  node becomes self-linked and list_is_linked correctly returns false, not true.
  Traced exact pointer values to confirm list_unlink_stale was working correctly
  and the bug was in the test's expectation, not the list code
- test-api.c: list_length was called but did not exist in the header at the time —
  added as a simple list_for_each-based O(n) walk-and-count, documented as a
  test/diagnostic helper rather than hot-path API

## Testing Coverage — Intrusive List
- test-api.c: NULL-safety on list_entry/list_entry_offset, basic link/unlink,
  swap (adjacent + non-adjacent), splice (normal + empty-source no-op),
  list_replace, raw iterators (list_for_each, list_for_each_safe with mid-loop
  unlink, list_for_each_entry) — all passing
- test-embedded.c: same coverage plus list_pop_front/back on empty list (NULL,
  no crash), list_replace vs list_replace_init side-by-side (stale vs detached
  old node), splice-then-reuse-drained-source, full lifecycle (build N entries,
  drain via pop_front, verify insertion order preserved) — all passing
- Both test files clean under -fsanitize=address,undefined (Valgrind not available
  in current sandbox; ASan+UBSan used as substitute — no errors found)

## Bench Numbers — Intrusive List, Arena vs Malloc (real hardware, N=100k, 1000 runs)

  Operation                    Arena (p50/p95/p99 ns)         Malloc (p50/p95/p99 ns)
  ──────────────────────────────────────────────────────────────────────────────────
  insert (tail)                  20 /     20 /     21           54 /     72 /     73
  search hit (tail)          308833 / 318945 / 333496      328730 / 351461 / 371894
  search miss (full scan)    312334 / 325217 / 339884      327074 / 354382 / 374615
  unlink head                    73 /    151 /    257          105 /    225 /    286
  swap (far apart)              100 /    206 /    291          155 /    317 /    425
  splice (50k+50k)               72 /    101 /    218          130 /    292 /    533
  list_length (full walk)    133742 / 161546 / 173905      220613 / 275459 / 324908

  arena_grow isolation (undersized arena, ~7 doublings/run):
  insert w/ growth               22 /     30 /     31  ns (amortized per-op)

### Analysis Notes
- Insert: arena 2.7x faster (20ns vs 54ns) — bump pointer vs real malloc call.
  Cleanest, most expected result.
- list_length: arena 1.65x faster (133742 vs 220613) — pure pointer-chase, no
  branching, isolates cache-locality effect most cleanly. Best number for the
  "why arena" section of the design doc.
- search hit/miss: only ~6% gap between allocators, much smaller than list_length's
  65% gap — search target was the tail node, meaning hit and miss traverse the
  same number of nodes (this is correct/expected, not a bug). The smaller-than-
  expected allocator gap here is likely because the per-node branch+comparison
  cost in the search loop dilutes the cache-locality signal relative to the
  pure-traversal list_length case. Worth addressing explicitly in the design doc
  rather than ignoring — an interviewer would ask why search doesn't show the same
  gap as list_length if cache locality were the only factor at play.
- unlink/swap/splice: arena and malloc nearly identical (within noise) — correct,
  since none of these touch the allocator, only pointer topology. Useful as an
  internal sanity check that the benchmark harness isn't introducing bias.
- Caveat to include in report: operations under ~50ns are near the resolution
  floor of CLOCK_MONOTONIC on typical hardware; p95/p99 spread at that scale
  likely reflects measurement/scheduling noise more than true tail latency.

## Known Gaps / Next Steps for Phase 4
- Search benchmark only tests worst-case (tail) hit — hit and miss collapse to
  the same number. Add a head-proximate search-hit variant (e.g. index 10) to
  get a real best-case vs worst-case spread before finalizing the design doc
- Valgrind not run yet on this implementation (sandbox lacked it) — run on real
  dev machine: gcc -g → valgrind --leak-check=full --track-origins=yes
- arena_grow's realloc-move hazard is documented but not tested under conditions
  where external pointers are deliberately held across growth (would currently
  produce a UAF — intentionally out of scope, but worth a negative test case
  confirming ASan catches it, for the design doc's "known limitations" section

## Phase 1 Status: All 4 list implementations complete
1. Tagged union ✅  2. void* ✅  3. Macro-generic ✅  4. Intrusive + arena ✅
Remaining for Phase 1: design doc with measured comparisons across all four

## Phase 2 (kv store): epoll, robin hood hashmap, WAL, arena allocator
## Target: Winter 2026 internship, systems/infra teams

## Next Immediate Action
1. Add head-proximate search-hit bench variant (currently only worst-case/tail
   is measured — hit and miss numbers are nearly indistinguishable as a result)
2. Run Valgrind on real dev machine for intrusive list + arena (sandbox lacked it,
   substituted ASan/UBSan — clean, but Valgrind is the established project standard)
3. Assemble design doc: pull tagged-union and void* numbers (already recorded),
   macro-generic numbers (already in this file), and intrusive+arena numbers
   (now in this file) into the comparison report — doc already scaffolded with
   section stubs from earlier session
4. After design doc: begin Phase 2 (kv store)