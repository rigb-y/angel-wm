/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "unmapped_list.h"
#include "unmapped_client.h"
#include "client.h"
#include "client_list.h"
#include "float_list.h"
#include "detached.h"
#include "minimized_list.h"
#include "minimized_client.h"
#include "workspaces.h"
#include "windows.h"
#include "monitors.h"
#include "monitor.h"
#include "docks.h"
#include "ewmh.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

static void reattach_into_cl(Monitor*, UnmappedClient*);
static void reattach_into_fl(Monitor*, UnmappedClient*);
static void reattach_into_ml(Monitor*, UnmappedClient*);

UnmappedClient* ul_find_from_win(UnmappedClients* list, Window win) {
    if (list == NULL) return NULL;

    UnmappedClient* curr = list->head;
    while (curr != NULL) {
        if (win == get_client_win(get_client_from_unmapped(curr)))
            return curr;

        curr = curr->next;
    }

    return NULL;
}

UnmappedClient* ul_find_from_client(UnmappedClients* list, const Client* client) {
    if (list == NULL) return NULL;

    UnmappedClient* curr = list->head;
    while (curr != NULL) {
        if (client == get_client_from_unmapped(curr))
            return curr;

        curr = curr->next;
    }

    return NULL;
}

_Bool ul_find(UnmappedClients* list, UnmappedClient* unmapped) {
    if (list == NULL) return false;

    UnmappedClient* curr = list->head;
    while (curr != NULL) {
        if (unmapped == curr)
            return true;

        curr = curr->next;
    }

    return false;
}

void ul_push(UnmappedClients* list, UnmappedClient* unmapped) {
    if (list == NULL || unmapped == NULL || ul_find(list, unmapped)) return;

    unmapped->next = NULL;

    UnmappedClient* next = list->head;
    list->head = unmapped;
    list->head->next = next;
    ++list->n;
}

void ul_remove(UnmappedClients* list, UnmappedClient* unmapped) {
    if (list == NULL || unmapped == NULL || !ul_find(list, unmapped)) return;

    if (list->head == unmapped) {
        list->head = list->head->next;
        --list->n;

        unmapped->next = NULL;

        return;
    }

    UnmappedClient* parent = ul_get_parent(list, unmapped);
    parent->next = unmapped->next;
    unmapped->next = NULL;
    --list->n;
}

_Bool ul_empty(UnmappedClients* list) {
    if (list == NULL) return true;
    return list->n == 0;
}

_Bool ul_single(UnmappedClients* list) {
    if (list == NULL) return false;
    return list->n == 1;
}

void ul_reattach_dock(UnmappedClients* list, UnmappedClient* unmapped) {
    if (list == NULL || unmapped == NULL) return;

    Client* client = get_client_from_unmapped(unmapped);
    int workspace = get_client_on_workspace(client);

    Monitor* monitor = get_monitor_from_list_membership(
        get_workspace_monitors(
            workspace
        ), 
        client
    );

    ul_remove(list, unmapped);

    Dock* dock = create_dock(
        client, 
        ewmh_get_dock_strut(
            get_client_win(client)
        )
    );
    
    if (dock == NULL)
        return;

    docks_push(
        get_monitor_dl(monitor),
        dock
    );

    free(unmapped);
}

void ul_reattach(UnmappedClients* list, UnmappedClient* unmapped) {
    if (list == NULL || unmapped == NULL || !ul_find(list, unmapped)) return;

    Client* client = get_client_from_unmapped(unmapped);
    Monitor* monitor = get_monitor_from_list_membership(
        get_workspace_monitors(
            get_client_on_workspace(
                client
            )
        ), 
        client
    );

    ul_remove(list, unmapped);

    switch (get_unmapped_list_origin(unmapped)) {
        case ORIGIN_CL:
            reattach_into_cl(monitor, unmapped);
            break;
        case ORIGIN_FL:
            reattach_into_fl(monitor, unmapped);
            break;
        case ORIGIN_ML:
            reattach_into_ml(monitor, unmapped);
            break;
        default:
            break;
    }

    free(unmapped);
}

void reattach_into_cl(Monitor* monitor, UnmappedClient* unmapped) {
    if (unmapped == NULL) return;

    Client* client = get_client_from_unmapped(unmapped);
    int workspace = get_client_on_workspace(client);
    int pos = get_client_pos_from_unmapped(unmapped);

    ClientList* monitor_cl = get_monitor_cl(monitor);

    if (pos == 1)
        cl_push(monitor_cl, client);

    else if (pos > cl_size(monitor_cl) || pos <= 0)
        cl_append(monitor_cl, client);

    else cl_insert_after(monitor_cl, client, pos-1);
}

