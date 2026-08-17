/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "client_list.h"
#include "client.h"
#include "utils.h"
#include "windows.h"

#include <stdlib.h>
#include <stdbool.h>

_Bool invalid_client_number(ClientList* list, int n) {
    if (list == NULL) return NULL;

    return n <= 0 || n > list->n;
}

Client* cl_head(ClientList* list) {
    if (list == NULL) return NULL;

    return list->head;
}

Client* cl_tail(ClientList* list) {
    if (list == NULL) return NULL;

    return list->tail;
}

Client* cl_client_number(ClientList* list, int n) {
    if (list == NULL || invalid_client_number(list, n)) return NULL;

    int pos = 1;
    Client* curr = list->head;
    while (curr != NULL && pos != n) {
        curr=curr->next;
        ++pos; 
    }

    return curr;
}

Client* cl_find_client_from_win(ClientList* list, Window win) {
    if (list == NULL) return NULL;

    Client* curr = list->head; 
    while (curr != NULL) {
        if (curr->win == win) 
            return curr;

        curr=curr->next;
    }

    return NULL;
}

Client* cl_find_client(ClientList* list, const Client* client) {
    if (list == NULL || client == NULL) return NULL;

    Client* curr = list->head;
    while (curr != NULL) {
        if (curr == client) return curr;
        curr=curr->next;
    }
    return NULL;
}

size_t cl_size(ClientList* list) {
    if (list == NULL) return 0;
    return list->n;
}

_Bool cl_empty(ClientList* list) {
    if (list == NULL) return true;
    return cl_size(list) == 0;
}

_Bool cl_single(ClientList* list) {
    if (list == NULL) return false;
    return cl_size(list) == 1;
}

void cl_append(ClientList* list, Client* client) {
    if (list == NULL 
        || client == NULL 
        || cl_find_client(list, client) != NULL
    ) return;

    client->next = NULL;

    if (cl_empty(list)) {
        list->head = client;
        list->tail = client;
        ++list->n;
        return;
    }

    list->tail->next = client;
    list->tail = client;

    ++list->n;
}

void cl_push(ClientList* list, Client* client) {
    if (list == NULL 
        || client == NULL 
        || cl_find_client(list, client) != NULL
    ) return;

    if (cl_empty(list)) {
        list->head = client;
        list->tail = client;
        client->next = NULL;
        ++list->n;
        return;
    }

    Client* head = list->head;
    list->head = client;
    list->head->next = head;
    ++list->n;
}

void cl_insert_after(ClientList* list, Client* client, int pos) {
    if (list == NULL 
        || client == NULL 
        || cl_find_client(list, client) != NULL 
        || invalid_client_number(list, pos)
    ) return;

    if (pos == cl_size(list)) {
        cl_append(list, client);
        return;
    }

    Client* client_parent = cl_client_number(list, pos);
    Client* client_next = client_parent->next;

    client_parent->next = client;
    client->next = client_next;

    ++list->n;
}

Client* cl_find_parent(ClientList* list, Client* client) {
    if (list == NULL 
        || client == NULL 
        || cl_find_client(list, client) == NULL
        || client == list->head 
    ) return NULL;

    Client* curr = list->head;
    Client* prev = curr;
    while (curr != NULL) {
        if (curr == client) return prev;
        prev = curr;
        curr = curr->next;
    }

    return NULL;
}

void cl_remove(ClientList* list, Client** client, _Bool free_client) {
    if (list == NULL ||
        client == NULL ||
        *client == NULL ||
        cl_find_client(list, *client) == NULL
    ) return;

    // Head is tail and head is client
    if (cl_single(list)) {
        list->head = NULL;
        list->tail = NULL;

        if (free_client) {
            free(*client);
            *client = NULL;
        }

        else {
            (*client)->next = NULL;
        }

        list->n = 0;
        return;
    }

    // Head is client and head is not tail
    if (*client == list->head) {
        list->head = list->head->next;

        if (free_client) {
            free(*client);
            *client = NULL;
        }

        else {
            (*client)->next = NULL;
        }
        
        --list->n;
        return;
    }
    
    // If client_parent == client, client is head
    Client* client_parent = cl_find_parent(list, *client);
    if (!client_parent) return;

    client_parent->next = (*client)->next;

    if (list->tail == *client) {
        list->tail = client_parent;
    }

    if (free_client) {
        free(*client);
        *client = NULL;
    }

    else {
        (*client)->next = NULL;
    }

    --list->n;
}

