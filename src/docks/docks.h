/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_DOCKS_H
#define ANGEL_DOCKS_H

#include "types.h"

#include <X11/Xlib.h>
#include <stddef.h>

typedef struct Client Client;

typedef struct Strut {
    unsigned long left;
    unsigned long right;
    unsigned long top;
    unsigned long bottom;

    unsigned long left_start_y;
    unsigned long left_end_y;
    unsigned long right_start_y;
    unsigned long right_end_y;
    unsigned long top_start_x;
    unsigned long top_end_x;
    unsigned long bottom_start_x;
    unsigned long bottom_end_x;

    _Bool valid;
    _Bool partial;
} Strut;

void fill_strut(
    Strut*,
    unsigned long,
    unsigned long,
    unsigned long,
    unsigned long
); 

void fill_strut_partial(
    Strut*,
    unsigned long,
    unsigned long,
    unsigned long,
    unsigned long,
    unsigned long,
    unsigned long,
    unsigned long,
    unsigned long,
    unsigned long,
    unsigned long,
    unsigned long,
    unsigned long
);

typedef struct Dock {
    Client* client;
    Strut strut;

    _Bool all_workspaces;
    int workspace;
    struct Dock* next;
} Dock;

void fill_dock(Dock*, Client*, Strut);
Dock* create_dock(Client*, Strut);
_Bool dock_strut_valid(const Dock*);
_Bool dock_has_strut_partial(const Dock*);

Client* get_client_from_dock(const Dock*);
void set_dock_client(Dock*, Client*);
const Strut* get_dock_strut(const Dock*);
void set_dock_strut(Dock*, Strut);

void set_dock_on_all_workspaces(Dock*, _Bool);
_Bool get_dock_on_all_workspaces(Dock*);
void set_dock_workspace(Dock*, int);
int get_dock_workspace(Dock*);

typedef struct Docks {
    Dock* head;
    size_t n;
} Docks;

Dock* docks_find_from_client(const Docks*, const Client*);
Dock* docks_find_from_win(const Docks*, Window);
_Bool docks_find(const Docks*, const Dock*);
void docks_push(Docks*, Dock*);
void docks_remove(Docks*, Dock*);
_Bool docks_empty(const Docks*);
size_t docks_size(const Docks*);
void docks_destroy(Docks*);
void dl_cancel_pending_unmaps(Docks*);
void dl_cancel_cancel_pending_unmaps(Docks*);
void dl_set_mapped_or_unmapped_from_workspace_switch(
    Docks*,
    SetMappedFromSwitchFn,
    _Bool
);
void dl_map_unmap(Docks*, MapClientFn, _Bool);

#endif
