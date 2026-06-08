#ifndef LIST_H
#define LIST_H
#include <stdlib.h>
#include <string.h>

/* DEFINE_LIST FOR ALL DATA TYPES */
#define DEFINE_LIST(T)                                                 \
    typedef struct T##Node                                             \
    {                                                                  \
        T data;                                                        \
        struct T##Node *next;                                          \
        struct T##Node *prev;                                          \
    } T##Node;                                                         \
                                                                       \
    static inline void T##_init(T##Node *head)                         \
    {                                                                  \
        head->next = head;                                             \
        head->prev = head;                                             \
    }                                                                  \
                                                                       \
    static inline T##Node *T##_create(T value)                         \
    {                                                                  \
        T##Node *newNode = malloc(sizeof(T##Node));                    \
        if (newNode == NULL)                                           \
        {                                                              \
            return NULL;                                               \
        }                                                              \
        newNode->data = value;                                         \
        newNode->next = newNode;                                       \
        newNode->prev = newNode;                                       \
        return newNode;                                                \
    }                                                                  \
                                                                       \
    static inline void T##_insert(T##Node **head, T data)              \
    {                                                                  \
        T##Node *newNode = T##_create(data);                           \
                                                                       \
        if (*head == NULL)                                             \
        {                                                              \
            *head = newNode;                                           \
        }                                                              \
        else                                                           \
        {                                                              \
                                                                       \
            T##Node *first = *head;                                    \
            T##Node *last = (*head)->prev;                             \
            newNode->next = first;                                     \
            newNode->prev = last;                                      \
            last->next = newNode;                                      \
            first->prev = newNode;                                     \
        }                                                              \
    }                                                                  \
                                                                       \
    static inline int                                                  \
    T##_delete_node(T##Node **head, T value)                           \
    {                                                                  \
        if (*head == NULL)                                             \
        {                                                              \
            return -1;                                                 \
        }                                                              \
        T##Node *first = *head;                                        \
        T##Node *curr = *head;                                         \
        do                                                             \
        {                                                              \
            if (curr->data == value)                                   \
            {                                                          \
                if (curr->next == curr)                                \
                {                                                      \
                    *head = NULL;                                      \
                }                                                      \
                else                                                   \
                {                                                      \
                    curr->next->prev = curr->prev;                     \
                    curr->prev->next = curr->next;                     \
                    if (curr == *head)                                 \
                    {                                                  \
                        *head = curr->next;                            \
                    }                                                  \
                }                                                      \
                free(curr);                                            \
                return 0;                                              \
            }                                                          \
            curr = curr->next;                                         \
        } while (curr != first);                                       \
        return -1;                                                     \
    }                                                                  \
                                                                       \
    static inline T##Node *T##_search(T##Node *head, T value)          \
    {                                                                  \
        if (head == NULL)                                              \
        {                                                              \
            return NULL;                                               \
        }                                                              \
                                                                       \
        T##Node *curr = head;                                          \
        do                                                             \
        {                                                              \
            if (curr->data == value)                                   \
            {                                                          \
                return curr;                                           \
            }                                                          \
            curr = curr->next;                                         \
        } while (curr != head);                                        \
        return NULL;                                                   \
    }                                                                  \
                                                                       \
    static inline void T##_display(T##Node *head, void (*print_fn)(T)) \
    {                                                                  \
        if (head == NULL)                                              \
        {                                                              \
            printf("List is empty\n");                                 \
            return;                                                    \
        }                                                              \
        T##Node *curr = head;                                          \
        do                                                             \
        {                                                              \
            print_fn(curr->data);                                      \
            curr = curr->next;                                         \
        } while (curr != head);                                        \
        printf("\n");                                                  \
    }                                                                  \
                                                                       \
    static inline void T##_free_list(T##Node **head)                   \
    {                                                                  \
                                                                       \
        if (*head == NULL || head == NULL)                             \
        {                                                              \
            return;                                                    \
        }                                                              \
                                                                       \
        T##Node *start = *head;                                        \
        T##Node *curr = start->next;                                   \
                                                                       \
        while (curr != start)                                          \
        {                                                              \
            T##Node *next = curr->next;                                \
                                                                       \
            free(curr);                                                \
            curr = next;                                               \
        }                                                              \
                                                                       \
        free(start);                                                   \
                                                                       \
        *head = NULL;                                                  \
    }                                                                  \
                                                                       \
    static inline T##Node *T##_reverse(T##Node **head)                 \
    {                                                                  \
        if (head == NULL || *head == NULL)                             \
            return NULL;                                               \
        T##Node *curr = *head;                                         \
        T##Node *temp = NULL;                                          \
        do                                                             \
        {                                                              \
            temp = curr->prev;                                         \
            curr->prev = curr->next;                                   \
            curr->next = temp;                                         \
            curr = curr->prev;                                         \
        } while (curr != *head);                                       \
        *head = (*head)->next;                                         \
        return *head;                                                  \
    }

