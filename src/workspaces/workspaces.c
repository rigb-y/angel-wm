/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "workspaces.h"
#include "client.h"
#include "windows.h"
#include "manage.h"
#include "geometry.h"
#include "focus_stack.h"
#include "layouts.h"
#include "types.h"
#include "icccm.h"
#include "ewmh.h"
#include "monitors.h"
#include "monitor.h"
#include "randr.h"
#include "detached.h"
#include "float_list.h"
#include "minimized_list.h"
#include "unmapped_list.h"
#include "unmapped_client.h"
#include "minimized_client.h"
#include "client_list.h"
#include "docks.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Workspaces workspaces = {0};
static int current_workspace = 0;

typedef struct InitialStatePair {
    const char* name;
    InitialState state;

    struct InitialStatePair* next;
} InitialStatePair;

static void set_initial_state_pair(InitialStatePair*, const char*, InitialState);
static InitialStatePair* create_initial_state_pair(const char*, InitialState);

typedef struct InitialStates {
    InitialStatePair* head;
} InitialStates;

static InitialStates initial_states = {0};

static void move_from_current_float_list(Monitor*, Monitor*, Client*, int);
static void move_from_current_minimized_list(Monitor*, Monitor*, Client*, int);
static void move_from_current_client_list(Monitor*, Monitor*, Client*, int);
static void move_from_current_dock_list(Monitor*, Monitor*, Client*, int);

static void handle_old_focus_state_changes(Client*);

void set_initial_state_pair(InitialStatePair* pair, const char* name, InitialState state) {
    if (pair == NULL || name == NULL) return;
    pair->name = name;
    pair->state = state;
    pair->next = NULL;
}

InitialStatePair* create_initial_state_pair(const char* name, InitialState state) {
    if (name == NULL) return NULL;

    InitialStatePair* pair;
    if ((pair = calloc(1, sizeof(InitialStatePair))) == NULL)
        return NULL;

    set_initial_state_pair(pair, name, state);
    return pair;
}

void create_initial_state(const char* name, InitialState state) {
    if (name == NULL) return;
    InitialStatePair* pair;
    if ((pair = create_initial_state_pair(name, state)) == NULL)
        return;

    InitialStatePair* head = initial_states.head;
    initial_states.head = pair;
    initial_states.head->next = head;
}

InitialState find_initial_state(const char* name) {
    if (name == NULL) return TILED;

    InitialStatePair* curr = initial_states.head;
    while (curr != NULL) {
        if (strcmp(curr->name, name) == 0)
            return curr->state;
        curr = curr->next;
    }

    return TILED;
}

void set_default_workspace_layout(Layout layout) {
    if (!is_layout_accepted(layout)) return;
    default_workspace_layout = layout;
}

Layout get_default_workspace_layout() {
    return default_workspace_layout;
}

int get_current_workspace() {
    return current_workspace;
}

void set_current_workspace(int space) {
    if (!workspace_is_valid(space)) space = 0;
    current_workspace = space;
}

void init_workspace(Workspace* space) {
    space->current_focus = NULL;
    space->focus_before_workspace_switch = NULL;
    space->current_focus_stack = calloc(1, sizeof(FocusStack));
    space->monitors = calloc(1, sizeof(Monitors));
    space->workspace_layout = ANGEL_LAYOUT_UNKNOWN;
    space->monitors_save = calloc(1, sizeof(Monitors));
}

void init_workspaces() {
    for (int i = 0; i < N_WORKSPACES; ++i) {
        init_workspace(&workspaces.spaces[i]);
    }
}

Workspace* get_workspace(int space) {
    if (!workspace_is_valid(space)) return NULL;

    return &workspaces.spaces[space];
}

FocusStack* get_workspace_fs(int space) {
    if (!workspace_is_valid(space)) return NULL;
    return get_workspace(space)->current_focus_stack;
}

