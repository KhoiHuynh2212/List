#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <list.h>

void insertIntNode(Node **head, int data)
{
    Node *newNode = malloc(sizeof(Node));
    if (newNode == NULL)
    {
        printf("Memory allocation error\n");
        return;
    }
    newNode->kind = INTEGER;
    newNode->data.node_int = data;

    if (*head == NULL)
    {
        newNode->prev = newNode;
        newNode->next = newNode;
        *head = newNode;
    }
    else
    {

        Node *first = *head;
        Node *last = first->prev;

        newNode->next = first;
        newNode->prev = last;
        last->next = newNode;
        first->prev = newNode;

        // tail insertion
    }
}

void insertStringNode(Node **head, char *data)
{
    Node *newNode = malloc(sizeof(Node));
    if (newNode == NULL)
    {
        printf("Memory allocation error\n");

        return;
    }

    int len = strlen(data);
    char *dest = malloc(sizeof(char) * (len + 1));
    if (dest == NULL)
    {
        printf("Cannot allocate dest string\n");
        // free what was allocated
        free(newNode);
        return;
    }

    strcpy(dest, data);
    newNode->kind = STRING;
    newNode->data.node_string = dest;

    if (*head == NULL)
    {
        newNode->next = newNode;
        newNode->prev = newNode;
        *head = newNode;
    }
    else
    {
        Node *first = *head;
        Node *last = first->prev;

        newNode->prev = last;
        newNode->next = first;
        last->next = newNode;
        first->prev = newNode;

        // tail insertion
    }
} 

void insertFloatNode(Node **head, float data)
{
    Node *newNode = malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("Memory allocation error\n");
        return;
    }
 
    newNode->data.node_float = data;
    newNode->kind            = FLOAT;
 
    if (*head == NULL) {
        newNode->next = newNode;
        newNode->prev = newNode;
        *head = newNode;
    } else {
        Node *first = *head;
        Node *last  = first->prev;
 
        newNode->next = first;
        newNode->prev = last;
        last->next    = newNode;
        first->prev   = newNode;
    }
}
                                    
void insertDoubleNode(Node **head, double data)
{
    Node *newNode = malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("Memory allocation error\n");
        return;
    }
 
    newNode->data.node_double = data;
    newNode->kind             = DOUBLE;
 
    if (*head == NULL) {
        newNode->next = newNode;
        newNode->prev = newNode;
        *head = newNode;
    } else {
        Node *first = *head;
        Node *last  = first->prev;
 
        newNode->next = first;
        newNode->prev = last;
        last->next    = newNode;
        first->prev   = newNode;
    }
}

void free_list(Node **head)
{
    Node *temp = *head;
    Node *first = *head;
    do
    {
        Node *next = temp->next;
        if (temp->kind == STRING)
            free(temp->data.node_string);
        free(temp);
        temp = next;
    } while (temp != first);

    *head = NULL; // prevents use-after-free
}

int deleteIntNode(Node **head, int value)
{
    if (head == NULL || *head == NULL)
    {
        return -1;
    }

    Node *curr = *head;
    Node *first = *head;
    do
    {
        if (curr->kind == INTEGER && curr->data.node_int == value)
        {
            if (curr->next == curr)
            {
                *head = NULL;
            }
            else
            {
                curr->prev->next = curr->next;
                curr->next->prev = curr->prev;

                if (curr == *head)
                {
                    *head = curr->next;
                }
            }

            free(curr);
            return 0;
        }
        curr = curr->next;
    } while (curr != first);

    return -1;
}

int deleteStringNode(Node **head, char *str)
{
    if (head == NULL || *head == NULL)
    {
        return -1;
    }

    Node *curr = *head;
    Node *first = *head;
    do
    {
        if (curr->kind == STRING && strcmp(curr->data.node_string, str) == 0)
        {
            if (curr->next == curr)
            {
                *head = NULL;
            }
            else
            {
                curr->prev->next = curr->next;
                curr->next->prev = curr->prev;

                if (curr == *head)
                {
                    *head = curr->next;
                }
            }

            if(curr->data.node_string != NULL) {
                free(curr->data.node_string);
            }
            free(curr);
            return 0;
        }
        curr = curr->next;
    } while (curr != first);

    return -1;
} 

int deleteFloatNode(Node **head, float value)
{
    if (*head == NULL) return -1;
 
    Node *curr  = *head;
    Node *first = *head;
 
    do {
        if (curr->kind == FLOAT && curr->data.node_float == value) {
            if (curr->next == curr) {
                *head = NULL;
            } else {
                curr->prev->next = curr->next;
                curr->next->prev = curr->prev;
                if (curr == *head)
                    *head = curr->next;
            }
            free(curr);
            return 0;
        }
        curr = curr->next;
    } while (curr != first);
 
    return -1;
}
                           
int deleteDoubleNode(Node **head, double value)
{
    if (*head == NULL) return -1;
 
    Node *curr  = *head;
    Node *first = *head;
 
    do {
        if (curr->kind == DOUBLE && curr->data.node_double == value) {
            if (curr->next == curr) {
                *head = NULL;
            } else {
                curr->prev->next = curr->next;
                curr->next->prev = curr->prev;
                if (curr == *head)
                    *head = curr->next;
            }
            free(curr);
            return 0;
        }
        curr = curr->next;
    } while (curr != first);
 
    return -1;
}


Node *searchString(Node *head, const char *str)
{
    if(head == NULL || str == NULL) {
        return NULL;
    }
    Node *curr = head;
    do {
        if (curr->kind == STRING && strcmp(curr->data.node_string, str) == 0)
            return curr; // found — return pointer to the node
        curr = curr->next;
    } while (curr != head);
    return NULL; // not found
}

Node *searchInt(Node *head, int key)
{   
    if(head == NULL ) {
        return NULL;
    }
    Node *curr = head;
    do {
        if (curr->kind == INTEGER && curr->data.node_int == key)
            return curr;
        curr = curr->next;
    }while (curr != head);
    return NULL; // not found
} 

Node *searchFloat(Node *head, float key)
{
    if (head == NULL) return NULL;
 
    Node *curr = head;
    do {
        if (curr->kind == FLOAT && curr->data.node_float == key)
            return curr;
        curr = curr->next;
    } while (curr != head);
 
    return NULL;
}
 
Node *searchDouble(Node *head, double key)
{
    if (head == NULL) return NULL;
 
    Node *curr = head;
    do {
        if (curr->kind == DOUBLE && curr->data.node_double == key)
            return curr;
        curr = curr->next;
    } while (curr != head);
 
    return NULL;
}


void reverse(Node **head)
{
    if (head == NULL || *head == NULL) return;

    Node *curr = *head;
    Node *temp = NULL;

    do {
        temp = curr->next;
        curr->next = curr->prev;
        curr->prev = temp;

        curr = curr->prev;
    } while (curr != *head);

    *head = (*head)->next;
}

void display(Node *head)
{
    if (head == NULL) {
        printf("\n");
        return;
    }
 
    Node *temp = head;
    do {
        switch (temp->kind) {
            case INTEGER: printf("%d ",  temp->data.node_int);    break;
            case FLOAT:   printf("%f ",  temp->data.node_float);  break;
            case DOUBLE:  printf("%lf ", temp->data.node_double); break;
            case STRING:  printf("%s ",  temp->data.node_string); break;
            default:      printf("unknown ");                     break;
        }
        temp = temp->next;
    } while (temp != head);
 
    printf("\n");
}
