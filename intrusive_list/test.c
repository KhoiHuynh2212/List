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

    arena a = arena_create(1024);
    int *x = arena_alloc(&a, sizeof(int));
    double *y = arena_alloc(&a, sizeof(double));

    long double *z = arena_alloc(&a, sizeof(long double));

    assert(x != NULL);
    assert(y != NULL);
    assert(z != NULL);

    *x = 42;
    *y = 3.14159;
    *z = 1.23456789L;

    printf("%d\n", *x);
    printf("%f\n", *y);
    printf("%Lf\n", *z);

    assert(((uintptr_t)x % _Alignof(int)) == 0);
    assert(((uintptr_t)y % _Alignof(double)) == 0);
    assert(((uintptr_t)z % _Alignof(long double)) == 0);

    printf("x = %p\n", (void *)x);
    printf("y = %p\n", (void *)y);
    printf("z = %p\n", (void *)z);

    arena b = arena_create(1024 * 1024);

    size_t count = 0;

    while (arena_alloc(&b, 8) != NULL)
    {
        count++;
    }

    printf("Allocated %zu blocks\n", count);
    arena_destroy(&a);
    arena_destroy(&b); 
    return 0;
}