CONTEXT: Systems programming internship prep, Summer 2026 → Winter internship goal.

## Current Project: libds (C data structures library)
Phase 1 of 4. Building 3 implementations of a linked list to benchmark against each other:
1. Tagged union (current) → 2. void* → 3. Macro-generic + arena allocator

## Tagged Union List — Current Status
File: list.h + list.c
DONE:
- insertIntNode, insertStringNode
- display (switch inside loop, handles mixed types)
- free_list (checks kind == STRING to free node_string before freeing node)
- deleteIntNode, deleteStringNode (look-ahead pattern, kind check before strcmp)
- searchInt, searchString (return Node*, NULL on miss)
- Header guards + correct includes in list.h
- test.c: full correctness suite, 25/25 pass, 0 Valgrind errors
- bench_fast.c: p50/p95/p99 harness, baseline numbers recorded
- .gitignore: binaries excluded

## Testing Knowledge Gained
- CHECK macro with do{}while(0) to avoid dangling else bug
- Four test categories: happy path, boundaries, miss case, after-effects
- Valgrind workflow: gcc -g → ./test → valgrind --leak-check=full --track-origins=yes ./test
- free_list(&head) required at end of every test function or Valgrind reports leak
- Passing wrong type to a function (e.g. int to char*) causes crash at call site, not inside function

## Key Decisions Made
- Mixed list supported but caller is responsible for calling right function
- deleteStringNode checks kind == STRING before strcmp (safe with mixed lists)
- deploy key vs personal SSH key: personal key covers all repos, deploy key is per-repo
- Node* to read, Node** to modify head itself

## Phase 1 Sequence
1. Complete tagged list (search, delete, display, free)
2. Benchmark tagged list (baseline numbers recorded)
3. Refactor to void* list, same benchmarks  ← YOU ARE HERE
4. Macro-generic list + arena allocator, same benchmarks
5. Design doc with measured comparisons

## Phase 2: kv store (epoll, robin hood hashmap, WAL, arena allocator)
## Target: Winter 2026 internship, systems/infra teams

## Next Immediate Action
Start void* list implementation — same API, replace tagged union with void* + size_t