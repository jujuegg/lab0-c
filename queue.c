#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

#define list_for_each_safe_reverse(node, safe, head)             \
    for (node = (head)->prev, safe = node->prev; node != (head); \
         node = safe, safe = node->prev)

/*declaration*/
element_t *new_node(char *s);

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));

    if (!head) {
        free(head);
        return NULL;
    }

    INIT_LIST_HEAD(head);

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    element_t *entry, *safe;

    list_for_each_entry_safe (entry, safe, l, list) {
        free(entry->value);
        free(entry);
    }
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (__glibc_unlikely(!head || !s))
        return false;

    element_t *node = new_node(s);
    if (!node)
        return false;

    list_add(&node->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (__glibc_unlikely(!head || !s))
        return false;

    element_t *node = new_node(s);
    if (!node) {
        return false;
    }

    list_add_tail(&node->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (__glibc_unlikely(!head || list_empty(head)))
        return NULL;

    struct list_head *first_ptr = head->next;
    element_t *node = list_entry(first_ptr, element_t, list);

    list_del(first_ptr);

    if (sp) {
        strncpy(sp, node->value, bufsize);
        if (strlen(sp) >= bufsize)
            sp[bufsize - 1] = '\0';
    }

    return node;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (__glibc_unlikely(!head || list_empty(head)))
        return NULL;

    struct list_head *last_ptr = head->prev;
    element_t *node = list_entry(last_ptr, element_t, list);

    list_del(last_ptr);

    if (sp) {
        strncpy(sp, node->value, bufsize);
        if (strlen(sp) >= bufsize)
            sp[bufsize - 1] = '\0';
    }

    return node;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;

    return len;
}

/* Delete the middle node in queue */
// https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    int mid;
    struct list_head *mid_ptr = head->next;

    mid = q_size(head) / 2;
    for (int i = 0; i < mid; ++i) {
        mid_ptr = mid_ptr->next;
    }
    list_del(mid_ptr);

    element_t *node = list_entry(mid_ptr, element_t, list);
    free(node->value);
    free(node);

    return true;
}

/* Delete all nodes that have duplicate string */
// https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    bool last_dup = false;
    struct list_head *node, *safe;

    list_for_each_safe (node, safe, head) {
        element_t *entry = list_entry(node, element_t, list);
        if (safe != head &&
            !strcmp(entry->value, list_entry(safe, element_t, list)->value)) {
            last_dup = true;
            list_del(node);
            free(entry->value);
            free(entry);
        } else if (last_dup) {
            last_dup = false;
            list_del(node);
            free(entry->value);
            free(entry);
        }
    }

    return true;
}

/* Swap every two adjacent nodes */
// https://leetcode.com/problems/swap-nodes-in-pairs/
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node, *safe;

    list_for_each_safe (node, safe, head) {
        if (safe == head)
            return;

        node->prev->next = safe;
        safe->next->prev = node;
        node->next = safe->next;
        safe->next = node;
        safe->prev = node->prev;
        node->prev = safe;

        safe = node->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node, *safe;

    list_for_each_safe (node, safe, head) {
        list_move(node, head);
    }

    return;
}

/* Reverse the nodes of the list k at a time */
// https://leetcode.com/problems/reverse-nodes-in-k-group/
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head))
        return;

    if (k == 1) {
        q_reverse(head);
        return;
    }

    struct list_head *node, *safe;
    int count = 0;
    LIST_HEAD(temp);

    q_reverse(head);
    list_splice_tail_init(head, &temp);

    list_for_each_safe_reverse(node, safe, &temp)
    {
        count++;
        if (count == k) {
            LIST_HEAD(remains);
            list_cut_position(&remains, &temp, safe);
            list_splice_tail_init(&temp, head);
            list_splice_tail(&remains, &temp);
            count = 0;
        }
    }

    if (!list_empty(&temp)) {
        q_reverse(&temp);
        list_splice_tail(&temp, head);
    }
}

