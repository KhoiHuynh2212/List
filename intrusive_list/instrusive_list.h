#pragma once

#include<stddef.h>
#include<stdint.h>  

/*
    struct list : Circular Doubly Linked List 
*/
typedef struct list {
    struct list* next;
    struct list* prev;
} list;


#define LIST_INIT(_var) {.next = &(_var), .prev = &(_var)} // initialize struct list

static inline list* list_init(list* what) { 
    *what = (list) LIST_INIT(*what); 
    return what;
} 

static inline void* list_entry_offset(list * what, size_t offset) {
    if(what) {

        return (void*) (((uintptr_t)(void*) what) - offset );
    }
    return NULL;
}







