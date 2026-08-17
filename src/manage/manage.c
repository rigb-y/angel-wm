/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "manage.h"
#include "client.h"
#include "client_list.h"
#include "workspaces.h"
#include "windows.h"
#include "focus_stack.h"
#include "events.h"
#include "detached.h"
#include "float_list.h"
#include "minimized_client.h"
#include "minimized_list.h"
#include "icccm.h"
#include "unmapped_client.h"
#include "unmapped_list.h"
#include "types.h"
#include "ewmh.h"
#include "monitors.h"
#include "monitor.h"
#include "docks.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

static FocusStart focus_start = FOCUS_START_ADJACENT;
static FocusEnd focus_end = FOCUS_END_NEXT;

static Client* search_for_client(Window, Monitor**);

static void start_manage_unmapped_float(Monitor*, Client*);
static void start_manage_unmapped_tiled(Monitor*, Client*);
static void start_manage_mapped_float(Monitor*, Client*);
static void start_manage_mapped_tiled(Monitor*, Client*, int);

static void start_manage_dock(Monitor*, Client*);
static void start_manage_unmapped_dock(Monitor*, Client*);
static void start_manage_mapped_dock(Monitor*, Client*);

FocusStart focus_start_adjacent() {
    return focus_start == FOCUS_START_ADJACENT;
}

FocusStart focus_start_end() {
    return focus_start == FOCUS_START_END;
}

_Bool set_focus_start(FocusStart mode) {
    if (mode != FOCUS_START_ADJACENT 
        && mode != FOCUS_START_END
    ) return false;

    focus_start = mode;
    return true;
}

FocusEnd focus_end_focus_stack() {
    return focus_end == FOCUS_END_FOCUS_STACK;
}

FocusEnd focus_end_next() {
    return focus_end == FOCUS_END_NEXT;
}

_Bool set_focus_end(FocusEnd mode) {
    if (mode != FOCUS_END_NEXT 
        && mode != FOCUS_END_FOCUS_STACK
    ) return false;

    focus_end = mode;
    return true;
}

void start_manage_unmapped_float(Monitor* monitor, Client* client) {
	if (monitor == NULL || client == NULL) return;

    Window win = get_client_win(client);
    client_set_float(client, true);
    enable_drag(win);
    enable_mouse_resize(win);

    // Sets client->mapped = false
    UnmappedClient* unmapped = create_unmapped_client(
        client,
        INT_MAX,
        ORIGIN_FL,
        ORIGIN_FL
    );

    ul_push(get_monitor_ul(monitor), unmapped);
}

void start_manage_unmapped_tiled(Monitor* monitor, Client* client) {
	if (monitor == NULL || client == NULL) return;

    Client* focus = get_current_focus();
    int pos = cl_get_client_position(get_monitor_cl(monitor), focus) + 1;

    grab_left_click(get_client_win(client));

    UnmappedClient* unmapped = create_unmapped_client(
        client,
        focus != NULL && focus_start_adjacent() ? pos : INT_MAX,
        ORIGIN_CL,
        ORIGIN_CL
    );

    ul_push(get_monitor_ul(monitor), unmapped);
}

void start_manage_mapped_float(Monitor* monitor, Client* client) {
	if (monitor == NULL || client == NULL) return;

    Window win = get_client_win(client);
    enable_drag(win);
    enable_mouse_resize(win);

    DetachedClient* detached = create_detached_client(
        client,
        INT_MAX,
        ORIGIN_FL
    );

    fl_push(get_monitor_fl(monitor), detached);
}

void start_manage_mapped_tiled(Monitor* monitor, Client* client, int workspace) {
	if (monitor == NULL || client == NULL || !workspace_is_valid(workspace)) return;

    ClientList* monitor_cl = get_monitor_cl(monitor);

    grab_left_click(get_client_win(client));

    Client* focus = get_focus_tracked_head(workspace);
    Monitor* focus_monitor = get_monitor_from_list_membership(
        get_workspace_monitors(workspace),
        focus
    );

    if (focus == NULL 
        || focus_monitor != monitor
        || client_is_float(focus) 
        || client_is_minimized(focus)
    ) {
        cl_append(get_monitor_cl(monitor), client);
        return;
    }

    cl_insert_after(
        get_monitor_cl(monitor),
        client,
        cl_get_client_position(
            monitor_cl,
            focus
        )
    );
}

void start_manage_unmapped_dock(Monitor* monitor, Client* client) {
    if (monitor == NULL || client == NULL) return;

    UnmappedClient* unmapped = create_unmapped_client(
        client,
        INT_MAX,
        ORIGIN_CL,
        ORIGIN_CL
    );

    ul_push(get_monitor_ul(monitor), unmapped);
}

void start_manage_mapped_dock(Monitor* monitor, Client* client) {
    if (monitor == NULL || client == NULL) return;

    Dock* dock = create_dock(
        client,
        ewmh_get_dock_strut(get_client_win(client))
    );

    docks_push(get_monitor_dl(monitor), dock);
}

void start_manage_dock(Monitor* monitor, Client* client) {
    client_set_dock(client, true);

    Monitor* correct_monitor = move_client_to_monitor_with_position(
        get_workspace_monitors(get_client_on_workspace(client)),
        monitor, 
        client
    );

    if (is_client_mapped(client)) {
        start_manage_mapped_dock(correct_monitor, client);
        return;
    }

    start_manage_unmapped_dock(correct_monitor, client);
}