FocusStack* get_current_workspace_fs() {
    return get_workspace_fs(get_current_workspace());
}

Monitors* get_workspace_monitors(int space) {
    if (!workspace_is_valid(space)) return NULL;

    return get_workspace(space)->monitors;
}

Layout get_workspace_layout(int space) {
    if (!workspace_is_valid(space)) return ANGEL_LAYOUT_UNKNOWN;
    return get_workspace(space)->workspace_layout;
}

void set_workspace_layout(int space, Layout layout) {
    if (!workspace_is_valid(space)) return;
    get_workspace(space)->workspace_layout = layout;
}

void teardown_workspaces() {
    for (int i = 0; i < N_WORKSPACES; ++i) {
        FocusStack* fs = get_workspace_fs(i);
        Monitors* monitors = get_workspace_monitors(i);
        Monitors* monitors_save = get_workspace(i)->monitors_save;

        free(fs);

        destroy_monitors(monitors);
        free(monitors);

        destroy_monitors(monitors_save);
        free(monitors_save);

        workspaces.spaces[i].current_focus = NULL;
        workspaces.spaces[i].focus_before_workspace_switch = NULL;
        workspaces.spaces[i].current_focus_stack = NULL;
        workspaces.spaces[i].monitors = NULL;
        workspaces.spaces[i].monitors_save = NULL;
    }

    InitialStatePair* curr = initial_states.head;
    while (curr != NULL) {
        InitialStatePair* tmp = curr;
        curr = curr->next;
        free((char*)tmp->name);
        free(tmp);
    }

    initial_states.head = NULL;
}

void wm_handled_focus(Client* client, Time time) {
    if (!is_client_mapped(client) || client_is_dock(client)) return;
    give_window_focus(get_client_win(client), time);
}

void handle_old_focus_state_changes(Client* old_focus) {
    if (old_focus == NULL || client_is_dock(old_focus)) return;

    int workspace = get_client_on_workspace(old_focus);
    if (get_focus_tracked_head(workspace) == old_focus)
        set_focus_tracked_head(NULL, workspace);

    set_client_in_resize_mode_no_border_change(old_focus, false);

    if (!is_client_mapped(old_focus)) return;

    if (client_get_was_configured(old_focus)) {
        int old_border_width = get_client_border_width(old_focus);
        int new_border_width = get_border_width(old_focus);

        int new_width = (get_client_width(old_focus) + 2 * old_border_width) 
            - 2 * new_border_width;

        int new_height = (get_client_height(old_focus) + 2 * old_border_width) 
            - 2 * new_border_width;

        if (new_height <= 0) new_height = 1;
        if (new_width <= 0) new_width = 1;

        resize_client(old_focus, new_width, new_height);
    }

    set_client_border(
        old_focus,
        get_border_width(old_focus),
        get_border_color(old_focus)
    );
}

void handle_new_focus_state_changes(Client* new_focus) {
    if (new_focus == NULL 
        || !is_client_mapped(new_focus) 
        || client_is_dock(new_focus)
    ) return;

    int workspace = get_client_on_workspace(new_focus);
    Monitor* monitor = get_monitor_from_list_membership(
        get_workspace_monitors(workspace),
        new_focus
    );

    fs_push(get_current_workspace_fs(), new_focus);
    set_focus_tracked_head(new_focus, get_client_on_workspace(new_focus));

    if (client_is_float(new_focus)) {
        raise_window(get_client_win(new_focus));

        if (monitor_fullscreen_exists(monitor))
            raise_window(
                get_client_win(
                    get_monitor_fullscreen(monitor)
                )
            );
    } 

    else if (in_monocle_stack(monitor, new_focus)) {
        raise_window(get_client_win(new_focus));
        raise_all_floaters(monitor);

        if (monitor_fullscreen_exists(monitor))
            raise_window(
                get_client_win(
                    get_monitor_fullscreen(monitor)
                )
            );
    }

    int old_border_width = get_client_border_width(new_focus);

    if (IN_RESIZE) 
        set_client_in_resize_mode_no_border_change(new_focus, true);

    if (client_get_was_configured(new_focus)) {
        int new_border_width = get_border_width(new_focus);

        int new_width = (get_client_width(new_focus) + 2 * old_border_width) 
            - 2 * new_border_width;

        int new_height = (get_client_height(new_focus) + 2 * old_border_width) 
            - 2 * new_border_width;

        resize_client(new_focus, new_width, new_height);
    }

    set_client_border(
        new_focus,
        get_border_width(new_focus),
        get_border_color(new_focus)
    );
}

