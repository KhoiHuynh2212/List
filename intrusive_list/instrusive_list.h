#pragma once

#include <stddef.h>
#include <stdint.h>
#include<stdbool.h>

/*
    struct list : Circular Doubly Linked List
*/
typedef struct list
{
    struct list *next;
    struct list *prev;
} list;

#define LIST_INIT(_var) {.next = &(_var), .prev = &(_var)} // initialize struct list

static inline list *list_init(list *what)
{
    *what = (list)LIST_INIT(*what);
    return what;
}

static inline void *list_entry_offset(list *what, size_t offset)
{
    if (what)
    {
        // cast list* - > void* then -> uintptr_t (becomes an integer)
        return (void *)(((char*)(void *)what) - offset);
    }
    return NULL;
} 

/**
 * list_entry() - get parent container of list entry
 * @_what:              list entry, or NULL
 * @_t:                 type of parent container
 * @_m:                 member name of list entry in @_t
 *
 * If the list entry @_what is embedded into a surrounding structure, this will
 * turn the list entry pointer @_what into a pointer to the parent container
 * (using offsetof(3), or sometimes called container_of(3)).
 *
 * If @_what is NULL, this will also return NULL.
 *
 * Return: Pointer to parent container, or NULL.
 */
#define list_entry(_what, _t, _m) \
        ((_t *) list_entry_offset((_what), offsetof(_t, _m)))

static inline _Bool list_is_linked(const list* what) {
    return what && (what->next != what || what->prev != what);
} 


static inline _Bool list_is_empty(const list* what) {
    return !list_is_linked(what); // only anchor node is empty 
}
