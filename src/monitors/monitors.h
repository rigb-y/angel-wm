/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_MONITORS_H
#define ANGEL_MONITORS_H

#include "types.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

typedef struct Monitor Monitor;
typedef struct Monitors Monitors;
typedef struct Client Client;

typedef struct Monitors {
    Monitor* head;
    int n;
} Monitors;

Monitor* monitors_head(const Monitors*);

Monitor* get_monitor_with_name(const Monitors*, Atom);
int get_monitors_size(const Monitors*);

Monitor* get_monitor_parent(const Monitors*, const Monitor*);
_Bool find_monitor(const Monitors*, const Monitor*);
void push_monitor(Monitors*, Monitor*);
void remove_monitor(Monitors*, Monitor*);
int number_of_monitors(const Monitors*);
Monitor* get_monitor_k(const Monitors*, int);
void remove_monitor_k(Monitors*, int);

Monitor* get_primary_monitor(const Monitors*);
Monitor* get_monitor_from_position(const Monitors*, int, int);
Monitor* get_monitor_from_pointer_position(const Monitors*);
Monitor* get_monitor_from_client(const Monitors*, const Client*);

Monitor* get_monitor_from_list_membership(const Monitors*, const Client*);

void destroy_monitors(Monitors*);

int get_min_width_of_monitors();
int get_min_height_of_monitors();

Client* monitors_search_for_client(const Monitors*, Window, Monitor**);
int count_viewable_clients_in_monitors(const Monitors*);

void monitors_cancel_pending_unmaps(const Monitors*);
void monitors_cancel_cancel_pending_unmaps(const Monitors*);

void monitors_set_all_unmapped_from_workspace_switch(const Monitors*, _Bool);
void monitors_set_all_mapped_from_workspace_switch(const Monitors*, _Bool);

void monitors_cancel_ul_pending_maps(const Monitors*);
void monitors_cancel_cancel_ul_pending_maps(const Monitors*);

void monitors_map_unmap_cl(const Monitors*, MapClientFn, _Bool);
void monitors_map_unmap_fl(const Monitors*, MapClientFn, _Bool);
void monitors_map_unmap_ml(const Monitors*, MapClientFn, _Bool);
void monitors_map_unmap_ul(const Monitors*, MapClientFn, _Bool);
void monitors_map_unmap_dl(const Monitors*, MapClientFn, _Bool);

void monitors_cl_set_future_unmap_stay_unmapped(const Monitors*);
void monitors_ul_set_all_can_be_mapped(const Monitors*);

Client* monitors_focus_from_cl_head_or_tail(const Monitors*, int);
Client* monitors_focus_from_fl(const Monitors*);
Client* monitors_focus_from_ml(const Monitors*);

int monitors_get_smallest_x(const Monitors*);
int monitors_get_smallest_y(const Monitors*);

int monitors_get_largest_x(const Monitors*);
int monitors_get_largest_y(const Monitors*);

Monitor* get_monitor_to_right(const Monitors*, const Monitor*);
Monitor* get_monitor_to_left(const Monitors*, const Monitor*);
Monitor* get_monitor_above(const Monitors*, const Monitor*);
Monitor* get_monitor_below(const Monitors*, const Monitor*);

Monitor* reconcile_client_starting_monitor(const Monitors*, const Client*);
_Bool monitors_absorb_monitor(Monitors*, Monitor*);
Monitor* get_first_monitor_that_is_not(const Monitors*, const Monitor*);
Atom find_removed_monitor(Monitors*, XRRMonitorInfo*, int);
Atom find_added_monitor(Monitors*, XRRMonitorInfo*, int);
Monitor* find_new_monitor(Monitors*, XRRMonitorInfo*, int);
void monitors_update_internal_data(Monitors*, XRRMonitorInfo*, int);

void adjust_monitors_for_strut(Monitors*, StrutSide, int, int, int, int);
Monitor* move_client_to_monitor_with_position(Monitors*, Monitor*, Client*);
void move_persistent_docks_to_workspace(int, int);

#endif