void cl_destroy(ClientList* list) {
    if (list == NULL) return;

    Client* curr = list->head; 

    while (curr != NULL) {
        Client* tmp = curr;
        curr=curr->next;

        Window win = get_client_win(tmp);
        ungrab_left_click(get_client_win(tmp));
        disable_drag(win);
        disable_mouse_resize(win);

        free(tmp);
    }

    list->head = NULL;
    list->tail = NULL;
    list->n = 0;
}

void cl_dump(ClientList* list) {
    if (list == NULL) return;

    Client* curr = list->head;
    while (curr != NULL) {
        dump_window(curr->win);
        curr=curr->next;
    }
}

void cl_swap_clients(ClientList* list, Client* a, Client* b) {
    if (a == NULL ||
        b == NULL ||
        cl_find_client(list, a) == NULL ||
        cl_find_client(list, b) == NULL ||
        a == b
    ) return;

    if (cl_get_client_position(list, a) > cl_get_client_position(list, b)) {
        cl_swap_clients(list, b,a);
        return;
    }

    Client* a_next = a->next;
    Client* a_parent = cl_find_parent(list, a);
    Client* b_parent = cl_find_parent(list, b);

    a->next = b->next;

    if (a_next != b) {
        b->next = a_next;
    } else b->next = a;

    if (a_parent != NULL) a_parent->next = b;
    if (b_parent != NULL && b_parent != a) b_parent->next = a;

    if (a == list->head) list->head = b; 
    if (b == list->tail) list->tail = a;

    swap_clients_geometry(a,b);
}

int cl_get_client_position(ClientList* list, const Client* client) {
    if (client == NULL || cl_find_client(list, client) == NULL) return -1;

    int pos = 1;
    Client* curr = list->head;
    while (curr != NULL && curr != client) {
        ++pos;
        curr=curr->next;
    }

    return pos;
}

void cl_inherit_sibling_max_step(Client* start, ResizeStepInfoFn step_info_fn, ResizeSetStepFn set_step_fn) {
    if (start == NULL) return;

    Client* curr = start;
    int max_step = 0;
    while (curr != NULL) {
        max_step = max(step_info_fn(curr), max_step);
        curr = curr->next;
    }
    curr = start;

    while (curr != NULL) {
        set_step_fn(curr, max_step);            
        curr=curr->next;
    }
}

void cl_set_clients_resize_step(Client* start, int step, ResizeSetStepFn set_step_fn) {
    if (start == NULL) return;

    while (start != NULL) {
        set_step_fn(start, step);
        start=start->next;
    }
}

int cl_client_distance(ClientList* list, Client* a, Client* b) {
    if (list == NULL 
        || a == NULL 
        || b == NULL 
        || cl_find_client(list, a) == NULL 
        || cl_find_client(list, b) == NULL
    ) return 0;

    return cl_get_client_position(list, b) 
        - cl_get_client_position(list, a);
}

void cl_set_future_unmap_stay_unmapped(ClientList* list) {
    if (list == NULL) return;

    Client* curr = list->head;
    while (curr != NULL) {
        client_set_future_unmap_stay_unmapped(curr);
        curr = curr->next;
    }
}

void cl_cancel_pending_unmaps(ClientList* list) {
    if (list == NULL) return;

    Client* curr = cl_head(list);
    while (curr != NULL)  {
        if (client_state_pending_unmap(curr) 
            && client_get_unmapped_from_workspace_switch(curr)
        ) {
            client_set_map_after_unmap_notify(curr, true);
        }

        curr = curr->next;
    }
}

void cl_cancel_cancel_pending_unmaps(ClientList* list) {
    Client* curr = cl_head(list);
    while (curr != NULL)  {
        if (client_get_map_after_unmap_notify(curr)) {
            client_set_map_after_unmap_notify(curr, false);
        }

        curr = curr->next;
    }
}

void cl_set_mapped_or_unmapped_from_workspace_switch(
    ClientList* list,
    SetMappedFromSwitchFn fn,
    _Bool flag
) {
    if (list == NULL || fn == NULL) return;

    Client* curr = cl_head(list);
    while (curr != NULL) {
        fn(curr, flag);
        curr = curr->next;
    }
}

void cl_map_unmap(ClientList* list, MapClientFn map_fn, _Bool manage_cont) {
    if (list == NULL || map_fn == NULL) return;

    Client* curr = cl_head(list);
    while (curr != NULL) {
        map_fn(curr, manage_cont); 
        curr = curr->next;
    }
}
