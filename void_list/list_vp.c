#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<list_vp.h> 

Node* list_init(void*data, size_t size) {
    Node* node = malloc(sizeof(Node)); 
    if(node == NULL) {
        fprintf(stderr, "Memory allocation failed in list_init");
        return NULL;
    } 

    node->data = malloc(size); 
    if(node->data == NULL) {
        fprintf(stderr, "Memory allocation failed in list_init");
        free(node);
        return NULL;
    } 

    // copy the data into the list 
    memcpy(node->data,data,size); 

    // point to itself
    node->next = node; 
    node->prev = node;
    return node;
}

void list_insertBegin(Node **head, void*data, size_t size) {
    if(*head == NULL) {
        Node* newNode = list_init(data,size); 
        *head = newNode;
    } else {

    Node* first = *head;
    Node* newNode = malloc(sizeof(Node)); 
    if(newNode == NULL) {
        fprintf(stderr, "Memory allocation failed insertHead");
        return;
    } 

    newNode->data = malloc(size); 
    if(newNode->data == NULL) {
        fprintf(stderr, "Memory allocation failed in insertHead");
        free(newNode);
        return;
    } 

    // copy the data into the node 
    memcpy(newNode->data,data,size);
    
    Node* last = first->prev;
    
    newNode->next = first;
    newNode->prev = last; 
    last->next = newNode;
    first->prev = newNode;

    // update new node 
    *head = newNode;
    }
}

void list_insertTail(Node **head, void*data, size_t size) {

    if(*head == NULL) {
        Node* newNode = list_init(data,size);
        *head = newNode;
    }  
;
    Node* newNode = malloc(sizeof(Node)); 
    if(newNode == NULL) {
        fprintf(stderr, "Memory allocation failed insertHead");
        return;
    } 

    newNode->data = malloc(size); 
    if(newNode->data == NULL) {
        fprintf(stderr, "Memory allocation failed in insertHead");
        free(newNode);
        return;
    } 

    // copy the data into the node 
    memcpy(newNode->data,data,size); 

    Node* first = *head;
    Node* last = first->prev;

    newNode->next = first;
    newNode->prev = last; 
    last->next = newNode;
    first->prev = newNode;

    // no update head here 
}

// free entire list and each node data
void free_list(Node**head) {

} 

/* The caller define comparator */
int deleteNode(Node**head, void* key, int(*cmp)(void*, void*)) {

} 

Node* searchNode(Node*head, void*data, int(*cmp)(void*, void*)) {

}

Node* reverse(Node**head) {

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

void display_ints(Node* head) {
    if (head == NULL) return;

    Node* temp = head;
    do {
        // FIX: Cast void* to int*, then dereference with *
        int value = *(int*)(temp->data);
        printf("%d ", value);
        
        temp = temp->next;
    } while(temp != head);
    
    printf("\n");
}

int main() {

    Node* head = NULL; 

    int a = 10;
    int b = 20; 
    int c = 30;
    list_insertBegin(&head,&a,sizeof(int)); 
    list_insertBegin(&head,&b,sizeof(int));
    list_insertBegin(&head,&c,sizeof(int)); 

    display_ints(head); 
    int value = *(int *)(head->data);
    printf("%d \n", value);

    free(head); 
}