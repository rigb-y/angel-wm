/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "minimized_client.h"
#include "client_list.h"
#include "client.h"
#include "workspaces.h"
#include "detached.h"
#include "float_list.h"
#include "types.h"
#include "monitor.h"
#include "monitors.h"
#include "icccm.h"

#include <X11/Xutil.h>
#include <stddef.h>
#include <stdlib.h>

static MinimizedClient* minimize_client_from_fl(Monitor*, Client*, int);

MinimizedClient* create_minimized_client(Client* client, int pos, ClientListOrigin origin) {
    if (client == NULL) return NULL;

    MinimizedClient* minimized;
    if ((minimized = calloc(1, sizeof(MinimizedClient))) == NULL)
        return NULL;

    minimized->client = client;
    minimized->position_in_client_list = pos;
    minimized->next = NULL;
    minimized->prev = NULL;
    minimized->origin = origin;

    return minimized;
}

MinimizedClient* minimize_client_from_fl(Monitor* monitor, Client* client, int ws) {
    if (monitor == NULL || client == NULL) return NULL;

    DetachedClient* detached = fl_find_from_client(
        get_monitor_fl(monitor), client
    ); 

    fl_remove(get_monitor_fl(monitor), detached);

    MinimizedClient* minimized = create_minimized_client(
        get_client_from_detached(detached),
        get_client_pos_from_detached(detached),
        ORIGIN_FL
    );

    free(detached);
    return minimized;
}

MinimizedClient* minimize_client(Client* client) {
    if (client == NULL) return NULL;

    int workspace = reconcile_clients_workspace(client);
    Monitor* monitor = get_monitor_from_list_membership(
        get_workspace_monitors(workspace),
        client
    );

    if (monitor == NULL)
        return NULL;

    if (client_is_float(client))
        minimize_client_from_fl(
            monitor,
            client,
            workspace
        );

    int pos = cl_get_client_position(
        get_monitor_cl(monitor),
        client
    );

    MinimizedClient* minimized = create_minimized_client(
        client,
        pos,
        ORIGIN_CL
    );

    if (minimized == NULL)
        return NULL;

    cl_remove(get_monitor_cl(monitor), &client, NO_FREE);

    set_wm_state(get_client_win(client), IconicState);
    return minimized;
}

Client* get_client_from_minimized(MinimizedClient* minimized) {
    if (minimized == NULL) return NULL;
    return minimized->client;
}

int get_client_pos_from_minimized(MinimizedClient* minimized) {
    if (minimized == NULL) return 0;
    return minimized->position_in_client_list;
}

void set_minimized_list_origin(MinimizedClient* minimized, ClientListOrigin origin) {
    if (minimized == NULL) return;
    minimized->origin = origin;
}

ClientListOrigin get_minimized_list_origin(MinimizedClient* minimized) {
    if (minimized == NULL) return ORIGIN_UNKNOWN;
    return minimized->origin;
}
