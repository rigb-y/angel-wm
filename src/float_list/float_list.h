/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_FLOAT_LIST_H
#define ANGEL_FLOAT_LIST_H

#include "types.h"

#include <X11/Xlib.h>
#include <stddef.h>

typedef struct DetachedClient DetachedClient;
typedef struct Client Client;

typedef struct FloatingClients {
    DetachedClient* head;
    size_t n;
} FloatingClients;

DetachedClient* fl_find_from_win(FloatingClients*, Window);
DetachedClient* fl_find_from_client(FloatingClients*, const Client*);
_Bool fl_find(FloatingClients*, DetachedClient*);
void fl_push(FloatingClients*, DetachedClient*);
void fl_remove(FloatingClients*, DetachedClient*);
_Bool fl_empty(FloatingClients*);
_Bool fl_single(FloatingClients*);
void fl_reattach_into_cl(FloatingClients*, DetachedClient*);
DetachedClient* fl_head(FloatingClients*);
size_t fl_size(FloatingClients*);
DetachedClient* fl_get_parent(FloatingClients*, DetachedClient*);

void fl_destroy(FloatingClients*);

void fl_cancel_pending_unmaps(FloatingClients*);
void fl_cancel_cancel_pending_unmaps(FloatingClients*);
void fl_set_mapped_or_unmapped_from_workspace_switch(
    FloatingClients*,
    SetMappedFromSwitchFn,
    _Bool
);

void fl_map_unmap(FloatingClients*, MapClientFn, _Bool);

#endif