#define DEFINE_LIST_STRING                                              \
                                                                        \
    typedef struct strNode                                              \
    {                                                                   \
        char *data;                                                     \
        struct strNode *next;                                           \
        struct strNode *prev;                                           \
    } strNode;                                                          \
                                                                        \
    static inline strNode *str_create(const char *value)                \
    {                                                                   \
        strNode *newNode = malloc(sizeof(strNode));                     \
        if (newNode == NULL)                                            \
            return NULL;                                                \
        newNode->data = strdup(value);                                  \
        if (newNode->data == NULL)                                      \
        {                                                               \
            free(newNode);                                              \
            return NULL;                                                \
        }                                                               \
        newNode->next = newNode;                                        \
        newNode->prev = newNode;                                        \
        return newNode;                                                 \
    }                                                                   \
                                                                        \
    static inline void str_insert(strNode **head, const char *data)     \
    {                                                                   \
        strNode *newNode = str_create(data);                            \
        if (*head == NULL)                                              \
        {                                                               \
            *head = newNode;                                            \
        }                                                               \
        else                                                            \
        {                                                               \
            strNode *first = *head;                                     \
            strNode *last = (*head)->prev;                              \
            newNode->next = first;                                      \
            newNode->prev = last;                                       \
            last->next = newNode;                                       \
            first->prev = newNode;                                      \
        }                                                               \
    }                                                                   \
                                                                        \
    static inline int str_delete(strNode **head, const char *value)     \
    {                                                                   \
        if (*head == NULL)                                              \
            return -1;                                                  \
        strNode *first = *head;                                         \
        strNode *curr = *head;                                          \
        do                                                              \
        {                                                               \
            if (strcmp(curr->data, value) == 0)                         \
            {                                                           \
                if (curr->next == curr)                                 \
                {                                                       \
                    *head = NULL;                                       \
                }                                                       \
                else                                                    \
                {                                                       \
                    curr->prev->next = curr->next;                      \
                    curr->next->prev = curr->prev;                      \
                    if (curr == *head)                                  \
                        *head = curr->next;                             \
                }                                                       \
                free(curr->data);                                       \
                free(curr);                                             \
                return 0;                                               \
            }                                                           \
            curr = curr->next;                                          \
        } while (curr != first);                                        \
        return -1;                                                      \
    }                                                                   \
                                                                        \
    static inline strNode *str_search(strNode *head, const char *value) \
    {                                                                   \
        if (head == NULL)                                               \
            return NULL;                                                \
        strNode *curr = head;                                           \
        do                                                              \
        {                                                               \
            if (strcmp(curr->data, value) == 0)                         \
                return curr;                                            \
            curr = curr->next;                                          \
        } while (curr != head);                                         \
        return NULL;                                                    \
    }                                                                   \
                                                                        \
    static inline void str_isplay(strNode *head)                        \
    {                                                                   \
        if (head == NULL)                                               \
        {                                                               \
            printf("List is empty\n");                                  \
            return;                                                     \
        }                                                               \
        strNode *curr = head;                                           \
        do                                                              \
        {                                                               \
            printf("%s", curr->data);                                   \
            curr = curr->next;                                          \
        } while (curr != head);                                         \
    }                                                                   \
                                                                        \
    static inline void str_free_list(strNode **head)                    \
    {                                                                   \
                                                                        \
        if (*head == NULL || head == NULL)                              \
        {                                                               \
            return;                                                     \
        }                                                               \
                                                                        \
        strNode *start = *head;                                         \
        strNode *curr = start->next;                                    \
                                                                        \
        while (curr != start)                                           \
        {                                                               \
            strNode *next = curr->next;                                 \
            free(curr->data);                                           \
            free(curr);                                                 \
            curr = next;                                                \
        }                                                               \
        free(start->data);                                              \
        free(start);                                                    \
                                                                        \
        *head = NULL;                                                   \
    }                                                                   \
                                                                        \
    static inline strNode *str_reverse(strNode **head)                  \
    {                                                                   \
        if (head == NULL || *head == NULL)                              \
            return NULL;                                                \
        strNode *curr = *head;                                          \
        strNode *temp = NULL;                                           \
        do                                                              \
        {                                                               \
            temp = curr->prev;                                          \
            curr->prev = curr->next;                                    \
            curr->next = temp;                                          \
            curr = curr->prev;                                          \
        } while (curr != *head);                                        \
        *head = (*head)->next;                                          \
        return *head;                                                   \
    }

// this list_node is for writing before convert to macro
typedef struct list_node
{
    void *data; // convert to data types you want, dont call malloc here
    struct list_node *next;
    struct list_node *prev;
} node;

#endif
