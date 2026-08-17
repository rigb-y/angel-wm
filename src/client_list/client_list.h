/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_CLIENT_LIST_H
#define ANGEL_CLIENT_LIST_H

#include "types.h"

#include <X11/Xlib.h>
#include <stddef.h>

#define FREE true
#define NO_FREE false

typedef struct Client Client;
typedef struct ClientList ClientList;

typedef struct ClientList {
    Client* head;
    Client* tail;

    size_t n;
} ClientList;

_Bool invalid_client_number(ClientList*, int);
Client* cl_head(ClientList*);
Client* cl_tail(ClientList*);
Client* cl_client_number(ClientList*, int);

Client* cl_find_parent(ClientList*, Client*);
Client* cl_find_client_from_win(ClientList*, Window);
Client* cl_find_client(ClientList*, const Client*);

size_t cl_size(ClientList*);
_Bool cl_empty(ClientList*);
_Bool cl_single(ClientList*);

void cl_append(ClientList*, Client*);
void cl_push(ClientList*, Client*);
void cl_insert_after(ClientList*, Client*, int);
void cl_remove(ClientList*, Client**, _Bool);
void cl_destroy(ClientList*);

void cl_dump(ClientList*);

void cl_swap_clients(ClientList*, Client*, Client*);
int cl_get_client_position(ClientList*, const Client*);

void cl_set_clients_resize_step(Client*, int, ResizeSetStepFn);

int cl_client_distance(ClientList*, Client*, Client*);
void cl_set_future_unmap_stay_unmapped(ClientList*);

void cl_cancel_pending_unmaps(ClientList*);
void cl_cancel_cancel_pending_unmaps(ClientList*);
void cl_set_mapped_or_unmapped_from_workspace_switch(
    ClientList*,
    SetMappedFromSwitchFn,
    _Bool
);

void cl_map_unmap(ClientList*, MapClientFn, _Bool);

#endif
