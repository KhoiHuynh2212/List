#include<stdio.h>
#include<stdlib.h>
#include<list.h> 
#include<string.h>


int main() {
    Node* head = NULL;

    insertIntNode(&head, 10);
    insertIntNode(&head, 20);
    insertIntNode(&head, 30);

    insertStringNode(&head, "Hello");
    insertStringNode(&head, "World");

    display(head);
    free_list(head);

    return 0;
}