void handle_focus_state_changes(Client* new_focus, Client* old_focus) {
    if ((new_focus != NULL && !is_client_mapped(new_focus))
        || client_is_dock(new_focus) 
        || client_is_dock(old_focus)
    ) return;

    handle_new_focus_state_changes(new_focus);
    handle_old_focus_state_changes(old_focus);
}

void set_current_focus(Client* client, Time time) {
    if (client_is_dock(client)) return;

    if (client == NULL) {
        give_window_focus(root, time);
        set_focus_tracked_head(NULL, get_current_workspace());
        ewmh_set_active_window(None);
        return;
    }

    Monitor* monitor = get_monitor_from_list_membership(
        get_workspace_monitors(get_current_workspace()), 
        client
    );

    if (reconcile_clients_workspace(client) != get_current_workspace()
        || !is_client_mapped(client)
    ) return;

    if (monitor_fullscreen_exists(monitor) && get_current_focus() == get_monitor_fullscreen(monitor))  {
        client_set_fullscreen(get_current_focus(), false);
        set_monitor_fullscreen(monitor, NULL);

        client_set_fullscreen(client, true);
        set_monitor_fullscreen(monitor, client);
        arrange_monitor(monitor, time, NO_JUSTIFY_FOCUS);
        raise_all_floaters(monitor);
    }

    Window win = get_client_win(client);
    _Bool supports_take_focus = window_supports_take_focus(win);
    _Bool input_hint = get_input_hint(win);

    ewmh_set_active_window(win);

    // No input
    if (!supports_take_focus && !input_hint) {
        return;
    }

    // Passive input
    else if (!supports_take_focus && input_hint) {
        wm_handled_focus(client, time);
    }

    // Locally active input
    else if (supports_take_focus && input_hint) {
        wm_handled_focus(client, time);
        send_take_focus(win, time);
    }

    // Globally active input
    else {
        send_take_focus(win, time);
    }
}

void set_focus_tracked_head(Client* client, int workspace) {
    if (!workspace_is_valid(workspace)) return;
    workspaces.spaces[workspace].current_focus = client;
}

Client* get_focus_tracked_head(int space) {
    if (!workspace_is_valid(space)) return NULL;

    return workspaces.spaces[space].current_focus;
}

Client* get_current_focus() {
    return get_focus_tracked_head(get_current_workspace());
}

void unmap_workspace(int space) {
    if (!workspace_is_valid(space)) return;

    Monitors* monitors = get_workspace_monitors(space);
    monitors_map_unmap_cl(monitors, unmap_client, MANAGE_CONT);
    monitors_map_unmap_fl(monitors, unmap_client, MANAGE_CONT);
    monitors_map_unmap_ml(monitors, unmap_client, MANAGE_CONT);
    monitors_map_unmap_dl(monitors, unmap_client, MANAGE_CONT);
}

void map_workspace(int space) {
    if (!workspace_is_valid(space)) return;

    monitors_map_unmap_ul(
        get_workspace_monitors(space),
        __map_client,
        NO_LONGER_UNMAPPED
    );
}

void unmap_current_workspace() {
    unmap_workspace(get_current_workspace());
}

void map_current_workspace() {
    map_workspace(get_current_workspace());
}

