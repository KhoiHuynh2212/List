#ifndef LIST_H
#define LIST_H
#include <stdlib.h>

typedef struct Node {
    size_t size;
    void* data;                 
    struct Node *next;
    struct Node *prev;
} Node;

Node* list_init(void*data, size_t size);
void insertEnd (Node **head, void*data, size_t size);
void insertHead(Node **head, void*data, size_t size);
void list_insertMid(Node*node_ptr, void*data, size_t size);

void free_list(Node**head);

int deleteNode(Node**head, void* key, int(*cmp)(void*, void*)); // The caller handle cmp
Node* searchNode(Node*head, void*data, int(*cmp)(void*, void*)); 
Node* reverse(Node**head);

void display_list(Node*head, void (*fptr)(void*));
int count(Node*head);
#endif /*LIST_H*/ 