#include <stdlib.h> 
#include<stdbool.h> 
#include<stdio.h> 
#include<list.h>


Node* create_int_node(int data) {
    Node *new_node = (Node*)malloc(sizeof(Node));
    if(new_node == NULL) {
        printf("Memory allocation error\n");
        return NULL;
    } 

    new_node->data.node_int = data;
    new_node->kind = INTEGER;
    new_node->next = NULL;
    return new_node;
} 

Node* create_string_node(char* data) {
    Node *new_node = (Node*)malloc(sizeof(Node));
    if(new_node == NULL) {
        printf("Memory allocation error\n");
    
        return NULL;
    }  

    int len = strlen(data);
    char* dest = malloc(sizeof(char) * (len + 1));
    if(dest == NULL) {
        printf("Cannot allocate dest string\n");
        // free what was allocated
        free(new_node);
        return NULL;
    } 

    strcpy(dest,data);

    new_node->kind = STRING;
    new_node->data.node_string = dest;
    new_node->next = NULL;
    return new_node;
}