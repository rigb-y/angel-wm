/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "focus_stack.h"
#include "client.h"
#include "workspaces.h"
#include "client_list.h"
#include "layouts.h"
#include "minimized_client.h"
#include "minimized_list.h"
#include "float_list.h"
#include "detached.h"
#include "monitors.h"
#include "monitor.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

static Client* revert_to_cl_fl_or_ml(Monitors*, Monitor*, int);

Client* fs_next_non_floating(FocusStack* list) {
    if (list == NULL) return NULL;
    Client* curr = list->head;
    while (curr != NULL && client_is_float(curr)) curr=curr->fs_next;
    return curr;
}

_Bool fs_find(FocusStack* stack, Client* client) {
    if (stack == NULL || client == NULL) return false;

    Client* curr = stack->head;
    while (curr != NULL) {
        if (curr == client) 
            return true;

        curr=curr->fs_next;
    }
    return false;
}

Client* fs_find_parent(FocusStack* stack, Client* client) {
    if (stack == NULL 
        || client == NULL 
        || !fs_find(stack, client) 
        || fs_head(stack) == client
    ) 
        return NULL;

    Client* curr = stack->head;
    Client* prev = curr;
    while (curr != NULL) {
        if (curr == client) 
            break;

        prev=curr;
        curr=curr->fs_next;
    }

    return prev;
}

void fs_remove(FocusStack* stack, Client* client) {
    if (stack == NULL || client == NULL || !fs_find(stack, client)) return;

    if (fs_head(stack) == client) {
        fs_pop(stack);
        client->fs_next = NULL;
        return;
    }

    // Guaranteed non-null
    Client* parent = fs_find_parent(stack, client);
    parent->fs_next = client->fs_next;
    client->fs_next = NULL;
    --stack->n;
}

Client* fs_pop(FocusStack* stack) {
    if (stack == NULL) return NULL;

    if (fs_empty(stack)) return NULL;

    Client* focus = fs_head(stack);
    if (fs_single(stack)) {
        stack->head = NULL; 
        --stack->n;

        focus->fs_next = NULL;
        return focus;
    }

    stack->head = stack->head->fs_next;
    --stack->n;

    focus->fs_next = NULL;
    return focus;
}

void fs_push(FocusStack* stack, Client* client) {
    if (stack == NULL || client == NULL || client_is_minimized(client)) return;

    // Remove it so that we can add it to the front
    if (fs_find(stack, client))
        fs_remove(stack, client);

    Client* head = stack->head;
    stack->head = client;
    stack->head->fs_next = head;
    ++stack->n;
}

Client* fs_head(FocusStack* stack) {
    if (stack == NULL) return NULL;

    return stack->head;
}

_Bool fs_empty(FocusStack* stack) {
    if (stack == NULL) return true;

    return stack->n == 0;
}

_Bool fs_single(FocusStack* stack) {
    if (stack == NULL) return false;

    return stack->n == 1;
}

void justify_focus(Monitor* monitor, Client* client, Time time) {
    if (monitor == NULL) return;

    if (monitor_fullscreen_exists(monitor)
        && get_monitor_fullscreen(monitor) == get_current_focus()
    ) return;

    if (client != NULL && is_client_mapped(client)) {
        set_current_focus(client, time);
        return;
    }

    set_current_focus(get_next_focus(monitor, NO_FS_REMOVE), time);
}

Client* focus_from_cl_head_or_tail(Monitors* monitors, Monitor* monitor, int workspace) {
    if (monitors == NULL || monitor == NULL) return NULL;

    Client* next = get_monitor_layout(monitor, workspace) == ANGEL_MONOCLE
        ? cl_tail(get_monitor_cl(monitor))
        : cl_head(get_monitor_cl(monitor));

    if (next != NULL)
        return next;

    return monitors_focus_from_cl_head_or_tail(monitors, workspace);
}

Client* focus_from_fl(Monitors* monitors, Monitor* monitor) {
    if (monitors == NULL || monitor == NULL) return NULL;

    Client* next = get_client_from_detached(
        fl_head(get_monitor_fl(monitor))
    );

    if (next != NULL)
        return next;

    return monitors_focus_from_fl(monitors);
}

Client* focus_from_ml(Monitors* monitors, Monitor* monitor) {
    if (monitors == NULL || monitor == NULL) return NULL;

    Client* next = get_client_from_minimized(
        ml_head(get_monitor_ml(monitor))
    );

    if (next != NULL)
        return next;

    return monitors_focus_from_fl(monitors);
}

Client* revert_to_cl_fl_or_ml(Monitors* monitors, Monitor* monitor, int workspace) {
    if (monitors == NULL || monitor == NULL) return NULL;

    Client* from_cl = focus_from_cl_head_or_tail(monitors, monitor, workspace);
    if (from_cl != NULL) return from_cl;

    Client* from_fl = focus_from_fl(monitors, monitor);
    if (from_fl) return from_fl;

    return focus_from_ml(monitors, monitor);
}

Client* get_next_focus(Monitor* monitor, _Bool remove_from_fs) {
    if (monitor == NULL) return NULL;

    int workspace = get_current_workspace();
    FocusStack* current_fs = get_current_workspace_fs();

    if (remove_from_fs == FS_REMOVE)
        fs_pop(current_fs);

    Client* next_in_line = fs_head(current_fs);
    while (next_in_line != NULL && !is_client_mapped(next_in_line))
        next_in_line = next_in_line->fs_next;

    if (next_in_line != NULL && is_client_mapped(next_in_line))
        return next_in_line;

    next_in_line = revert_to_cl_fl_or_ml(
        get_workspace_monitors(workspace),
        monitor,
        workspace
    );

    while (next_in_line != NULL && !is_client_mapped(next_in_line))
        next_in_line = next_in_line->fs_next;

    return is_client_mapped(next_in_line) ? next_in_line : NULL;
}
