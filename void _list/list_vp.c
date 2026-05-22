#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<list_vp.h> 



void insertHead(Node** head, void* data, size_t size) {
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
    newNode->next = *head; 

    if(*head != NULL) {
        (*head)->prev = newNode;
    } 
    *head = newNode;
}

void insertEnd(Node** head, void* data, size_t size) {
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
        // head insertion 
        Node* first = *head;

        while(first->prev != NULL) {
            first = first->prev;
        } 

        first->prev = newNode;
        newNode->next = first;
        *head = newNode;
       
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

/* The caller define comparator */
int deleteNode(Node**head, void* key, int(*cmp)(void*, void*)) {
    Node* curr = *head;

    while(curr != NULL) {
        if(cmp( curr->data , key)) {
            Node* prevNode = curr->prev;
            Node* nextNode = curr->next;
            
            if(prevNode != NULL) {
                prevNode->next = nextNode;
            } else {
                // delete head 
                *head = nextNode;
            }

            if(nextNode!= NULL) {
                nextNode->prev = prevNode;
            }

            free(curr->data);
            free(curr);
            return 0;    
        }
        curr = curr->next;
    }

    return -1;
} 

Node* searchNode(Node*head, void*data, int(*cmp)(void*, void*)) {
    Node* curr = head; 
    while(curr != NULL) {
        if(cmp(curr->data, data)) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

// COMPARATOR 
int cmp_int(void* v1, void* v2) {
    return *(int*)v1 == *(int*)v2;
} 

int cmp_str(void*a , void* b) {
    return strcmp((char*) a, (char*) b) == 0;
}