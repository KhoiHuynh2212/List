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
    struct Node * prev; 
    node_kind_t kind;
    node_data data;
} Node; 

// insert
void insertIntNode(Node** head, int data);
void insertStringNode(Node** head, char* data);
void insertFloatNode(Node** head, float data);
void insertDoubleNode(Node**head, double data);
// delete
int deleteIntNode(Node** head, int value);
int deleteFloatNode (Node **head, float  value);
int deleteDoubleNode(Node **head, double value);
int deleteStringNode(Node** head, char* str);

void free_list(Node** head);
void display(Node* head);

// search
Node *searchString(Node* head, const char* str);
Node *searchInt(Node* head, int val);
Node *searchFloat (Node *head, float key);
Node *searchDouble(Node *head, double key);

//reverse
void reverse(Node**head);
#endif