/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "float_list.h"
#include "detached.h"
#include "client_list.h"
#include "workspaces.h"
#include "client.h"
#include "windows.h"
#include "monitor.h"
#include "monitors.h"

#include <stdlib.h>
#include <stdbool.h>

DetachedClient* fl_find_from_win(FloatingClients* list, Window win) {
    if (list == NULL) return NULL;

    for (DetachedClient* curr = list->head; curr != NULL; curr=curr->next) {
        if (get_client_win(get_client_from_detached(curr)) == win) 
            return curr;
    }

    return NULL;
}

DetachedClient* fl_find_from_client(FloatingClients* list, const Client* client) {
    if (list == NULL || client == NULL) return NULL;

    for (DetachedClient* curr = list->head; curr != NULL; curr=curr->next) {
        if (get_client_from_detached(curr) == client) 
            return curr;
    }

    return NULL;
}

_Bool fl_find(FloatingClients* list, DetachedClient* detached) {
    if (list == NULL || detached == NULL) return false;

    for (DetachedClient* start = list->head; start != NULL; start=start->next) {
        if (start == detached)
            return true;
    }

    return false;
}

DetachedClient* fl_get_parent(FloatingClients* list, DetachedClient* detached) {
    if (list == NULL || detached == NULL || !fl_find(list, detached)) return NULL;

    for (DetachedClient* start = list->head; start != NULL; start=start->next) {
        if (start->next == detached) 
            return start;
    }

    return NULL;
}

void fl_push(FloatingClients* list, DetachedClient* detached) {
    if (list == NULL || detached == NULL || fl_find(list, detached)) return;

    detached->next = NULL;

    DetachedClient* next = list->head;
    list->head = detached;
    list->head->next = next;
    ++list->n;

    client_set_float(
        get_client_from_detached(detached), 
        true
    );
}

void fl_remove(FloatingClients* list, DetachedClient* detached) {
    if (list == NULL || detached == NULL || !fl_find(list, detached)) return;

    if (list->head == detached) {
        list->head = list->head->next;
        --list->n;

        detached->next = NULL;

        client_set_float(
            get_client_from_detached(detached),
            false
        );

        return;
    }

    DetachedClient* parent = fl_get_parent(list, detached);
    parent->next = detached->next;
    detached->next = NULL;
    --list->n;

    client_set_float(
        get_client_from_detached(detached),
        false
    );
}

_Bool fl_empty(FloatingClients* list) {
    if (list == NULL) return true;
    return list->n == 0;
}

_Bool fl_single(FloatingClients* list) {
    if (list == NULL) return false;
    return list->n == 1;
}

void fl_reattach_into_cl(FloatingClients* list, DetachedClient* detached) {
    if (list == NULL || detached == NULL || !fl_find(list, detached)) return;

    Client* client = get_client_from_detached(detached);
    int workspace = reconcile_clients_workspace(client);
    Monitors* monitors = get_workspace_monitors(workspace);

    Monitor* from_monitor = get_monitor_from_list_membership(
        monitors,
        client
    );

    Monitor* to_monitor = get_monitor_from_client(
        monitors,
        client
    );

    fl_remove(list, detached);

    Window win = get_client_win(client);
    disable_drag(win);
    disable_mouse_resize(win);
    grab_left_click(win);

    ClientList* monitor_cl = get_monitor_cl(to_monitor);
    int pos = get_client_pos_from_detached(detached);

    if (from_monitor != to_monitor) {
        cl_append(monitor_cl, client);
        free(detached);
        return;
    }

    if (pos == 1)
        cl_push(monitor_cl, client);

    else if (pos > cl_size(monitor_cl) || pos <= 0)
        cl_append(monitor_cl, client);

    else cl_insert_after(monitor_cl, client, pos-1);

    free(detached);
}

void fl_destroy(FloatingClients* list) {
    if (list == NULL) return;

    DetachedClient* curr = list->head; 

    while (curr != NULL) {
        DetachedClient* tmp = curr;
        curr=curr->next;

        Client* client = get_client_from_detached(tmp);
        Window win = get_client_win(client);
        ungrab_left_click(get_client_win(client));
        disable_drag(win);
        disable_mouse_resize(win);

        free(tmp);
    }

    list->head = NULL;
    list->n = 0;
}

DetachedClient* fl_head(FloatingClients* list) {
    if (list == NULL) return NULL;
    return list->head;
}

size_t fl_size(FloatingClients* list) {
    if (list == NULL) return 0;
    return list->n;
}

void fl_cancel_pending_unmaps(FloatingClients* list) {
    if (list == NULL) return;

    DetachedClient* curr = fl_head(list);
    while (curr != NULL)  {
        Client* client = get_client_from_detached(curr);
        if (client_state_pending_unmap(client) 
            && client_get_unmapped_from_workspace_switch(client)
        ) {
            client_set_map_after_unmap_notify(client, true);
        }

        curr = curr->next;
    }
}

void fl_cancel_cancel_pending_unmaps(FloatingClients* list) {
    if (list == NULL) return;

    DetachedClient* curr = fl_head(list);
    while (curr != NULL)  {
        Client* client = get_client_from_detached(curr);
        if (client_get_map_after_unmap_notify(client)) {
            client_set_map_after_unmap_notify(client, false);
        }

        curr = curr->next;
    }
}

void fl_set_mapped_or_unmapped_from_workspace_switch(
    FloatingClients* list,
    SetMappedFromSwitchFn fn,
    _Bool flag
) {
    if (list == NULL || fn == NULL) return; 

    DetachedClient* curr = fl_head(list);
    while (curr != NULL) {
        fn(
            get_client_from_detached(curr),
            flag
        );
        curr = curr->next;
    }
}

void fl_map_unmap(FloatingClients* list, MapClientFn map_fn, _Bool manage_cont) {
    if (list == NULL || map_fn == NULL) return;

    DetachedClient* detached_curr = fl_head(list);
    while (detached_curr != NULL) {
        map_fn(get_client_from_detached(detached_curr), manage_cont);
        detached_curr = detached_curr->next;
    }
}