void move_from_current_float_list(Monitor* src, Monitor* dest, Client* client, int workspace) {
    if (client == NULL
        || src == NULL
        || !workspace_is_valid(workspace)
        || src == dest
    ) return;

    DetachedClient* detached = fl_find_from_client(
        get_monitor_fl(src),
        client
    );

    fl_remove(get_monitor_fl(src), detached);
    fl_push(get_monitor_fl(dest), detached);
}

void move_from_current_minimized_list(Monitor* src, Monitor* dest, Client* client, int workspace) {
    if (client == NULL
        || src == NULL
        || !workspace_is_valid(workspace)
        || src == dest
    ) return;

    MinimizedClient* minimized = ml_find_from_client(
        get_monitor_ml(src),
        client
    );

    ml_remove(get_monitor_ml(src), minimized);
    ml_push(get_monitor_ml(dest), minimized);
}

void move_from_current_client_list(Monitor* src, Monitor* dest, Client* client, int workspace) {
    if (client == NULL
        || src == NULL
        || !workspace_is_valid(workspace)
        || src == dest
    ) return;

    cl_remove(get_monitor_cl(src), &client, NO_FREE);
    cl_append(get_monitor_cl(dest), client);
}

void move_from_current_dock_list(Monitor* src, Monitor* dest, Client* client, int workspace) {
    if (client == NULL
        || src == NULL
        || !workspace_is_valid(workspace)
        || src == dest
    ) return;

    Dock* dock = docks_find_from_client(
        get_monitor_dl(src),
        client
    );

    docks_remove(get_monitor_dl(src), dock);
    docks_push(get_monitor_dl(dest), dock);
}

void move_client_to_workspace(Client* client, int workspace) {
    if (client == NULL) return;

    client_set_on_workspace(client, workspace);

    Monitor* monitor = get_monitor_from_list_membership(
        get_workspace_monitors(get_current_workspace()),
        client
    );

    Monitor* dest = get_symmetric_monitor(
        monitor, 
        workspace
    );

    if (monitor == dest) return;

    if (client_is_float(client))
        move_from_current_float_list(monitor, dest, client, workspace);

    else if (client_is_minimized(client))
        move_from_current_minimized_list(monitor, dest, client, workspace);

    else if (client_is_dock(client))
        move_from_current_dock_list(monitor, dest, client, workspace);

    else
        move_from_current_client_list(monitor, dest, client, workspace);
}

_Bool scan_other_workspaces_for_window(Window win) {
    for (int i = 0; i < N_WORKSPACES; ++i) {
        if (i == get_current_workspace()) continue;

        Monitors* monitors = get_workspace_monitors(i);

        if (monitors_search_for_client(monitors, win, NULL))
            return true;
    }

    return false;
}

_Bool workspace_is_valid(int space) {
    return space >= 0 && space < N_WORKSPACES;
}

int reconcile_clients_workspace(Client* client) {
    if (client == NULL) return get_current_workspace();

    int workspace = get_client_on_workspace(client);
    return workspace_is_valid(workspace) 
        ? workspace : get_current_workspace(); 
}

int reconcile_client_starting_workspace(Client* client) {
    if (client == NULL) return get_current_workspace();

    int ewmh_workspace = (int)ewmh_get_workspace_num(get_client_win(client));
    return workspace_is_valid(ewmh_workspace) 
        ? ewmh_workspace : get_current_workspace();
}

void set_focus_before_workspace_switch(int space, Client* client) {
    if (!workspace_is_valid(space)) return;
    workspaces.spaces[space].focus_before_workspace_switch = client;
}

Client* get_focus_before_workspace_switch(int space) {
    if (!workspace_is_valid(space)) return NULL;
    return workspaces.spaces[space].focus_before_workspace_switch;
}

int get_mapped_in_workspace(int workspace) {
    if (!workspace_is_valid(workspace)) return 0;
    return workspaces.spaces[workspace].mapped_in_workspace;
}

