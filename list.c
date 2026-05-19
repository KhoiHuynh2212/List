#include <stdlib.h> 
#include<stdbool.h> 
#include<string.h>
#include<stdio.h> 
#include<list.h>


void insertIntNode(Node ** head,int data) {
    Node* newNode = malloc(sizeof(Node));
    if(newNode == NULL) {
        printf("Memory allocation error\n");
    
        return;
    } 

    newNode->data.node_int = data;
    newNode->kind = INTEGER; 
    newNode->next = NULL;

    if(*head == NULL) {
        *head = newNode;
    } else {
        Node* lastNode = *head;

        while(lastNode->next != NULL) {
            lastNode = lastNode->next;
        } 

        lastNode->next = newNode;
    }
} 

void insertStringNode(Node **head, char* data) {
    Node* newNode = malloc(sizeof(Node));
    if(newNode == NULL) {
        printf("Memory allocation error\n");
    
        return;
    } 

    int len = strlen(data);
    char* dest = malloc(sizeof(char) * (len + 1));
    if(dest == NULL) {
        printf("Cannot allocate dest string\n");
        // free what was allocated
        free(newNode);
        return;
    } 

    strcpy(dest,data);

    newNode->kind = STRING; 
    newNode->data.node_string = dest;
    newNode->next = NULL;

    if(*head == NULL) {
        *head = newNode;
    } else {
        Node* lastNode = *head;

        while(lastNode->next != NULL) {
            lastNode = lastNode->next;
        } 

        lastNode->next = newNode;
    }
}
// Assumes homogeneous list — all nodes share the same kind as head
void free_list(Node* head) {
    Node* temp = head;

    if(head == NULL) {
        return;
    }
    // Free nodes based on their kind
    switch (temp->kind)
   {
   case STRING:
    while(temp != NULL) {
        Node* nextNode = temp->next;
        free(temp->data.node_string);
        free(temp);
        temp = nextNode;
    }
    break;
   case INTEGER:
   case DOUBLE:
   case FLOAT:

    while(temp != NULL) {
        Node* nextNode = temp->next;
        free(temp);
        temp = nextNode;
    }
    break;
   }
} 


void deleteIntNode(Node** head, int value){

    if(*head == NULL) {
        return;
    }  
    
    // head is the target
    if(*head != NULL && (*head)->kind ==INTEGER && (*head)->data.node_int == value) {
        Node* temp = *head;
        *head = (*head)->next;
        free(temp);
        return;
    } 


    Node* prev = *head;
    Node* temp = (*head)->next;
    while(temp != NULL) {
        if(temp->data.node_int == value) {
            prev->next = temp->next;
            free(temp);
            return;
        } else {
            prev = temp;
            temp = temp->next;
        }
    }
        
}


void display(Node* head) {
    Node* temp = head;
    while (temp != NULL) {
        switch (temp->kind) {
            case INTEGER: printf("%d ", temp->data.node_int);    break;
            case FLOAT:   printf("%f ", temp->data.node_float);  break;
            case DOUBLE:  printf("%lf ", temp->data.node_double); break;
            case STRING:  printf("%s ", temp->data.node_string); break;
            default:      printf("unknown ");                    break;
        }
        temp = temp->next;
    }
    printf("\n");
}