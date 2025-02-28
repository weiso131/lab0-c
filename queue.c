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

#define __remove(link)                                                     \
    {                                                                      \
        if (head && list_empty(head) == 0) {                               \
            struct list_head *target = head->link;                         \
            element_t *target_entry = list_entry(target, element_t, list); \
            list_del(target);                                              \
            if (sp) {                                                      \
                strncpy(sp, target_entry->value, bufsize - 1);             \
                sp[bufsize - 1] = '\0';                                    \
            }                                                              \
            return target_entry;                                           \
        }                                                                  \
        return NULL;                                                       \
    }

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    __remove(next);
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    __remove(prev);
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
    if (head == NULL || list_empty(head))
        return false;
    if (list_is_singular(head))
        return true;
    element_t *entry = NULL, *safe = NULL, *delete_target = NULL;
    list_for_each_entry_safe (entry, safe, head, list) {
        if (entry->list.next != head &&
            strcmp(safe->value, entry->value) == 0) {
            list_del(&entry->list);
            q_release_element(entry);
            delete_target = safe;
        }
        if (delete_target == entry) {
            list_del(&entry->list);
            q_release_element(entry);
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    q_reverseK(head, 2);
}

/* Reverse the linklist nodes from start to end */
void __reverse(struct list_head *start, struct list_head *end)
{
    struct list_head *next = start->next, *prev = NULL;
    do {
        prev = start;
        start->prev = next;
        start = next;
        next = next->next;
        start->next = prev;
    } while (start != end);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (head == NULL || list_empty(head) || list_is_singular(head))
        return;
    __reverse(head, head);
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
    __reverse(start, end);
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

struct list_head *__merge(struct list_head *l, struct list_head *r, bool decend)
{
    struct list_head *head = NULL, **tmp = &head, *last = head;
    while (l != NULL && r != NULL) {
        char *l_value = list_entry(l, element_t, list)->value,
             *r_value = list_entry(r, element_t, list)->value;

        struct list_head **indir =
            ((strcmp(l_value, r_value) > 0) == decend) ? &l : &r;
        (*indir)->prev = last;
        *tmp = *indir;
        last = *tmp;

        *indir = (*indir)->next;
        tmp = &(*tmp)->next;
    }
    struct list_head *tail = (l != NULL) ? l : r;
    *tmp = tail;
    tail->prev = last;
    return head;
}

struct list_head *__merge_sort(struct list_head *l, bool decend)
{
    if (l == NULL || l->next == NULL) {
        return l;
    }
    struct list_head *r = l, *fast = l;
    while (fast != NULL && fast->next != NULL) {
        r = r->next;
        fast = fast->next->next;
    }

    r->prev->next = NULL;
    r->prev = NULL;

    l = __merge_sort(l, decend), r = __merge_sort(r, decend);

    return __merge(l, r, decend);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (head == NULL || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *l = head->next, *r = head->prev;
    l->prev = NULL;
    r->next = NULL;
    l = __merge_sort(l, descend);

    l->prev = head;
    head->next = l;
    for (r = l; r->next != NULL; r = r->next)
        ;
    r->next = head;
    head->prev = r;
}

int __monotonic(struct list_head *head, bool descend)
{
    if (head == NULL || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;
    int count = 0;
    const char *last = "\0";

    for (struct list_head *node = (head)->prev, *safe = node->prev;
         node != (head); node = safe, safe = node->prev) {
        element_t *entry = list_entry(node, element_t, list);
        int cmp = strcmp(entry->value, last);
        if ((cmp < 0) != descend || cmp == 0) {
            last = entry->value;
            count++;
        } else {
            list_del(&entry->list);
            q_release_element(entry);
        }
    }
    return count;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    return __monotonic(head, false);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    return __monotonic(head, true);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    if (head == NULL || list_empty(head))
        return 0;

    int chain_size = q_size(head), cnt = 0, chain_cnt = 0;
    struct list_head *chain[1000];
    struct list_head *q_head = list_first_entry(head, queue_contex_t, chain)->q;
    queue_contex_t *chain_entry = NULL;
    list_for_each_entry (chain_entry, head, chain) {
        chain[chain_cnt] = chain_entry->q->next;
        chain_entry->q->next->prev = NULL;
        chain_entry->q->prev->next = NULL;
        INIT_LIST_HEAD(chain_entry->q);
        cnt += chain_entry->size;
        chain_cnt++;
    }

    for (int i = 1; i < chain_size; i = i << 1)
        for (int j = 0; j < chain_size; j += i << 1)
            chain[j] = __merge(chain[j], chain[j + i], descend);

    q_head->next = chain[0];
    chain[0]->prev = q_head;
    struct list_head *r = NULL;
    for (r = chain[0]; r->next != NULL; r = r->next)
        ;

    r->next = q_head;
    q_head->prev = r;

    return cnt;
}