int get_mapped_in_current_workspace() {
    return get_mapped_in_workspace(get_current_workspace());
}


void inc_mapped_in_workspace(int workspace) {
    if (!workspace_is_valid(workspace)) return;
    ++workspaces.spaces[workspace].mapped_in_workspace;
}

void inc_mapped_in_current_workspace() {
    ++workspaces.spaces[get_current_workspace()].mapped_in_workspace;
}

void dec_mapped_in_workspace(int workspace) {
    if (!workspace_is_valid(workspace) 
        || get_mapped_in_workspace(workspace) == 0
    ) return;

    --workspaces.spaces[workspace].mapped_in_workspace;
}

void dec_mapped_in_current_workspace() {
    if (get_mapped_in_workspace(
            get_current_workspace()
        ) == 0
    ) return;

    --workspaces.spaces[get_current_workspace()].mapped_in_workspace;
}

int count_viewable_clients_in_workspace(int workspace) {
    if (!workspace_is_valid(workspace)) return 0;

    return count_viewable_clients_in_monitors(
        get_workspace_monitors(workspace)
    );
}

int count_viewable_clients_in_current_workspace() {
    return count_viewable_clients_in_workspace(get_current_workspace());
}

_Bool clients_still_mapped(int workspace) {
    return count_viewable_clients_in_workspace(workspace) > 0; 
}

_Bool client_is_focus_before_workspace_switch(Client* client) {
    for (int i = 0; i < N_WORKSPACES; ++i) {
        if (workspaces.spaces[i].focus_before_workspace_switch == client)
            return true;
    }

    return false;
}

void workspace_cancel_pending_unmaps(int workspace) {
    if (!workspace_is_valid(workspace)) return;

    monitors_cancel_pending_unmaps(
        get_workspace_monitors(workspace)
    );
}

void workspace_cancel_cancel_pending_unmaps(int workspace) {
    if (!workspace_is_valid(workspace)) return;

    monitors_cancel_cancel_pending_unmaps(
        get_workspace_monitors(workspace)
    );
}

void workspace_set_all_unmapped_from_workspace_switch(int workspace, _Bool flag) {
    if (!workspace_is_valid(workspace)) return;

    monitors_set_all_unmapped_from_workspace_switch(
        get_workspace_monitors(workspace),
        flag
    );
}

void workspace_set_all_mapped_from_workspace_switch(int workspace, _Bool flag) {
    if (!workspace_is_valid(workspace)) return;

    monitors_set_all_mapped_from_workspace_switch(
        get_workspace_monitors(workspace),
        flag
    );
}

void workspace_cancel_pending_maps(int workspace) {
    if (!workspace_is_valid(workspace)) return;

    monitors_cancel_ul_pending_maps(
        get_workspace_monitors(workspace)
    );
}

void workspace_cancel_cancel_pending_maps(int workspace) {
    if (!workspace_is_valid(workspace)) return;

    monitors_cancel_cancel_ul_pending_maps(
        get_workspace_monitors(workspace)
    );
}

void fill_monitor(int space) {
    int n = 0;
    XRRMonitorInfo* monitor_info = get_active_monitor_info(&n);
    if (monitor_info == NULL)
        return;

    for (int i = 0; i < n; ++i) {
        Monitor* monitor = create_monitor(
            monitor_info[i].name,
            monitor_info[i].primary,
            monitor_info[i].x,
            monitor_info[i].y,
            monitor_info[i].width,
            monitor_info[i].height
        );

        push_monitor(get_workspace_monitors(space), monitor);
    }

    free_active_monitor_info(monitor_info);
}

void fill_monitors() {
    for (int i = 0; i < N_WORKSPACES; ++i)
        fill_monitor(i);
}

