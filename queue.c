#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new_head = malloc(sizeof(struct list_head));
    if (new_head) {
        new_head->next = new_head;
        new_head->prev = new_head;
    }
    return new_head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (head == NULL)
        return;
    element_t *entry = NULL, *safe = NULL;
    list_for_each_entry_safe (entry, safe, head, list) {
        q_release_element(entry);
    }

    free(head);
}


bool __q_insert(struct list_head *head,
                char *s,
                void (*add_func)(struct list_head *node,
                                 struct list_head *head))
{
    if (head) {
        element_t *new_node = malloc(sizeof(element_t));
        if (new_node) {
            size_t size = strlen(s) + 1;
            new_node->value = malloc(sizeof(char) * size);
            if (new_node->value) {
                strncpy(new_node->value, s, size);
                add_func(&new_node->list, head);
                // cppcheck-suppress memleak
                return true;
            }
            free(new_node);
        }
    }
    return false;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    return __q_insert(head, s, list_add);
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    return __q_insert(head, s, list_add_tail);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head && list_empty(head) == 0) {
        struct list_head *target = head->next;
        element_t *target_entry = list_entry(target, element_t, list);
        list_del(target);
        if (sp) {
            strncpy(sp, target_entry->value, bufsize - 1);
            sp[bufsize - 1] = '\0';
        }
        return target_entry;
    }
    return NULL;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head && list_empty(head) == 0) {
        struct list_head *target = head->prev;
        element_t *target_entry = list_entry(target, element_t, list);
        list_del(target);
        if (sp) {
            strncpy(sp, target_entry->value, bufsize - 1);
            sp[bufsize - 1] = '\0';
        }

        return target_entry;
    }
    return NULL;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    struct list_head *node = NULL;
    int size = 0;
    list_for_each (node, head) {
        size++;
    }
    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (head == NULL || list_empty(head))
        return false;
    struct list_head *slow = head->next, *fast = head->next->next;
    while (fast != head && fast->prev != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    list_del(slow);
    element_t *target = list_entry(slow, element_t, list);
    q_release_element(target);

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (head == NULL || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *odd = head->next, *even = head->next->next;

    while (odd != head && even != head) {
        struct list_head *prev = odd->prev, *next = even->next;
        odd->next = next;
        next->prev = odd;
        even->prev = prev;
        prev->next = even;
        odd->prev = even;
        even->next = odd;

        odd = next;
        even = next->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (head == NULL || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *tmp = head, *next = head->next, *prev = NULL;
    do {
        prev = tmp;
        tmp->prev = next;
        tmp = next;
        next = next->next;
        tmp->next = prev;
    } while (tmp != head);
}

int __move_k(struct list_head *head, struct list_head **node, int k)
{
    int i = 0;
    for (; i < k && (*node)->next != head; i++, *node = (*node)->next)
        ;
    return i;
}

void __reverse_k(struct list_head *start, struct list_head *end)
{
    struct list_head *l = start->prev, *r = end->next;
    struct list_head *tmp = start, *next = start->next, *prev = NULL;
    while (next != r) {
        prev = tmp;
        tmp->prev = next;
        tmp = next;
        next = next->next;
        tmp->next = prev;
    }

    l->next = end;
    end->prev = l;
    r->prev = start;
    start->next = r;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (head == NULL || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *start = head->next, *end = head;
    int i = __move_k(head, &end, k);
    while (i == k) {
        struct list_head *next_start = start, *next_end = end;

        __move_k(head, &next_start, k);
        i = __move_k(head, &next_end, k);

        __reverse_k(start, end);
        start = next_start;
        end = next_end;
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend) {}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
