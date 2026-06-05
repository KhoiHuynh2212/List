#include<stdio.h>
#include<stdlib.h>
#include<list_vp.h> 
#include<string.h>
#include <unistd.h>

void printInt(void* data) {
    int value = *(int*) data; 
    printf("%d ", value);
} 

int main() {

    Node* head = NULL; 

    int a = 10;
    int b = 20; 
    int c = 30;
    printf("Initial program break : %10p\n", sbrk(0));
    insertHead(&head,&a,sizeof(int)); 
    insertHead(&head,&b,sizeof(int));
    insertHead(&head,&c,sizeof(int));

    int d = 100;
    int e = 200;
    int f = 300;
    insertEnd(&head,&d, sizeof(int));
    insertEnd(&head,&e, sizeof(int));
    insertEnd(&head,&f, sizeof(int));
   

    printf("Before reverse :");
    display_list(head,printInt); 

    reverse(&head);
    printf("After reverse: ");
    display_list(head,printInt);


    printf("Program break is now : %10p\n", sbrk(0));

    free_list(&head);

    printf("After free(), program break is %10p\n", sbrk(0));
}