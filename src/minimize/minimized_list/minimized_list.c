/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "minimized_list.h"
#include "minimized_client.h"
#include "client.h"
#include "workspaces.h"
#include "client_list.h"
#include "float_list.h"
#include "detached.h"
#include "windows.h"
#include "monitor.h"
#include "monitors.h"
#include "icccm.h"

#include <X11/Xutil.h>
#include <stddef.h>
#include <stdlib.h>

static void reattach_into_cl(Monitor*, MinimizedClient*);
static void reattach_into_fl(Monitor*, MinimizedClient*);

MinimizedClient* ml_find_from_win(MinimizedList* list, Window win) {
    if (list == NULL) return NULL;

    for (MinimizedClient* curr = list->head; curr != NULL; curr=curr->next) {
        if (get_client_win(get_client_from_minimized(curr)) == win) 
            return curr;
    }

    return NULL;
}

MinimizedClient* ml_find_from_client(MinimizedList* list, const Client* client) {
    if (list == NULL || client == NULL) return NULL;

    for (MinimizedClient* curr = list->head; curr != NULL; curr=curr->next) {
        if (get_client_from_minimized(curr) == client) 
            return curr;
    }

    return NULL;
}

_Bool ml_find(MinimizedList* list, MinimizedClient* minimized) {
    if (list == NULL || minimized == NULL) return false;

    for (MinimizedClient* start = list->head; start != NULL; start=start->next) {
        if (start == minimized)
            return true;
    }

    return false;
}

void ml_push(MinimizedList* list, MinimizedClient* minimized) {
    if (list == NULL || minimized == NULL || ml_find(list, minimized)) return;

    minimized->next = NULL;
    minimized->prev = NULL;

    MinimizedClient* next = list->head;
    list->head = minimized;
    list->head->next = next;
    list->head->prev = NULL;

    if (next)
        next->prev = minimized;

    ++list->n;

    client_set_minimized(
        get_client_from_minimized(minimized),
        true
    );
}

void ml_remove(MinimizedList* list, MinimizedClient* minimized) {
    if (list == NULL || minimized == NULL || !ml_find(list, minimized)) return;

    if (list->head == minimized) {
        list->head = list->head->next;
        minimized->next = NULL;
        minimized->prev = NULL;
        --list->n;

        client_set_minimized(
            get_client_from_minimized(minimized),
            false
        );

        return;
    }

    MinimizedClient* parent = minimized->prev;
    parent->next = minimized->next;
    minimized->next = NULL;
    minimized->prev = NULL;

    if (parent->next)
        parent->next->prev = parent;

    --list->n;
    client_set_minimized(
        get_client_from_minimized(minimized),
        false
    );
}

_Bool ml_empty(MinimizedList* list) {
    if (list == NULL) return true;
    return list->n == 0;
}

_Bool ml_single(MinimizedList* list) {
    if (list == NULL) return false;
    return list->n == 1;
}

void reattach_into_cl(Monitor* monitor, MinimizedClient* minimized) {
    if (minimized == NULL) return;

    Client* client = get_client_from_minimized(minimized);
    int workspace = reconcile_clients_workspace(client);
    int pos = get_client_pos_from_minimized(minimized);

    ClientList* monitor_cl = get_monitor_cl(monitor);

    if (pos == 1)
        cl_push(get_monitor_cl(monitor), client);

    else if (pos > cl_size(monitor_cl) || pos <= 0)
        cl_append(get_monitor_cl(monitor), client);

    else cl_insert_after(get_monitor_cl(monitor), client, pos-1);
}

void reattach_into_fl(Monitor* monitor, MinimizedClient* minimized) {
    if (minimized == NULL) return;

    int workspace = reconcile_clients_workspace(
        get_client_from_minimized(minimized)
    );

    DetachedClient* detached = create_detached_client(
        get_client_from_minimized(minimized),
        get_client_pos_from_minimized(minimized),
        ORIGIN_ML
    );

    if (detached == NULL)
        return;

    fl_push(
        get_monitor_fl(monitor),
        detached
    );
}

void ml_reattach(MinimizedList* list, MinimizedClient* minimized) {
    if (list == NULL || minimized == NULL || !ml_find(list, minimized)) return;

    Client* client = get_client_from_minimized(minimized);
    Monitor* monitor = get_monitor_from_list_membership(
        get_workspace_monitors(
            get_client_on_workspace(
                client
            )
        ), 
        client
    );

    if (monitor == NULL)
        return;

    ml_remove(list, minimized);

    switch (get_minimized_list_origin(minimized)) {
        case ORIGIN_UNKNOWN:
        case ORIGIN_CL:
            reattach_into_cl(monitor, minimized);
            break;
        case ORIGIN_FL:
            reattach_into_fl(monitor, minimized);
            break;

        default:
            break;
    }

    set_wm_state(get_client_win(client), NormalState);
    free(minimized);
}

void ml_destroy(MinimizedList* list) {
    if (list == NULL) return;

    MinimizedClient* curr = list->head; 

    while (curr != NULL) {
        MinimizedClient* tmp = curr;
        curr=curr->next;

        Client* client = get_client_from_minimized(tmp);
        Window win = get_client_win(client);
        ungrab_left_click(get_client_win(client));
        disable_drag(win);
        disable_mouse_resize(win);

        free(tmp);
    }

    list->head = NULL;
    list->n = 0;
}

MinimizedClient* ml_head(MinimizedList* list) {
    if (list == NULL) return NULL;
    return list->head;
}

size_t ml_size(MinimizedList* list) {
    if (list == NULL) return 0;
    return list->n;
}

int ml_get_client_position(MinimizedList* list, MinimizedClient* minimized) {
    if (list == NULL || minimized == NULL || !ml_find(list, minimized)) return 0;

    int pos = 1;
    MinimizedClient* curr = list->head;
    while (curr != NULL && curr != minimized) {
        ++pos;
        curr=curr->next;
    }

    return pos;
}

void ml_cancel_pending_unmaps(MinimizedList* list) {
    if (list == NULL) return;

    MinimizedClient* curr = ml_head(list);
    while (curr != NULL)  {
        Client* client = get_client_from_minimized(curr);
        if (client_state_pending_unmap(client) 
            && client_get_unmapped_from_workspace_switch(client)
        ) {
            client_set_map_after_unmap_notify(client, true);
        }

        curr = curr->next;
    }
}

void ml_cancel_cancel_pending_unmaps(MinimizedList* list) {
    if (list == NULL) return;

    MinimizedClient* curr = ml_head(list);
    while (curr != NULL)  {
        Client* client = get_client_from_minimized(curr);
        if (client_get_map_after_unmap_notify(client)) {
            client_set_map_after_unmap_notify(client, false);
        }

        curr = curr->next;
    }
}

void ml_set_mapped_or_unmapped_from_workspace_switch(
    MinimizedList* list,
    SetMappedFromSwitchFn fn,
    _Bool flag
) {
    if (list == NULL || fn == NULL) return; 

    MinimizedClient* curr = ml_head(list);
    while (curr != NULL) {
        fn(
            get_client_from_minimized(curr),
            flag
        );
        curr = curr->next;
    }
}

void ml_map_unmap(MinimizedList* list, MapClientFn map_fn, _Bool manage_cont) {
    if (list == NULL || map_fn == NULL) return;

    MinimizedClient* minimized_curr = ml_head(list);
    while (minimized_curr != NULL) {
        map_fn(get_client_from_minimized(minimized_curr), manage_cont);
        minimized_curr = minimized_curr->next;
    }
}
