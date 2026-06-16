#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include "arena.h"
#include "instrusive_list.h"
#include <stdint.h>

typedef struct
{
    int data;
    list link;
} Node;

int main()
{

    // list mylist = LIST_INIT(mylist);

    // Node node = {.data = 10, .link = LIST_INIT(node.link)};

    // list* l1 = &node.link;

    // Node *parent = list_entry(l1, Node, link);
    // printf("data = %d\n", parent->data);

    // printf("Maximum fundamental alignment : %zu bytes\n", ARENA_ALIGN);
    // printf("pointer:     %zu\n", _Alignof(void *));
    // printf("long double: %zu\n", _Alignof(long double));

    arena b = arena_create(1024 * 1024);

    size_t count = 0;
    size_t cap_before = b.cap;
    printf("bump start %p\n", (void*)b.bump);
    while (b.bump + 8 <= b.base + cap_before)
    { // check bounds yourself, don't call arena_alloc blindly
        arena_alloc(&b, 8);
        count++;
    }
    printf("Allocated %zu blocks before growth would trigger\n", count);
    printf("bump end  %p\n", (void *)b.bump);
    arena_reset(&b);
    printf("Reset to base : %p\n", (void *)b.bump);
    printf("Allocated %zu blocks\n", count);
    arena_destroy(&b);
    return 0;
}