void reattach_into_fl(Monitor* monitor, UnmappedClient* unmapped) {
    if (unmapped == NULL) return;

    Client* client = get_client_from_unmapped(unmapped);
    int workspace = get_client_on_workspace(client);
    
    DetachedClient* detached = create_detached_client(
        client,
        get_client_pos_from_unmapped(unmapped),
        get_unmapped_list_origin_origin(unmapped)
    );

    if (detached == NULL)
        return;

    fl_push(
        get_monitor_fl(monitor),
        detached
    );
}

void reattach_into_ml(Monitor* monitor, UnmappedClient* unmapped) {
    if (unmapped == NULL) return;

    Client* client = get_client_from_unmapped(unmapped);
    int workspace = get_client_on_workspace(client);

    MinimizedClient* minimized = create_minimized_client(
        client,
        get_client_pos_from_unmapped(unmapped),
        get_unmapped_list_origin_origin(unmapped)
    );

    if (minimized == NULL)
        return;

    ml_push(
        get_monitor_ml(monitor),
        minimized
    );
}

UnmappedClient* ul_head(UnmappedClients* list) {
    if (list == NULL) return NULL;
    return list->head;
}

size_t ul_size(UnmappedClients* list) {
    if (list == NULL) return 0;
    return list->n;
}

UnmappedClient* ul_get_parent(UnmappedClients* list, UnmappedClient* unmapped) {
    if (list == NULL || unmapped == NULL || !ul_find(list, unmapped)) return NULL;

    UnmappedClient* curr = list->head;
    while (curr != NULL && curr->next != unmapped)
        curr = curr->next;

    return curr;
}

int ul_get_client_position(UnmappedClients* list, UnmappedClient* unmapped) {
    if (list == NULL 
        || unmapped == NULL 
        || !ul_find(list, unmapped)
    ) return 0;

    int pos = 1;
    UnmappedClient* curr = list->head;
    while (curr != NULL && curr != unmapped) {
        ++pos;
        curr=curr->next;
    }

    return pos;
}

void ul_destroy(UnmappedClients* list) {
    if (list == NULL) return;

    UnmappedClient* curr = list->head;
    while (curr != NULL) {
        UnmappedClient* tmp = curr;
        curr = curr->next;

        Client* client = get_client_from_unmapped(tmp);
        Window win = get_client_win(client);
        ungrab_left_click(get_client_win(client));
        disable_drag(win);
        disable_mouse_resize(win);

        free(tmp);
    }

    list->n = 0;
    list->head = NULL;
}

void ul_set_all_stay_unmapped(UnmappedClients* list) {
    if (list == NULL) return;

    UnmappedClient* curr = list->head;
    while (curr != NULL) {
        unmapped_set_stay_unmapped(curr, STAY_UNMAPPED);
        curr = curr->next;
    }
}

void ul_set_all_can_be_mapped(UnmappedClients* list) {
    if (list == NULL) return;

    UnmappedClient* curr = list->head;
    while (curr != NULL) {
        unmapped_set_stay_unmapped(curr, CAN_BE_MAPPED);
        client_set_future_unmap_can_be_remapped(
            get_client_from_unmapped(curr)
        );
        curr = curr->next;
    }
}

void ul_set_mapped_or_unmapped_from_workspace_switch(
    UnmappedClients* list,
    SetMappedFromSwitchFn fn,
    _Bool flag
) {
    if (list == NULL || fn == NULL) return; 

    UnmappedClient* curr = ul_head(list);
    while (curr != NULL) {
        fn(
            get_client_from_unmapped(curr),
            flag
        );
        curr = curr->next;
    }
}

void ul_cancel_pending_maps(UnmappedClients* list) {
    if (list == NULL) return;

    UnmappedClient* curr = ul_head(list);
    while (curr != NULL)  {
        Client* client = get_client_from_unmapped(curr);
        if (client_state_pending_map(client) 
            && client_get_unmapped_from_workspace_switch(client)
        ) {
            client_set_unmap_after_map_notify(
                client,
                true
            );
        }

        curr = curr->next;
    }
}

void ul_cancel_cancel_pending_maps(UnmappedClients* list) {
    if (list == NULL) return;

    UnmappedClient* curr = ul_head(list);
    while (curr != NULL) {
        Client* client = get_client_from_unmapped(curr);
        if (client_get_unmap_after_map_notify(client)) {
            client_set_unmap_after_map_notify(client, false);
        }
        curr = curr->next;
    }
}

void ul_map_unmap(UnmappedClients* list, MapClientFn map_fn, _Bool manage_cont) {
    if (list == NULL || map_fn == NULL) return;

    UnmappedClient* unmapped = ul_head(list);
    while (unmapped != NULL) {
        UnmappedClient* next = unmapped->next;
        map_fn(get_client_from_unmapped(unmapped), manage_cont);
        unmapped = next;
    }
}
