#include <stddef.h>
#include <stdlib.h>

#define ARENA_ALIGN _Alignof(max_align_t)

typedef struct {
    char* base;
    char* bump;
    size_t cap;
} arena; 

arena arena_create(size_t cap) {
    char* base = malloc(cap);
    return (arena){ .base = base, .bump = base, .cap = base ? cap : 0};
} 

void* arena_alloc(arena* a, size_t size) {
    size = (size + ARENA_ALIGN - 1) & ~(ARENA_ALIGN - 1); 
    if(a->bump + size > a->base + a->cap) {
        return NULL;
    } 

    void* ptr = a->bump; 
    a->bump += size; 
    return ptr; 
} 

void arena_destroy(arena * a) {
    free(a->base);
    a->base = NULL;
    a->bump = NULL; 
    a->cap = 0;
}