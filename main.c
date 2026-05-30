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

    int a = 10;
   // int b = 20; 
   // int c = 30;
    list_insertBegin(&head,&a,sizeof(int)); 
    // list_insertBegin(&head,&b,sizeof(int));
    // list_insertBegin(&head,&c,sizeof(int));

    int d = 100;
    int e = 200;
    int f = 300;
    list_insertTail(&head,&d, sizeof(int));
    list_insertTail(&head,&e, sizeof(int));
    list_insertTail(&head,&f, sizeof(int));

    printf("Before reverse :");
    display_list(head,printInt); 

    reverse(&head);
    printf("After reverse: ");
    display_list(head,printInt);


    printf("Head previous value %d\n ", *(int*)(head->prev->data));
    free_list(&head);
}