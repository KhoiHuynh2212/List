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
    newNode->prev = NULL;

    if(*head == NULL) {
        *head = newNode;
    } else {
        Node* lastNode = *head;

        while(lastNode->next != NULL) {
            lastNode = lastNode->next;
        } 

        lastNode->next = newNode;
        newNode->prev = lastNode;
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
    newNode->prev = NULL;

    if(*head == NULL) {
        *head = newNode;
    } else {
        Node* lastNode = *head;

        while(lastNode->next != NULL) {
            lastNode = lastNode->next;
        } 

        lastNode->next = newNode;
        newNode->prev = lastNode;

    }
}
// Assumes homogeneous list — all nodes share the same kind as head
void free_list(Node** head) {
  Node* temp = *head;
    while (temp != NULL) {
        Node* next = temp->next;
        if (temp->kind == STRING)
            free(temp->data.node_string);
        free(temp);
        temp = next;
    }
    *head = NULL;   // prevents use-after-free
} 

int deleteIntNode(Node** head, int value) {
    Node** curr = head;

    while (*curr != NULL) {
        Node* node = *curr;  // the node curr is pointing at

        if (node->kind == INTEGER && node->data.node_int == value) {
            *curr = node->next;  // rewrite the arrow to skip this node
            free(node);
            return 0;
        }

        curr = &node->next;  // move to the next arrow
    }
    return -1;
}

int deleteStringNode(Node** head, char* str) {
    Node** curr = head;

    while(*curr != NULL) {
        Node* node = *curr; 
        if(node->kind == STRING && strcmp(node->data.node_string, str) == 0) {
           *curr = node->next;
            free(node->data.node_string);
            free(node); 
            return 0 ;
        } 
        curr = &node->next;
    }
    return -1;
} 

Node* searchString(Node* head, const char* str) {
    Node* curr = head;
    while (curr != NULL) {
        if (curr->kind == STRING && strcmp(curr->data.node_string, str) == 0)
            return curr;   // found — return pointer to the node
        curr = curr->next;
    }
    return NULL;           // not found
}

Node* searchInt(Node* head,int key) {
    Node* curr = head;
    while (curr != NULL) {
        if (curr->kind == INTEGER && curr->data.node_int == key)
            return curr;   // found — return pointer to the node
        curr = curr->next;
    }
    return NULL;           // not found
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