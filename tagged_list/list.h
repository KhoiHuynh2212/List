#ifndef LIST_H
#define LIST_H

#include <string.h>  
#include <stdlib.h>

typedef enum node_kind_t {
    INTEGER = 0, 
    DOUBLE = 1,
    FLOAT = 2, 
    STRING = 3
} node_kind_t;

typedef union node_data {
    int node_int;
    double node_double;
    float node_float;
    char* node_string;
} node_data;

typedef struct Node {
    struct Node * next;
    node_kind_t kind;
    node_data data;

} Node; 

void insertIntNode(Node ** head, int data);
void insertStringNode(Node ** head, char* data);

int deleteIntNode(Node** head, int value);
int deleteStringNode(Node** head, char* str);
void free_list(Node** head);
void display(Node* head);

Node* searchString(Node* head, const char* str);
Node* searchInt(Node* head, int val);

#endif