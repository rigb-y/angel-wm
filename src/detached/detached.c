/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "detached.h"
#include "workspaces.h"
#include "client_list.h"
#include "client.h"
#include "windows.h"
#include "minimized_client.h"
#include "minimized_list.h"
#include "monitors.h"
#include "monitor.h"

#include <stdlib.h>
#include <stdbool.h>

static DetachedClient* detach_client_from_ml(Monitor*, Client*, int);

DetachedClient* create_detached_client(Client* client, int cl_pos, ClientListOrigin origin) {
    if (client == NULL) return NULL;

    DetachedClient* detached;
    if ((detached = calloc(1, sizeof(DetachedClient))) == NULL)
        return NULL;
    
    detached->client = client;
    detached->next = NULL;
    detached->position_in_client_list = cl_pos;
    detached->in_move_mode = false;
    detached->in_mouse_resize_mode = false;
    detached->pointer_x = 0;
    detached->pointer_y = 0;
    detached->origin = origin;
    detached->configured = false;

    detached->x = 0;
    detached->y = 0;
    detached->width = 0;
    detached->height = 0;

    return detached;
}

DetachedClient* detach_client_from_ml(Monitor* monitor, Client* client, int ws) {
    if (monitor == NULL || client == NULL)
        return NULL;

    MinimizedClient* minimized = ml_find_from_client(
        get_monitor_ml(monitor), client
    );

    ml_remove(get_monitor_ml(monitor), minimized);

    DetachedClient* detached = create_detached_client(
        get_client_from_minimized(minimized),
        get_client_pos_from_minimized(minimized), 
        ORIGIN_ML
    );

    free(minimized);
    return detached;
}

DetachedClient* detach_client(Client* client) {
    if (client == NULL) return NULL;

    int workspace = reconcile_clients_workspace(client);
    Monitor* monitor = get_monitor_from_list_membership(
        get_workspace_monitors(workspace),
        client
    );

    if (monitor == NULL)
        return NULL;

    Window win = get_client_win(client);
    ungrab_left_click(win);
    enable_drag(win);
    enable_mouse_resize(win);

    if (client_is_minimized(client))
        detach_client_from_ml(monitor, client, workspace);

    int pos = cl_get_client_position(get_monitor_cl(monitor), client);
    DetachedClient* detached = create_detached_client(client, pos, ORIGIN_CL);

    if (detached == NULL)
        return NULL;

    cl_remove(get_monitor_cl(monitor), &client, NO_FREE);

    return detached;
}

void set_detached_configured(DetachedClient* detached, _Bool configured) {
    if (detached == NULL) return;
    detached->configured = configured;
}

_Bool get_detached_configured(DetachedClient* detached) {
    if (detached == NULL) return false;
    return detached->configured;
}

Client* get_client_from_detached(DetachedClient* detached) {
    if (detached == NULL) return NULL;
    return detached->client;
}

int get_client_pos_from_detached(DetachedClient* detached) {
    if (detached == NULL) return 1;
    return detached->position_in_client_list;
}

_Bool get_move_mode(DetachedClient* detached) {
    if (detached == NULL) return false;
    return detached->in_move_mode;
}

_Bool get_mouse_resize_mode(DetachedClient* detached) {
    if (detached == NULL) return false;
    return detached->in_mouse_resize_mode;
}

void set_move_mode(DetachedClient* detached, _Bool mode) {
    if (detached == NULL) return;
    detached->in_move_mode = mode;
}

void set_mouse_resize_mode(DetachedClient* detached, _Bool mode) {
    if (detached == NULL) return;
    detached->in_mouse_resize_mode = mode;
}

void set_detached_pointer_xy(DetachedClient* detached, int x, int y) {
    if (detached == NULL) return;
    detached->pointer_x = x;
    detached->pointer_y = y;
}

int get_detached_pointer_x(DetachedClient* detached) {
    if (detached == NULL) return 0;
    return detached->pointer_x;
}

int get_detached_pointer_y(DetachedClient* detached) {
    if (detached == NULL) return 0;
    return detached->pointer_y;
}

void set_detached_list_origin(DetachedClient* detached, ClientListOrigin origin) {
    if (detached == NULL) return;
    detached->origin = origin;
}

ClientListOrigin get_detached_list_origin(DetachedClient* detached) {
    if (detached == NULL) return ORIGIN_UNKNOWN;
    return detached->origin;
}

int get_detached_x(DetachedClient* detached) {
    if (detached == NULL) return 0;
    return detached->x;
}

int get_detached_y(DetachedClient* detached) {
    if (detached == NULL) return 0;
    return detached->y;
}

int get_detached_width(DetachedClient* detached) {
    if (detached == NULL) return 0;
    return detached->width;
}

int get_detached_height(DetachedClient* detached) {
    if (detached == NULL) return 0;
    return detached->height;
}

void set_detached_x(DetachedClient* detached, int x) {
    if (detached == NULL) return;
    detached->x = x;
}

void set_detached_y(DetachedClient* detached, int y) {
    if (detached == NULL) return;
    detached->y = y;
}

void set_detached_width(DetachedClient* detached, int w) {
    if (detached == NULL) return;
    detached->width = w;
}

void set_detached_height(DetachedClient* detached, int h) {
    if (detached == NULL) return;
    detached->height = h;
}
