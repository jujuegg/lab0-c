#include <linux/kernel.h>
//#include <linux/bug.h>
//#include <linux/compiler.h>
//#include <linux/export.h>
#include <linux/string.h>
//#include <linux/list_sort.h>
//#include <linux/list.h>
#include <list_sort.h>
#include "list.h"
#include "queue.h"

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

__attribute__((nonnull(1, 2, 3))) static struct list_head *merge(
    list_cmp_func_t cmp,
    struct list_head *a,
    struct list_head *b,
    bool descend)
{
    struct list_head *head = NULL, **tail = &head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(a, b, descend) <= 0) {
            *tail = a;
            tail = &a->next;
            a = a->next;
            if (!a) {
                *tail = b;
                break;
            }
        } else {
            *tail = b;
            tail = &b->next;
            b = b->next;
            if (!b) {
                *tail = a;
                break;
            }
        }
    }
    return head;
}

__attribute__((nonnull(1, 2, 3, 4))) static void merge_final(
    list_cmp_func_t cmp,
    struct list_head *head,
    struct list_head *a,
    struct list_head *b,
    bool descend)
{
    struct list_head *tail = head;
    uint8_t count = 0;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (cmp(a, b, descend) <= 0) {
            tail->next = a;
            a->prev = tail;
            tail = a;
            a = a->next;
            if (!a)
                break;
        } else {
            tail->next = b;
            b->prev = tail;
            tail = b;
            b = b->next;
            if (!b) {
                b = a;
                break;
            }
        }
    }

    /* Finish linking remainder of list b on to tail */
    tail->next = b;
    do {
        if (unlikely(!++count))
            cmp(b, b, descend);
        b->prev = tail;
        tail = b;
        b = b->next;
    } while (b);

    /* And the final links to make a circular doubly-linked list */
    tail->next = head;
    head->prev = tail;
}

__attribute__((nonnull(1, 2))) void list_sort(struct list_head *head,
                                              list_cmp_func_t cmp,
                                              bool descend)
{
    struct list_head *list = head->next, *pending = NULL;
    size_t count = 0; /* Count of pending */

    if (list == head->prev) /* Zero or one elements */
        return;

    /* Convert to a null-terminated singly-linked list. */
    head->prev->next = NULL;

    do {
        size_t bits;
        struct list_head **tail = &pending;

        /* Find the least-significant clear bit in count */
        for (bits = count; bits & 1; bits >>= 1)
            tail = &(*tail)->prev;
        /* Do the indicated merge */
        if (likely(bits)) {
            struct list_head *a = *tail, *b = a->prev;

            a = merge(cmp, b, a, descend);
            /* Install the merged result in place of the inputs */
            a->prev = b->prev;
            *tail = a;
        }

        /* Move one element from input list to pending */
        list->prev = pending;
        pending = list;
        list = list->next;
        pending->next = NULL;
        count++;
    } while (list);

    /* End of input; merge together all the pending lists. */
    list = pending;
    pending = pending->prev;
    for (;;) {
        struct list_head *next = pending->prev;

        if (!next)
            break;
        list = merge(cmp, pending, list, descend);
        pending = next;
    }
    /* The final merge, rebuilding prev links */
    merge_final(cmp, head, pending, list, descend);
}

int cmp(const struct list_head *a, const struct list_head *b, bool descend)
{
    element_t *a_entry = list_entry(a, element_t, list);
    element_t *b_entry = list_entry(b, element_t, list);

    if (!descend)
        return strcmp(a_entry->value, b_entry->value) < 0 ? 0 : 1;
    else
        return strcmp(a_entry->value, b_entry->value) > 0 ? 0 : 1;
}
// EXPORT_SYMBOL(list_sort);