void start_manage(Window win, XWindowAttributes* attrs, Monitor* start_monitor) {
    if (attrs == NULL) return;

    Client* client;
    if ((client = calloc(1, sizeof(Client))) == NULL)
        return;

    init_client(client, win);
    set_client_position(client, attrs->x, attrs->y);
    set_client_geometry(client, attrs->width, attrs->height, attrs->border_width);
    start_event_watch(get_client_win(client));

    const char* win_name = get_window_class_name(win);
    InitialState initial_state = find_initial_state(win_name);

    Window parent = None;
    if (win_is_transient(win, &parent))
        initial_state = FLOAT;

    _Bool is_mapped = attrs->map_state != IsUnmapped; 

    XFree(attrs);

    int workspace = reconcile_client_starting_workspace(client);
    client_set_on_workspace(client, workspace);

    Monitors* monitors = get_workspace_monitors(workspace);
    Monitor* monitor = reconcile_client_starting_monitor(monitors, client);

    if (start_monitor != NULL && get_monitor_name_property(win) == None) { 
        monitor = start_monitor;
    }

    store_monitor_name_property(win, get_monitor_name(monitor));

    if (is_mapped && workspace != get_current_workspace()) {
        unmap_window(get_client_win(client));
        inc_pending_unmap(client);
        client->mapped = false;
        is_mapped = false;
    }

    if (is_mapped) {
        client->mapped = true;
        client_set_transition_state(client, CLIENT_MAPPED);
        inc_mapped_in_workspace(get_client_on_workspace(client));
    }

    else {
        client->mapped = false;
        client_set_transition_state(client, CLIENT_UNMAPPED);
    }

    if (ewmh_get_workspace_num(win) == -2)
        ewmh_store_workspace_num(win, workspace);

    if (ewmh_window_is_dock(win))
        start_manage_dock(monitor, client); 

    else if (initial_state == FLOAT && is_mapped)
        start_manage_mapped_float(monitor, client);

    else if (initial_state == FLOAT && !is_mapped)
        start_manage_unmapped_float(monitor, client);

    else if (initial_state == TILED && is_mapped)
        start_manage_mapped_tiled(monitor, client, workspace);

    else if (initial_state == TILED && !is_mapped)
        start_manage_unmapped_tiled(monitor, client);

    set_wm_state(win, NormalState);
    free((char*)win_name);
}

_Bool is_managed(Window win) {
    return search_for_client(win, NULL) != NULL;
}

Client* get_managed(Window win) {
    return search_for_client(win, NULL);
}

Client* search_for_client(Window win, Monitor** monitor_return) {
    Monitor* ret = NULL;
    for (int i = 0; i < N_WORKSPACES; ++i) {
        Client* search = monitors_search_for_client(
            get_workspace_monitors(i),
            win,
            monitor_return
        );

        if (search != NULL) return search;
    }

    return NULL;
} 

_Bool end_manage(Window win) {
    Monitor* monitor = NULL;
    Client* client = search_for_client(win, &monitor);
    if (client == NULL || monitor == NULL)
        return false;

    int workspace = reconcile_clients_workspace(client);

    // NULL unless the client is detached
    DetachedClient* detached = fl_find_from_client(
        get_monitor_fl(monitor),
        client
    );

    // NULL unless the client is minimized
    MinimizedClient* minimized = ml_find_from_client(
        get_monitor_ml(monitor),
        client
    );

    // NULL unless the client is unmapped
    UnmappedClient* unmapped = ul_find_from_client(
        get_monitor_ul(monitor),
        client
    );

    // NULL unless the client is a dock
    Dock* dock = docks_find_from_client(
        get_monitor_dl(monitor),
        client
    );

    if (dock != NULL) {
        docks_remove(get_monitor_dl(monitor), dock);
        free(dock);
        return true;
    }

    Client* next = NULL;
    Client* next_minimized_focus = NULL;
    Client* focus = get_current_focus();
    int focus_workspace = reconcile_clients_workspace(focus);
    if (focus == client
        && client_is_minimized(client) 
        && focus_workspace == workspace
        && ml_size(get_monitor_ml(monitor)) > 1
    ) {
        next_minimized_focus = get_client_from_minimized(
            ml_find_from_client(
                get_monitor_ml(monitor), 
                focus
            )->next
        );
    }

    else if (focus == client) {
        next = client->next;
    }

    if (client_is_must_leave(client))
        clear_must_leave();

    if (client_is_current_focus(client))
        set_focus_tracked_head(NULL, get_current_workspace());

    if (client_is_fullscreen(client)) {
        set_monitor_fullscreen(
            monitor, NULL
        );
    }

    if (client_is_focus_before_workspace_switch(client))
        set_focus_before_workspace_switch(
            workspace, NULL
        );

    // Frees the client and removes it from the focus stack
    fs_remove(get_workspace_fs(workspace), client);
    
    cl_remove(get_monitor_cl(monitor), &client, FREE);

    // These No-op on NULL so it's safe to do this if cl_remove destroyed the client
    fl_remove(get_monitor_fl(monitor), detached);
    ml_remove(get_monitor_ml(monitor), minimized);

    ul_remove(get_monitor_ul(monitor), unmapped);

    free(detached);
    free(minimized);
    free(unmapped);

    if (next_minimized_focus != NULL) {
        set_current_focus(next_minimized_focus, CurrentTime);
        return true;
    }

    if (next == NULL || focus_end_focus_stack()) 
        set_current_focus(
            get_next_focus(
                monitor,
                NO_FS_REMOVE
            ),
            CurrentTime
        );

    else 
        set_current_focus(
            next,
            CurrentTime
        );

    return true;
}
