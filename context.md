CONTEXT: Systems programming internship prep, Summer 2026 → Winter internship goal.

## Current Project: libds (C data structures library)
Phase 1 of 4. Building 4 implementations of a linked list to benchmark against each other:
1. Tagged union ✅ → 2. void* ✅ → 3. Macro-generic ✅ → 4. Intrusive + arena allocator

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

## Phase 4: Intrusive List + Arena Allocator
Next implementation:
- Embed list_node_t inside user struct, recover via container_of + offsetof
- Arena allocator: one big malloc upfront, bump pointer for each node, one free at end
- Same test/bench harness for apples-to-apples comparison
- Expected: zero per-node heap allocation, best cache locality, lowest p99

## Phase 2 (kv store): epoll, robin hood hashmap, WAL, arena allocator
## Target: Winter 2026 internship, systems/infra teams

## Next Immediate Action
Implement intrusive list + arena allocator (Phase 4)