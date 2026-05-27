#ifndef LIST_H
#define LIST_H
#include <stdlib.h>

typedef struct Node {
    void* data;
    struct Node *next;
    struct Node *prev;
    size_t size;
} Node;


void insertEnd(Node **head, void*data, size_t size);
void insertHead(Node **head, void*data, size_t size);
void free_list(Node**head);
int deleteNode(Node**head, void* key, int(*cmp)(void*, void*)); // The caller handle cmp
Node* searchNode(Node*head, void*data, int(*cmp)(void*, void*)); 
Node* reverse(Node**head);

int count(Node*head);
#endif /*LIST_H*/