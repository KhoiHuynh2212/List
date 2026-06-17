#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "instrusive_list.h"

typedef struct
{
    int id;
    list link;
} Node; 

static void test_api(void) {
    list head = LIST_INIT(head), list2 = LIST_INIT(list2);
    Node node = {.id = 0, .link = LIST_INIT(node.link)}; 
    Node node2 = {.id = 1, .link = LIST_INIT(node2.link)};
    assert(list_init(&head) == &head); 
    assert(list_entry_offset(NULL, 0) == NULL); 
    assert(list_entry_offset(NULL, offsetof(Node, link)) == NULL);
    assert(list_entry(NULL, Node, link) == NULL); 
    assert(list_entry(&node.link, Node, link) == &node);
    assert(list_is_linked(&node.link) == false);
    assert(list_is_empty(&node.link) == true);

    // basic insert - link and unlink 
    list_add_after(&head, &node.link); 
    list_add_after(&node.link, &node2.link);
    assert(list_is_linked(&node.link));
    assert(list_is_first(&node.link, &head) == true);
    assert(list_is_last(&node.link, &head) == false);
    assert(list_is_empty(&head) == false);

    list_unlink_stale(&node.link); 
    assert(list_is_linked(&node.link) == true); // still remember the link
    assert(list_length(&head) == 1);

    list_unlink(&node2.link);
    assert(list_is_empty(&head));
    assert(!list_is_linked(&node2.link)); 

   
    /* swap and splice operation */
 
    // rebuild list1: node -> node2, in order to test swap on a clean 2-node list
    list_init(&node.link);
    list_init(&node2.link);
    list_add_tail(&head, &node.link);
    list_add_tail(&head, &node2.link);
    assert(list_length(&head) == 2);
    assert(list_is_first(&node.link, &head) == true);
    assert(list_is_last(&node2.link, &head) == true);
 
    // swap adjacent nodes: node <-> node2 should become node2 <-> node
    list_swap(&node.link, &node2.link);
    assert(list_is_first(&node2.link, &head) == true);
    assert(list_is_last(&node.link, &head) == true);
    assert(list_length(&head) == 2); // swap must not change list size

        // swap back so the list is in a known state for the next block
    list_swap(&node2.link, &node.link);
    assert(list_is_first(&node.link, &head) == true);
    assert(list_is_last(&node2.link, &head) == true);
 
    // swap with a third, non-adjacent node to exercise the general case
    Node node3 = {.id = 3, .link = LIST_INIT(node3.link)};
    list_add_tail(&head, &node3.link); // head: node -> node2 -> node3
    assert(list_length(&head) == 3);
    list_swap(&node.link, &node3.link); // expect: node3 -> node2 -> node
    assert(list_is_first(&node3.link, &head) == true);
    assert(list_is_last(&node.link, &head) == true);
    assert(list_length(&head) == 3);
 
    // restore head to node -> node2 -> node3 ordering for splice test
    list_unlink(&node.link);
    list_unlink(&node2.link);
    list_unlink(&node3.link);
    assert(list_is_empty(&head));
    list_add_tail(&head, &node.link);
    list_add_tail(&head, &node2.link);
    list_add_tail(&head, &node3.link);
 
    // splice: move head's contents onto the end of list2
    Node node4 = {.id = 4, .link = LIST_INIT(node4.link)};
    list_add_tail(&list2, &node4.link); // list2: node4
    assert(list_length(&list2) == 1);
 
    list_splice(&list2, &head); // list2: node4 -> node -> node2 -> node3 ; list1: empty
    assert(list_is_empty(&head) == true);
    assert(list_length(&list2) == 4);
    assert(list_is_first(&node4.link, &list2) == true);
    assert(list_is_last(&node3.link, &list2) == true);
 
    // splicing an empty source must be a safe no-op
    list_splice(&list2, &head); // head is empty
    assert(list_length(&list2) == 4);
    assert(list_is_empty(&head) == true);
 
    /* list accessors */
 
    assert(list_is_first(&node4.link, &list2) == true);
    assert(list_is_last(&node4.link, &list2) == false);
    assert(list_is_first(&node3.link, &list2) == false);
    assert(list_is_last(&node3.link, &list2) == true);
    assert(list_length(&list2) == 4);
    assert(list_length(&head) == 0); // empty anchor
 
    // list_replace: swap node2 out for a freshly detached node5
    Node node5 = {.id = 5, .link = LIST_INIT(node5.link)};
    list_replace(&node2.link, &node5.link);
    assert(list_length(&list2) == 4); // size unchanged — one-for-one swap
    assert(list_is_linked(&node2.link) == true); // old node retains its stale pointers (not reinitialized)
    assert(list_entry(&node5.link, Node, link)->id == 5);

        // confirm ordering after replace: node4 -> node -> node5 -> node3
    {
        list* pos = list2.next;
        assert(list_entry(pos, Node, link)->id == 4); pos = pos->next;
        assert(list_entry(pos, Node, link)->id == 0); pos = pos->next;
        assert(list_entry(pos, Node, link)->id == 5); pos = pos->next;
        assert(list_entry(pos, Node, link)->id == 3); pos = pos->next;
        assert(pos == &list2); // back to anchor
    }
 
    /* direct/raw iterators */
 
    // list_for_each: raw list* traversal, caller does list_entry manually
    {
        int sum = 0;
        list* pos;
        list_for_each(pos, &list2) {
            Node* n = list_entry(pos, Node, link);
            sum += n->id;
        }
        assert(sum == 4 + 0 + 5 + 3); // node4 + node + node5 + node3
    }
 
    // list_for_each_safe: must tolerate unlinking the current node mid-loop
    {
        size_t count_before = list_length(&list2);
        list* pos;
        list* tmp;
        list_for_each_safe(pos, tmp, &list2) {
            Node* n = list_entry(pos, Node, link);
            if (n->id == 0) {
                list_unlink(pos); // unlink "node" mid-traversal
            }
        }
        assert(list_length(&list2) == count_before - 1);
        assert(list_is_linked(&node.link) == false); // confirms it was actually removed
    }
 
    // list_for_each_entry: typed traversal, no manual list_entry calls
    {
        int sum = 0;
        Node* n;
        list_for_each_entry(n, &list2, link) {
            sum += n->id;
        }
        assert(sum == 4 + 5 + 3); // node4 + node5 + node3 (node was removed above)
    }
 
    printf("all assertions passed\n");


}

int main() {
    test_api();
    return 0;
}

