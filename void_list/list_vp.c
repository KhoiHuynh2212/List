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
    
        Node* first = *head;

        while(first->next != NULL) {
            first = first->next;
        } 

        first->next = newNode;
        newNode->prev = first;
       
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
    // prevent use after free 
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

Node* reverse(Node**head) {
    Node* curr = *head;
    Node* temp = NULL;
    // edge case
    if(curr == NULL || curr->next == NULL) {
        return curr;
    } 

    Node *last = NULL;
    while (curr != NULL) {
        temp = curr->prev;
        curr->prev = curr->next;
        curr->next = temp;

        last = curr;       
        curr = curr->prev;
    }
    *head = last;

    return *head;
}

int count(Node*head) {
    int cnt = 0;
    Node* temp = head;
    while(temp != NULL) {
        cnt++;
        temp = temp->next;
    }
    return cnt;
}
