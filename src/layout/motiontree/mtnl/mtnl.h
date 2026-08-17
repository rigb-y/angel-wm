/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_MTNL_H
#define ANGEL_MTNL_H

#include "node.h"

#include <stddef.h>

// Motion tree node list (for children)
typedef struct Children {
    MTNode* head;
    size_t n;
} Children;

size_t mtnl_size(Children*);
_Bool mtnl_empty(Children*);
_Bool mtnl_single(Children*);
MTNode* mtnl_find(Children*, MTNode*);
MTNode* mtnl_get_child_n(Children*, int);
int mtnl_child_number(Children*, MTNode*);
void mtnl_append(Children*, MTNode*);
void mtnl_prepend(Children*, MTNode*);
void mtnl_insert_after(Children*, MTNode*, MTNode*);
void mtnl_remove(Children*, MTNode**);
void mtnl_destroy(Children*);

#endif
