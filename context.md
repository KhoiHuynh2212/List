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
- deleteIntNode (look-ahead pattern)
- Header guards + correct includes in list.h

TODO:
- search(Node* head, int value) → returns Node*
- Then: benchmark harness (clock_gettime, p50/p95/p99, perf stat)

## Key Decisions Made
- Mixed list supported: display/free_list check each node's kind individually
- deleteIntNode uses look-ahead pattern (temp->next), not prev+curr
- free_list takes Node** and sets *head = NULL after freeing (production pattern)
- Learned: Node* to read, Node** to modify head itself

## Phase 1 Sequence
1. Complete tagged list (add search) ← YOU ARE HERE
2. Benchmark tagged list (baseline numbers)
3. Refactor to void* list, same benchmarks
4. Macro-generic list + arena allocator, same benchmarks
5. Design doc with measured comparisons

## Phase 2: kv store (epoll, robin hood hashmap, WAL, arena allocator)
## Target: Winter 2026 internship, systems/infra teams

## Next Immediate Action
Write search() then compile clean with gcc -Wall -Wextra -I. -o list list.c main.c