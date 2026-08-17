/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_WORKSPACES_H
#define ANGEL_WORKSPACES_H

#include "types.h"
#include "layouts.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#define N_WORKSPACES 10

typedef struct ClientList ClientList;
typedef struct Workspaces Workspaces;
typedef struct Client Client;
typedef struct FloatingClients FloatingClients;
typedef struct MinimizedList MinimizedList;
typedef struct FocusStack FocusStack;
typedef struct UnmappedClients UnmappedClients;
typedef struct Monitors Monitors;
typedef struct DetachedClient DetachedClient;
typedef struct UnmappedClient UnmappedClient;
typedef struct MinimizedClient MinimizedClient;

extern Workspaces workspaces;

typedef enum InitialState {
    TILED, FLOAT, NULL_STATE
} InitialState;

void create_initial_state(const char*, InitialState);
InitialState find_initial_state(const char*);

void set_default_workspace_layout(Layout);
Layout get_default_workspace_layout();

int get_current_workspace();
void set_current_workspace(int);

typedef struct Workspace {
    Monitors* monitors;

    Client* current_focus;
    Client* focus_before_workspace_switch;
    FocusStack* current_focus_stack;

    Monitors* monitors_save;

    int mapped_in_workspace;
    Layout workspace_layout;
} Workspace;

typedef struct Workspaces {
    Workspace spaces[N_WORKSPACES];
} Workspaces;

void init_workspace(Workspace*);
void init_workspaces();

Workspace* get_workspace(int);
FocusStack* get_workspace_fs(int);
FocusStack* get_current_workspace_fs();

Layout get_workspace_layout(int);
void set_workspace_layout(int, Layout);

Monitors* get_workspace_monitors(int);

void handle_focus_state_changes(Client*, Client*);
void wm_handled_focus(Client*, Time);
void set_current_focus(Client*, Time);
void set_focus_before_workspace_switch(int, Client*);
Client* get_focus_before_workspace_switch(int);
Client* get_focus_tracked_head(int);
Client* get_current_focus();

void set_focus_tracked_head(Client*, int);

void unmap_workspace(int);
void map_workspace(int);
void unmap_current_workspace();
void map_current_workspace();

void move_client_to_workspace(Client*, int);

void teardown_workspaces();

_Bool scan_other_workspaces_for_window(Window);

_Bool workspace_is_valid(int);
int reconcile_clients_workspace(Client*);
int reconcile_client_starting_workspace(Client*);

int get_mapped_in_workspace(int);
int get_mapped_in_current_workspace();

void inc_mapped_in_workspace(int);
void inc_mapped_in_current_workspace();

void dec_mapped_in_workspace(int);
void dec_mapped_in_current_workspace();

int count_viewable_clients_in_workspace(int);
int count_viewable_clients_in_current_workspace();

_Bool clients_still_mapped(int);

_Bool client_is_focus_before_workspace_switch(Client*);

void workspace_cancel_pending_unmaps(int);
void workspace_cancel_cancel_pending_unmaps(int);

void workspace_set_all_unmapped_from_workspace_switch(int, _Bool);
void workspace_set_all_mapped_from_workspace_switch(int, _Bool);

void workspace_cancel_pending_maps(int);
void workspace_cancel_cancel_pending_maps(int);

void fill_monitor(int);
void fill_monitors();
void update_monitors();

Monitor* get_symmetric_monitor(Monitor*, int);
Monitor* reconcile_monitor_from_focus(int, Client*);
void remove_monitor_from_workspaces(Atom);
void add_monitor_to_workspaces(Monitor*);
void workspaces_update_monitors_internal_data(XRRMonitorInfo*, int);
void add_monitor_to_save(Monitor*, int);
void reconcile_saved_monitors();
void absorb_all_saved_monitors(Monitors*, Monitors*);

#endif
