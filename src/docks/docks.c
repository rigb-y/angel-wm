/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "docks.h"
#include "client.h"
#include "types.h"
#include "workspaces.h"
#include "ewmh.h"

#include <X11/Xlib.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

static Dock* get_dock_parent(const Docks*, const Dock*);
static Dock* get_dock_tail(const Docks*);

void fill_strut(
    Strut* strut,
    unsigned long left,
    unsigned long right,
    unsigned long top,
    unsigned long bottom
) {
    if (strut == NULL) return; 

    strut->left = left;
    strut->right = right;
    strut->top = top;
    strut->bottom = bottom;

    strut->valid = true;
    strut->partial = false;
}

void fill_strut_partial(
    Strut* strut,
    unsigned long left,
    unsigned long right,
    unsigned long top,
    unsigned long bottom,
    unsigned long left_start_y,
    unsigned long left_end_y,
    unsigned long right_start_y,
    unsigned long right_end_y,
    unsigned long top_start_x,
    unsigned long top_end_x,
    unsigned long bottom_start_x,
    unsigned long bottom_end_x
) {
    if (strut == NULL) return;

    strut->left = left;
    strut->right = right;
    strut->top = top;
    strut->bottom = bottom;

    strut->top_start_x = top_start_x;
    strut->left_start_y = left_start_y;

    strut->top_end_x = top_end_x;
    strut->right_start_y = right_start_y;

    strut->bottom_start_x = bottom_start_x;
    strut->left_end_y = left_end_y;

    strut->right_end_y = right_end_y;
    strut->bottom_end_x = bottom_end_x;

    strut->valid = true;
    strut->partial = true;
}

_Bool dock_strut_valid(const Dock* dock) {
    if (dock == NULL) return false;
    return dock->strut.valid;
}

_Bool dock_has_strut_partial(const Dock* dock) {
    if (dock == NULL) return false;
    return dock->strut.partial;
}

void fill_dock(Dock* dock, Client* client, Strut strut) {
    if (dock == NULL || !strut.valid) return;
    
    dock->client = client;
    dock->strut = strut;
    dock->next = NULL;
}

Dock* create_dock(Client* client, Strut strut) {
    if (client == NULL || !strut.valid) return NULL;

    Dock* dock;
    if ((dock = calloc(1, sizeof(Dock))) == NULL)
        return NULL;

    fill_dock(dock, client, strut);

    int wsn = ewmh_get_workspace_num(
        get_client_win(client)
    );

    if (wsn == -1)
        set_dock_on_all_workspaces(dock, true);

    else if (wsn == -2 || !workspace_is_valid(wsn))
        set_dock_workspace(
            dock,
            get_client_on_workspace(client)
        );

    else
        set_dock_workspace(dock, wsn);

    return dock;
}

Client* get_client_from_dock(const Dock* dock) {
    if (dock == NULL)
        return NULL;

    return dock->client;
}

void set_dock_client(Dock* dock, Client* client) {
    if (dock == NULL || client == NULL)
        return;

    dock->client = client;
}

const Strut* get_dock_strut(const Dock* dock) {
    if (dock == NULL)
        return NULL;

    return &dock->strut;
}

void set_dock_strut(Dock* dock, Strut strut) {
    if (dock == NULL)
        return;

    dock->strut = strut;
}

void set_dock_on_all_workspaces(Dock* dock, _Bool all) {
    if (dock == NULL) return;
    dock->all_workspaces = all;
}

_Bool get_dock_on_all_workspaces(Dock* dock) {
    if (dock == NULL) return false;
    return dock->all_workspaces;
}

void set_dock_workspace(Dock* dock, int ws) {
    if (dock == NULL || !workspace_is_valid(ws))
        return;
    dock->workspace = ws;
}

int get_dock_workspace(Dock* dock) {
    if (dock == NULL) return 0;
    return dock->workspace;
}

Dock* docks_find_from_client(const Docks* docks, const Client* client) {
    if (docks == NULL || client == NULL)
        return NULL;

    Dock* curr = docks->head;
    while (curr != NULL && get_client_from_dock(curr) != client)
        curr = curr->next;

    return curr;
}

