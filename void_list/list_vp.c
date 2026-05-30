#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list_vp.h>

Node *list_init(void *data, size_t size)
{
    if (data == NULL)
    {
        fprintf(stderr, "No data to hold");
        return NULL;
    }
    Node *node = malloc(sizeof(Node));
    if (node == NULL)
    {
        fprintf(stderr, "Memory allocation failed in list_init");
        return NULL;
    }

    node->data = malloc(size);
    if (node->data == NULL)
    {
        fprintf(stderr, "Memory allocation failed in list_init");
        free(node);
        return NULL;
    }

    // copy the data into the list
    memcpy(node->data, data, size);

    // point to itself
    node->next = node;
    node->prev = node;
    return node;
}

void list_insertBegin(Node **head, void *data, size_t size)
{

    if (head == NULL || data == NULL)
    {
        return;
    }
    Node *newNode = list_init(data, size);

    if (newNode == NULL)
    {
        return;
    }
    // check if head is null
    if (*head == NULL)
    {
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

        // update new node
        *head = newNode;
    }
}

void list_insertTail(Node **head, void *data, size_t size)
{

    if (head == NULL || data == NULL)
    {
        return;
    }
    Node *newNode = list_init(data, size);

    if (newNode == NULL)
    {
        return;
    }
    // check if head is null
    if (*head == NULL)
    {
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
        // no update head here
    }
}

/* Assume the pointer is not null
    @ptr :
    @data:
    @size:
*/

void list_insertMid(Node *ptr, void *data, size_t size)
{
    if (ptr == NULL || data == NULL)
    {
        return;
    }
    // [head]-->[1]-->[inset here]-->[3]
    Node *newNode = list_init(data, size);

    Node *nextcurrNode = ptr->next;

    newNode->next = nextcurrNode;
    newNode->prev = ptr;

    nextcurrNode->prev = newNode;
    ptr->next = newNode;
}

// free entire list and each node data
void free_list(Node **head)
{
    if (head == NULL || *head == NULL)
    {
        printf("Empty list \n");
        return;
    }
    Node *curr = *head;
    Node *temp = *head; // keep one pointer to read a value to avoid dangling pointer
    Node *next = NULL;
    do
    {
        next = curr->next;
        free(curr->data);
        free(curr);
        curr = next;
    } while (curr != temp);

    *head = NULL;
}

/* The caller define comparator */
int deleteNode(Node **head, void *key, int (*cmp)(void *, void *))
{
    if (head == NULL || key == NULL || *head == NULL || cmp == NULL)
    {
        return 0;
    }

    Node *curr = *head;
    Node *first = *head;

    do
    {
        if (cmp(curr->data, key) == 0)
        {

            if ((curr->next) == curr)
            {
                *head = NULL;
            }
            else
            {
                curr->next->prev = curr->prev;
                curr->prev->next = curr->next;

                if (curr == *head)
                {
                    *head = curr->next;
                }
            }

            if (curr->data != NULL)
            {
                free(curr->data);
            }

            free(curr);
            return 1;
        }

        curr = curr->next;
    } while (curr != first);

    return -1;
}

Node *searchNode(Node *head, void *data, int (*cmp)(void *, void *))
{
    if (head == NULL || data == NULL || cmp == NULL)
    {
        return NULL;
    }

    Node *curr = head;
    do
    {
        if (cmp(curr->data, data) == 0)
        {
            return curr;
        }
        curr = curr->next;
    } while (curr != head);

    return NULL;
}

Node *reverse(Node **head)
{
    if (head == NULL || *head == NULL)
    {
        return NULL;
    }

    Node *curr = *head;
    Node *temp = NULL;
    do
    {
        temp = curr->prev;
        curr->prev = curr->next;
        curr->next = temp;

        curr = curr->prev;

    } while (curr != *head);

    *head = (*head)->next;
    return *head;
}

int count(Node *head)
{   
    if(head == NULL) return 0;

    int cnt = 0;
    Node *temp = head;
    do
    {
        cnt++;
        temp = temp->next;
    } while (temp != head);
    return cnt;
}

void display_list(Node *head, void (*fptr)(void *))
{
    if (head == NULL)
    {
        printf("List is empty");
        return;
    } 

    if(fptr == NULL) {
        printf("Please pass the comparator function here");
        return;
    }
    Node *temp = head;
    do
    {
        (*fptr)(temp->data);
        temp = temp->next;
    } while (temp != head);

    printf("\n");
}
