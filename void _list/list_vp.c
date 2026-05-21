#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<list_vp.h> 


void insert(Node** head, void* data, size_t size) {
    Node* newNode = malloc(sizeof(Node));
    if(newNode == NULL) {
        printf("Memory allocation error\n");
        return;
    } 
    newNode->data = malloc(size); 
    if(newNode->data == NULL) {
        free(newNode);
        printf("Memory allocation error\n");
        return;
    }
    memcpy(newNode->data, data, size);


    newNode->prev = NULL;
    newNode->next = NULL; 

    if(*head == NULL) {
        *head = newNode;
    } else {
        // tail insertion 
        Node* first = *head;

        while(first->next != NULL) {
            first = first->next;
        } 

        first->next = newNode;
        newNode->prev = first;
        newNode->next = NULL;
    }
}

// free entire list and each node data
void free_list(Node**head) {
    Node* curr = *head;
    while(curr != NULL) {
        Node* next = curr->next;
        free(curr->data);
        free(curr);
    
        curr = next;
    }

    *head = NULL;
} 

int deleteNode(Node**head, void* key, int(*cmp)(void*, void*)) {
    Node* curr = *head;

    while(curr != NULL) {
        if(cmp( curr->data , key)) {

            return 1;
        }
        curr = curr->next;
    }

    return 0;
}