Dock* docks_find_from_win(const Docks* docks, Window win) {
    if (docks == NULL || win == None)
        return NULL;

    Dock* curr = docks->head;
    while (curr != NULL 
        && get_client_win(
            get_client_from_dock(curr)
        ) != win
    )
        curr = curr->next;

    return curr;
}

_Bool docks_find(const Docks* docks, const Dock* dock) {
    if (docks == NULL || dock == NULL) 
        return false;

    Dock* curr = docks->head;
    while (curr != NULL) {
        if (curr == dock)
            return true;
        curr = curr->next;
    }

    return false;
}

Dock* get_dock_parent(const Docks* docks, const Dock* dock) {
    if (docks == NULL 
        || dock == NULL 
        || !docks_find(docks, dock) 
        || docks->head == dock
    ) return NULL;

    Dock* curr = docks->head;
    while (curr != NULL && curr->next != dock)
        curr = curr->next;

    return curr;
}

Dock* get_dock_tail(const Docks* docks) {
    if (docks == NULL) return NULL;

    if (docks->head == NULL)
        return NULL;

    Dock* curr = docks->head;
    while (curr->next != NULL)
        curr = curr->next;
    return curr;
}

void docks_push(Docks* docks, Dock* dock) {
    if (docks == NULL || dock == NULL || docks_find(docks, dock)) 
        return;

    if (docks->head == NULL) {
        docks->head = dock;
        docks->head->next = NULL;
        ++docks->n;
        return;
    }

    Dock* tail = get_dock_tail(docks);
    tail->next = dock;
    dock->next = NULL;
    ++docks->n;
}

void docks_remove(Docks* docks, Dock* dock) {
    if (docks == NULL 
        || dock == NULL 
        || !docks_find(docks, dock)
    ) return;

    if (docks->head == dock) {
        Dock* head = docks->head;
        docks->head = docks->head->next;
        head->next = NULL;
        --docks->n;
        return;
    }

    Dock* parent = get_dock_parent(docks, dock);
    parent->next = dock->next;
    dock->next = NULL;
    --docks->n;
}

_Bool docks_empty(const Docks* docks) {
    if (docks == NULL) return true;
    return docks->n == 0;
}

size_t docks_size(const Docks* docks) {
    if (docks == NULL) return 0;
    return docks->n;
}

void docks_destroy(Docks* docks) {
    if (docks == NULL) return;

    Dock* curr = docks->head;
    while (curr != NULL) {
        Dock* tmp = curr;
        curr = curr->next;

        free(tmp);
    }
}

void dl_cancel_pending_unmaps(Docks* list) {
    if (list == NULL) return;

    Dock* curr = list->head;
    while (curr != NULL)  {
        Client* client = get_client_from_dock(curr);
        if (client_state_pending_unmap(client) 
            && client_get_unmapped_from_workspace_switch(client)
        ) {
            client_set_map_after_unmap_notify(client, true);
        }

        curr = curr->next;
    }
}

void dl_cancel_cancel_pending_unmaps(Docks* list) {
    if (list == NULL) return;

    Dock* curr = list->head;
    while (curr != NULL)  {
        Client* client = get_client_from_dock(curr);
        if (client_get_map_after_unmap_notify(client)) {
            client_set_map_after_unmap_notify(client, false);
        }

        curr = curr->next;
    }
}

void dl_set_mapped_or_unmapped_from_workspace_switch(
    Docks* list,
    SetMappedFromSwitchFn fn,
    _Bool flag
) {
    if (list == NULL || fn == NULL) return; 

    Dock* curr = list->head;
    while (curr != NULL) {
        fn(
            get_client_from_dock(curr),
            flag
        );
        curr = curr->next;
    }
}

void dl_map_unmap(Docks* list, MapClientFn map_fn, _Bool manage_cont) {
    if (list == NULL || map_fn == NULL) return;

    Dock* dock_curr = list->head;
    while (dock_curr != NULL) {
        map_fn(get_client_from_dock(dock_curr), manage_cont);
        dock_curr = dock_curr->next;
    }
}