void merge_two_sorted_list(struct list_head *left_head,
                           struct list_head *right_head,
                           struct list_head *head,
                           bool descend)
{
    while (!list_empty(left_head) && !list_empty(right_head)) {
        element_t *left_entry = list_entry(left_head->next, element_t, list);
        element_t *right_entry = list_entry(right_head->next, element_t, list);

        if (!descend) {
            if (strcmp(left_entry->value, right_entry->value) <= 0)
                list_move_tail(left_head->next, head);
            else
                list_move_tail(right_head->next, head);
        } else {
            if (strcmp(left_entry->value, right_entry->value) >= 0)
                list_move_tail(left_head->next, head);
            else
                list_move_tail(right_head->next, head);
        }
    }

    if (list_empty(left_head))
        list_splice_tail_init(right_head, head);
    else
        list_splice_tail_init(left_head, head);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *slow = head, *fast = head;

    do {
        fast = fast->next->next;
        slow = slow->next;
    } while (!(fast == head || fast->next == head));

    LIST_HEAD(left_head);
    LIST_HEAD(right_head);
    list_splice_tail_init(head, &right_head);
    list_cut_position(&left_head, &right_head, slow);

    q_sort(&left_head, descend);
    q_sort(&right_head, descend);
    merge_two_sorted_list(&left_head, &right_head, head, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
// https://leetcode.com/problems/remove-nodes-from-linked-list/
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    int size = 1;
    struct list_head *node, *head_safe, *first = head->next;
    element_t *temp_entry, *temp_safe;
    LIST_HEAD(temp);

    list_splice_tail_init(head, &temp);
    list_cut_position(head, &temp, first);

    list_for_each_entry_safe (temp_entry, temp_safe, &temp, list) {
        list_for_each_safe_reverse(node, head_safe, head)
        {
            element_t *head_entry = list_entry(node, element_t, list);

            if (strcmp(temp_entry->value, head_entry->value) < 0) {
                list_del(node);
                free(head_entry->value);
                free(head_entry);
                size--;

                if (head_safe == head) {
                    list_move_tail(&temp_entry->list, head);
                    size++;
                    break;
                }
            } else {
                list_move_tail(&temp_entry->list, head);
                size++;
                break;
            }
        }
    }

    return size;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
// https://leetcode.com/problems/remove-nodes-from-linked-list/
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    int size = 1;
    struct list_head *node, *head_safe, *first = head->next;
    element_t *temp_entry, *temp_safe;
    LIST_HEAD(temp);

    list_splice_tail_init(head, &temp);
    list_cut_position(head, &temp, first);

    list_for_each_entry_safe (temp_entry, temp_safe, &temp, list) {
        list_for_each_safe_reverse(node, head_safe, head)
        {
            element_t *head_entry = list_entry(node, element_t, list);

            if (strcmp(temp_entry->value, head_entry->value) > 0) {
                list_del(node);
                free(head_entry->value);
                free(head_entry);
                size--;

                if (head_safe == head) {
                    list_move_tail(&temp_entry->list, head);
                    size++;
                    break;
                }
            } else {
                list_move_tail(&temp_entry->list, head);
                size++;
                break;
            }
        }
    }

    return size;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
// https://leetcode.com/problems/merge-k-sorted-lists/
int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;

    if (list_is_singular(head))
        return q_size(list_first_entry(head, queue_contex_t, chain)->q);

    LIST_HEAD(ans);
    queue_contex_t *q_ptr = NULL;
    queue_contex_t *last_q_ptr = list_last_entry(head, queue_contex_t, chain);
    int size;

    list_for_each_entry (q_ptr, head, chain) {
        if (q_ptr == last_q_ptr) {
            list_splice_init(q_ptr->q, &ans);
            break;
        }

        list_splice_init(q_ptr->q, &ans);
        list_splice_init(last_q_ptr->q, &ans);
        last_q_ptr = list_entry(last_q_ptr->chain.prev, queue_contex_t, chain);
    }

    q_sort(&ans, descend);
    size = q_size(&ans);
    list_splice_init(&ans, list_first_entry(head, queue_contex_t, chain)->q);

    return size;
}

/* Constructor of a node with data */
element_t *new_node(char *s)
{
    element_t *node = malloc(sizeof(element_t));
    if (!node)
        return NULL;

    node->value = strdup(s);
    if (__glibc_likely(!node->value)) {
        free(node);
        return NULL;
    }

    return node;
}
