#ifndef LIST_H
#define LIST_H
#include <stdlib.h>

typedef struct {
    void* data;
    struct Node *next;
    struct Node *prev;
    size_t size;
} Node;

void insert(Node **head, void*data, size_t size);
void free_list(Node**head);

int deleteNode(Node**head, void* key, int(*cmp)(void*, void*)); // The caller handle cmp

#endif /*LIST_H*/