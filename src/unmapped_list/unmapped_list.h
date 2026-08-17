/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_UNMAPPED_LIST_H
#define ANGEL_UNMAPPED_LIST_H

#include "types.h"

#include <X11/Xlib.h>
#include <stddef.h>

typedef struct UnmappedClient UnmappedClient;
typedef struct Client Client;

typedef struct UnmappedClients {
    UnmappedClient* head;
    size_t n;
} UnmappedClients;

UnmappedClient* ul_find_from_win(UnmappedClients*, Window);
UnmappedClient* ul_find_from_client(UnmappedClients*, const Client*);
_Bool ul_find(UnmappedClients*, UnmappedClient*);
void ul_push(UnmappedClients*, UnmappedClient*);
void ul_remove(UnmappedClients*, UnmappedClient*);
_Bool ul_empty(UnmappedClients*);
_Bool ul_single(UnmappedClients*);
void ul_reattach(UnmappedClients*, UnmappedClient*);
UnmappedClient* ul_head(UnmappedClients*);
size_t ul_size(UnmappedClients*);
UnmappedClient* ul_get_parent(UnmappedClients*, UnmappedClient*);
int ul_get_client_position(UnmappedClients*, UnmappedClient*);

void ul_destroy(UnmappedClients*);

void ul_set_all_stay_unmapped(UnmappedClients*);
void ul_set_all_can_be_mapped(UnmappedClients*);
void ul_set_mapped_or_unmapped_from_workspace_switch(
    UnmappedClients*,
    SetMappedFromSwitchFn,
    _Bool
);

void ul_cancel_pending_maps(UnmappedClients*);
void ul_cancel_cancel_pending_maps(UnmappedClients*);

void ul_map_unmap(UnmappedClients*, MapClientFn, _Bool);
void ul_reattach_dock(UnmappedClients*, UnmappedClient*);

#endif
