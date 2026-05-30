#include<stdio.h>
#include<stdlib.h>
#include<list_vp.h> 
#include<string.h>

void printInt(void* data) {
    int value = *(int*) data; 
    printf("%d ", value);
} 

int main() {

    Node* head = NULL; 

    Node* rs = NULL;
    int b = 20; 
    int c = 30;
    list_insertBegin(&head,&rs,sizeof(int)); 
    list_insertBegin(&head,&b,sizeof(int));
    list_insertBegin(&head,&c,sizeof(int)); 

    int d = 100;
    int e = 200;
    int f = 300;
    list_insertTail(&head,&d, sizeof(int));
    list_insertTail(&head,&e, sizeof(int));
    list_insertTail(&head,&f, sizeof(int));

    int ee = 900;
    list_insertBegin(&head, &ee, sizeof(int));
    display_list(head,printInt); 
    int value = *(int *)(head->data);
    printf("%d \n", value);

    free_list(&head);
}