#ifndef _LINUX_LIST_SORT_H
#define _LINUX_LIST_SORT_H

#include <stdbool.h>
#include "list.h"
#include "stdint.h"

typedef int
    __attribute__((nonnull(1, 2))) (*list_cmp_func_t)(const struct list_head *,
                                                      const struct list_head *,
                                                      bool);

__attribute__((nonnull(1, 2))) void list_sort(struct list_head *head,
                                              list_cmp_func_t cmp,
                                              bool descend);

__attribute__((nonnull(1, 2))) int cmp(const struct list_head *a,
                                       const struct list_head *b,
                                       bool descend);
#endif
