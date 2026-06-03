#include <stdio.h>
#include "list.h"

DEFINE_LIST(int)

DEFINE_LIST_STRING

int main()
{
    intNode *head = NULL;

    int_insert(&head, 10);
    int_insert(&head, 20);
    int_insert(&head, 30);

    // walk and print
    intNode *cur = head;
    do
    {
        printf("%d\n", cur->data);
        cur = cur->next;
    } while (cur != head);

    // delete
    int_delete_node(&head, 20);

    // search
    intNode *found = int_search(head, 30);
    if (found)
        printf("found: %d\n", found->data);

    strNode *headStr= NULL;

    str_insert(&headStr, "khoi");
    str_insert(&headStr, "quyen");
    str_insert(&headStr, "lavang");

    strNode *curr =headStr; 


    do {
        printf("%s\n", curr->data);
        curr = curr->next;
    } while(curr != headStr);

    strNode * found2 = str_search(headStr, "diep"); 

    if(found2) 
        printf("found2: %s\n", found2->data);
        

    return 0;
}