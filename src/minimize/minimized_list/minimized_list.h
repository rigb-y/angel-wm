/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_MINIMIZED_LIST_H
#define ANGEL_MINIMIZED_LIST_H

#include "types.h"

#include <X11/Xlib.h>
#include <stddef.h>

typedef struct MinimizedClient MinimizedClient;
typedef struct Client Client;

typedef struct MinimizedList {
    MinimizedClient* head;
    size_t n;
} MinimizedList;

MinimizedClient* ml_find_from_win(MinimizedList*, Window);
MinimizedClient* ml_find_from_client(MinimizedList*, const Client*);
_Bool ml_find(MinimizedList*, MinimizedClient*);
void ml_push(MinimizedList*, MinimizedClient*);
void ml_remove(MinimizedList*, MinimizedClient*);
_Bool ml_empty(MinimizedList*);
_Bool ml_single(MinimizedList*);
void ml_reattach(MinimizedList*, MinimizedClient*);
MinimizedClient* ml_head(MinimizedList*);
size_t ml_size(MinimizedList*);
int ml_get_client_position(MinimizedList*, MinimizedClient*);

void ml_destroy(MinimizedList*);

void ml_cancel_pending_unmaps(MinimizedList*);
void ml_cancel_cancel_pending_unmaps(MinimizedList*);
void ml_set_mapped_or_unmapped_from_workspace_switch(
    MinimizedList*,
    SetMappedFromSwitchFn,
    _Bool
);

void ml_map_unmap(MinimizedList*, MapClientFn, _Bool);

#endif
