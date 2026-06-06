#include<stddef.h>
#include "instrusive_list.h"
#include<stdio.h>

typedef struct {
    int data;
    list link;
} Node;

int main() {

    list mylist = LIST_INIT(mylist); 

    Node node = {.data = 10, .link = LIST_INIT(node.link)};

    list* l1 = &node.link;

    Node *parent = list_entry(l1, Node, link);
    printf("data = %d\n", parent->data); 

    return 0;
}