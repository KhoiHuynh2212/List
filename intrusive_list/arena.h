#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#define ARENA_ALIGN _Alignof(max_align_t)

typedef struct {
    char* base;
    char* bump;
    size_t cap;
} arena; 

arena arena_create(size_t cap) {
    char* base = malloc(cap);

    if(base == NULL) {
        return (arena){0};
    }
    return (arena){ .base = base, .bump = base, .cap = cap};
} 

void arena_grow(arena*a, size_t extra) {
    size_t used = a->bump - a->base; 
    size_t new_cap = a->cap * 2; 
    size_t extra_required_cap = extra + a->cap;
    if(new_cap < extra_required_cap) new_cap = extra_required_cap;

    char* new_base = realloc(a->base, new_cap); 
    if(new_base == NULL) {
        fprintf(stderr, "arena_grow failed : cap = %zu\n", new_cap);
        abort();
    }

    a->base = new_base;
    a->cap = new_cap;
    a->bump =  new_base + used;
}

void* arena_alloc(arena* a, size_t size) {
    if (size == 0) return NULL;
    size = (size + ARENA_ALIGN - 1) & ~(ARENA_ALIGN - 1); 
    if(a->bump + size > a->base + a->cap) {
        arena_grow(a, size);
    } 

    void* ptr = a->bump; 
    a->bump += size; 
    return ptr; 
} 

void arena_reset(arena * a) {
    // reset to base
    a->bump = a->base;
}

void arena_destroy(arena * a) {
    free(a->base);
    a->base = NULL;
    a->bump = NULL; 
    a->cap = 0;
} 

#endif 