void update_monitors() {
    int n = 0;
    XRRMonitorInfo* monitor_info = get_active_monitor_info(&n);
    if (monitor_info == NULL)
        return;

    Atom removed = None;
    while ((removed = find_removed_monitor(
            get_workspace_monitors(0),
            monitor_info,
            n
        )) != None
    ) {
        remove_monitor_from_workspaces(removed); 
    }

    Monitor* new_monitor = None;
    while ((new_monitor = find_new_monitor(
            get_workspace_monitors(0),
            monitor_info, n
        )) != NULL
    ) {
        add_monitor_to_workspaces(new_monitor);
        reconcile_saved_monitors();
        
        // Monitors now own their own copy
        free_monitor_resources(new_monitor);
        free(new_monitor);
    }

    workspaces_update_monitors_internal_data(monitor_info, n);
    dump_monitor_info();

    free_active_monitor_info(monitor_info);
}

Monitor* get_symmetric_monitor(Monitor* monitor, int to) {
    if (monitor == NULL 
        || !workspace_is_valid(to)
    ) return NULL;

    return get_monitor_with_name(
        get_workspace_monitors(to),
        get_monitor_name(monitor)
    );
}

Monitor* reconcile_monitor_from_focus(int space, Client* focus) {
    if (!workspace_is_valid(space)) return NULL;

    return focus == NULL 
        ? get_monitor_from_pointer_position(
            get_workspace_monitors(space)
        )

        : get_monitor_from_list_membership(
            get_workspace_monitors(space),
            focus
        );
}

void remove_monitor_from_workspaces(Atom name) {
    if (name == None) return;

    for (int i = 0; i < N_WORKSPACES; ++i) {
        Monitors* monitors = get_workspace_monitors(i);
        Monitor* monitor = get_monitor_with_name(monitors, name);
        if (!monitors_absorb_monitor(monitors, monitor)) {
            add_monitor_to_save(monitor, i);
        }

        remove_monitor(monitors, monitor);
        destroy_monitor(monitor);
        free(monitor);
    }
}

void add_monitor_to_workspaces(Monitor* monitor) {
    if (monitor == NULL) return;

    for (int i = 0; i < N_WORKSPACES; ++i) {
        Monitor* owned_copy = create_monitor(
            get_monitor_name(monitor),
            is_monitor_primary(monitor),
            monitor_x(monitor),
            monitor_y(monitor),
            monitor_width(monitor),
            monitor_height(monitor)
        );

        push_monitor(get_workspace_monitors(i), owned_copy);
    }
}

void workspaces_update_monitors_internal_data(XRRMonitorInfo* info, int n) {
    if (info == NULL) return;

    for (int i = 0; i < N_WORKSPACES; ++i)
        monitors_update_internal_data(
            get_workspace_monitors(i),
            info,
            n
        );
}

void add_monitor_to_save(Monitor* monitor, int workspace) {
    if (monitor == NULL || !workspace_is_valid(workspace)) return;

    Monitor* saved = create_monitor(None, false, 0, 0, 0, 0);
    if (saved == NULL)
        return;

    absorb_monitor(saved, monitor);
    push_monitor(get_workspace_monitors(workspace), saved);
}

void reconcile_saved_monitors() {
    for (int i = 0; i < N_WORKSPACES; ++i) {
        Monitors* monitors = get_workspace_monitors(i);
        Monitors* saved_monitors = get_workspace(i)->monitors_save;

        if (get_monitors_size(saved_monitors) > 0)
            absorb_all_saved_monitors(saved_monitors, monitors);
    }
}

void absorb_all_saved_monitors(Monitors* saved_monitors, Monitors* workspace_monitors) {
    if (saved_monitors == NULL || workspace_monitors == NULL) return;

    Monitor* curr = saved_monitors->head;
    while (curr != NULL) {
        Monitor* tmp = curr;
        curr = curr->next;
        monitors_absorb_monitor(workspace_monitors, tmp);
        remove_monitor(saved_monitors, tmp);
        free_monitor_resources(tmp);
        free(tmp);
    } 
}
