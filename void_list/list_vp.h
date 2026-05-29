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
void list_insertTail(Node **head, void*data, size_t size);
void list_insertBegin(Node **head, void*data, size_t size);
void insertMid(Node**head, void*data, size_t size, int pos);
void free_list(Node**head);
int deleteNode(Node**head, void* key, int(*cmp)(void*, void*)); // The caller handle cmp
Node* searchNode(Node*head, void*data, int(*cmp)(void*, void*)); 
Node* reverse(Node**head);

void display_ints(Node*head);
int count(Node*head);
#endif /*LIST_H*/