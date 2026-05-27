#include <stdio.h>
#include <stdlib.h>
#include "list.h"

#define CHECK(name, condition) do { \
    if(condition) \
        printf("PASS %s\n", name);\
    else \
        printf("FAIL %s\n", name);\
} while(0)     


// assume mixed list
void testDeleteStringNode() {
    printf("Testing Delete String Node function\n");
    Node* head = NULL;
    insertStringNode(&head, "khanhquyen");
    insertStringNode(&head, "khoihuynh");
    insertIntNode(&head, 30);
    insertStringNode(&head, "xuandiep"); 

    int r = deleteStringNode(&head, "xuandiep");
    CHECK("return 1 on hit", r == 1);
    int y = deleteStringNode(&head, "khanhquyen");
    CHECK("Delete head", y == 1);
    CHECK("what is left ?", strcmp(head->data.node_string, "khoihuynh") == 0);
    int removeInt = deleteIntNode(&head, 30);
    CHECK("Remove int",removeInt == 1);

    free_list(&head);
    
} 


int main() {
    printf("tests running\n");
    testDeleteStringNode();
    return 0;
}