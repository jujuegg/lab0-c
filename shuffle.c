#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "shuffle.h"

void q_shuffle(struct list_head *head)
{
    if (!head || list_is_singular(head))
        return;

    struct list_head *completed = head;
    int cnt = q_size(head);

    while (cnt > 0) {
        int random_index = rand() % cnt;
        struct list_head *current = completed->next;
        struct list_head *selected = current;
        while (random_index--)
            selected = selected->next;

        swap(current, selected);

        completed = completed->next;
        cnt--;
    }
    return;
}

void swap(struct list_head *n1, struct list_head *n2)
{
    if (n1 == n2)
        return;

    struct list_head *n1_next = n1->next;
    struct list_head *n2_next = n2->next;

    if (n1->next != n2)
        list_move_tail(n1_next, n2);

    list_move_tail(n2_next, n1);
}
