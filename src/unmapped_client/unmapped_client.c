/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "unmapped_client.h"
#include "workspaces.h"
#include "client.h"
#include "client_list.h"
#include "float_list.h"
#include "detached.h"
#include "minimized_list.h"
#include "minimized_client.h"
#include "types.h"
#include "monitor.h"
#include "monitors.h"
#include "docks.h"

#include <stdlib.h>
#include <limits.h>

static UnmappedClient* detach_unmapped_from_fl(Monitor*, Client*, int);
static UnmappedClient* detach_unmapped_from_ml(Monitor*, Client*, int);
static UnmappedClient* detach_unmapped_from_dl(Monitor*, Client*, int);

UnmappedClient* create_unmapped_client(Client* client, int pos, ClientListOrigin origin, ClientListOrigin origin_origin) {
    if (client == NULL) return NULL;

    UnmappedClient* unmapped;
    if ((unmapped = calloc(1, sizeof(UnmappedClient))) == NULL)
        return NULL;

    client->mapped = false;
    
    unmapped->client = client;
    unmapped->next = NULL;
    unmapped->position_in_client_list = pos;
    unmapped->origin = origin;
    unmapped->origin_origin = origin_origin;
    unmapped->stay_unmapped = false;

    return unmapped;
}

UnmappedClient* detach_unmapped_from_fl(Monitor* monitor, Client* client, int ws) {
    if (monitor == NULL 
        || client == NULL 
        || !workspace_is_valid(ws)
    ) return NULL;

    DetachedClient* detached = fl_find_from_client(
        get_monitor_fl(monitor), client
    ); 

    if (detached == NULL)
        return NULL;

    UnmappedClient* unmapped = create_unmapped_client(
        get_client_from_detached(detached),
        get_client_pos_from_detached(detached),
        ORIGIN_FL, 
        ORIGIN_CL
    );

    if (unmapped == NULL)
        return NULL;

    fl_remove(get_monitor_fl(monitor), detached);
    free(detached);

    return unmapped;
}

UnmappedClient* detach_unmapped_from_ml(Monitor* monitor, Client* client, int ws) {
    if (monitor == NULL 
        || client == NULL 
        || !workspace_is_valid(ws)
    ) return NULL;

    MinimizedClient* minimized = ml_find_from_client(
        get_monitor_ml(monitor), client
    ); 

    if (minimized == NULL)
        return NULL;

    UnmappedClient* unmapped = create_unmapped_client(
        get_client_from_minimized(minimized),
        get_client_pos_from_minimized(minimized),
        ORIGIN_ML,
        minimized->origin
    );

    if (unmapped == NULL)
        return NULL;

    ml_remove(get_monitor_ml(monitor), minimized);
    free(minimized);

    return unmapped;
}

UnmappedClient* detach_unmapped_from_dl(Monitor* monitor, Client* client, int ws) {
    if (monitor == NULL 
        || client == NULL 
        || !workspace_is_valid(ws)
    ) return NULL;

    Dock* dock = docks_find_from_client(
        get_monitor_dl(monitor),
        client
    ); 

    if (dock == NULL)
        return NULL;

    UnmappedClient* unmapped = create_unmapped_client(
        get_client_from_dock(dock),
        INT_MAX,
        ORIGIN_UNKNOWN,
        ORIGIN_UNKNOWN
    );

    if (unmapped == NULL)
        return NULL;

    docks_remove(get_monitor_dl(monitor), dock);
    free(dock);

    return unmapped;
}

UnmappedClient* detach_unmapped(Client* client) {
    if (client == NULL) return NULL;

    client->mapped = false;

    int ws = reconcile_clients_workspace(client);
    Monitor* monitor = get_monitor_from_list_membership(
        get_workspace_monitors(ws),
        client
    );

    if (monitor == NULL) 
        return NULL;

    if (client_is_float(client))
        return detach_unmapped_from_fl(
            monitor,
            client,
            ws
        );

    else if (client_is_minimized(client))
        return detach_unmapped_from_ml(
            monitor,
            client,
            ws
        );

    else if (client_is_dock(client))
        return detach_unmapped_from_dl(
            monitor,
            client, 
            ws
        );

    int pos = cl_get_client_position(get_monitor_cl(monitor), client);
    UnmappedClient* unmapped = create_unmapped_client(
        client,
        pos,
        ORIGIN_CL,
        ORIGIN_CL
    );

    if (unmapped == NULL)
        return NULL;

    cl_remove(get_monitor_cl(monitor), &client, NO_FREE);

    return unmapped;
}

Client* get_client_from_unmapped(UnmappedClient* unmapped) {
    if (unmapped == NULL) return NULL;
    return unmapped->client;
}

int get_client_pos_from_unmapped(UnmappedClient* unmapped) {
    if (unmapped == NULL) return -1;
    return unmapped->position_in_client_list;
}

void set_unmapped_list_origin(UnmappedClient* unmapped, ClientListOrigin origin) {
    if (unmapped == NULL) return;
    unmapped->origin = origin;
}

ClientListOrigin get_unmapped_list_origin(UnmappedClient* unmapped) {
    if (unmapped == NULL) return ORIGIN_UNKNOWN;
    return unmapped->origin;
}

void set_unmapped_list_origin_origin(UnmappedClient* unmapped, ClientListOrigin origin_origin) {
    if (unmapped == NULL) return;
    unmapped->origin_origin = origin_origin;
}

ClientListOrigin get_unmapped_list_origin_origin(UnmappedClient* unmapped) {
    if (unmapped == NULL) return ORIGIN_UNKNOWN;
    return unmapped->origin_origin;
}

void unmapped_set_stay_unmapped(UnmappedClient* unmapped, _Bool stay_unmapped) {
    if (unmapped == NULL) return;
    unmapped->stay_unmapped = stay_unmapped;
}

_Bool unmapped_get_stay_unmapped(UnmappedClient* unmapped) {
    if (unmapped == NULL) return false;
    return unmapped->stay_unmapped;
}
