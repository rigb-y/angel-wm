/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "client.h"
#include "windows.h"
#include "colors.h"
#include "geometry.h"
#include "workspaces.h"
#include "client_list.h"
#include "unmapped_client.h"
#include "unmapped_list.h"
#include "focus_stack.h"
#include "monitor.h"
#include "monitors.h"

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdbool.h>

void init_client(Client* client, Window win) {
    if (client == NULL) return;
    client->win = win;

    client->x = 0, client->y = 0;
    client->w = 0, client->h = 0;

    client->in_resize_mode = false;
    client->leave_notify_from_motion = false;

    client->fullscreen = false;
    client->floating = false;
    client->minimized = false;

    client->next = NULL;
    client->fs_next = NULL;

    client->mapped = false;
    client->future_unmap_stay_unmapped = false;
    client->pending_unmap = 0;
    client->on_workspace = 0;
    client->should_set_focus = false;
    client->was_configured = false;
    client->state = STATE_UNKNOWN;
    client->moved = false;
    client->map_after_unmap_notify = false;
    client->unmap_after_map_notify = false;
    client->unmapped_from_workspace_switch = false;
    client->mapped_from_workspace_switch = false;
    client->persistent = false;

    client->dock = false;
}

_Bool client_persistent(Client* client) {
    if (client == NULL) return false;
    return client->persistent;
}

void client_set_persistent(Client* client, _Bool persistent) {
    if (client == NULL) return;
    client->persistent = persistent;
}

int client_pending_unmaps(Client* client) {
    if (client == NULL) return 0;
    return client->pending_unmap;
}

void inc_pending_unmap(Client* client) {
    if (client == NULL) return;
    ++client->pending_unmap;
}

void dec_pending_unmap(Client* client) {
    if (client == NULL) return;
    --client->pending_unmap;
}

Window get_client_win(const Client* client) {
    if (client == NULL) return 0;
    return client->win;
}

void set_client_border(Client* client, unsigned int border_width, Color color) {
    if (client == NULL || !is_client_mapped(client)) return;

    Window client_win = get_client_win(client);
    unsigned int bw = 
        client_is_fullscreen(client) 
        ? get_fs_border_width()
        : get_border_width(client);
    set_window_border_width(client_win, bw);
    set_window_border_color(client_win, color);
}

void set_client_in_resize_mode(Client* client, _Bool mode) {
    if (client == NULL) return;
    client->in_resize_mode = mode;
    set_client_border(client, get_border_width(client), get_border_color(client));
}

void set_client_in_resize_mode_no_border_change(Client* client, _Bool mode) {
    if (client == NULL) return;
    client->in_resize_mode = mode;
}

_Bool get_client_in_resize_mode(const Client* client) {
    if (client == NULL) return false;
    return client->in_resize_mode;
}

void set_client_leave_notify_from_motion(Client* client, _Bool flag) {
    if (client == NULL) return;
    client->leave_notify_from_motion = flag;
}

_Bool get_client_leave_notify_from_motion(const Client* client) {
    if (client == NULL) return false;
    return client->leave_notify_from_motion;
}

void clear_resize_step(Client* client) {
    set_client_resize_step_down(client, false); 
    set_client_resize_step_up(client, false); 
    set_client_resize_step_left(client, false); 
    set_client_resize_step_right(client, false); 
}

void set_client_resize_step(Client* client, int rsd, int rsu, int rsl, int rsr) {
    client->resize_step_down = rsd;
    client->resize_step_up = rsu;
    client->resize_step_left = rsl;
    client->resize_step_right = rsr;
}

void set_client_resize_step_down(Client* client, int amount) {
    if (client == NULL) return;
    if (amount < 0) amount = 0;
    client->resize_step_down = amount;
}

void inc_client_resize_step_down(Client* client) {
    if (client == NULL) return;
    ++client->resize_step_down; 
}

int get_client_resize_step_down(const Client* client) {
    if (client == NULL) return false;
    return client->resize_step_down;
}

void set_client_resize_step_up(Client* client, int amount) {
    if (client == NULL) return;
    if (amount < 0) amount = 0;
    client->resize_step_up = amount;
}

void inc_client_resize_step_up(Client* client) {
    if (client == NULL) return;
    ++client->resize_step_up; 
}

int get_client_resize_step_up(const Client* client) {
    if (client == NULL) return false;
    return client->resize_step_up;
}

void set_client_resize_step_left(Client* client, int amount) {
    if (client == NULL) return;
    if (amount < 0) amount = 0;
    client->resize_step_left = amount;
}

void inc_client_resize_step_left(Client* client) {
    if (client == NULL) return;
    ++client->resize_step_left; 
}

int get_client_resize_step_left(const Client* client) {
    if (client == NULL) return false;
    return client->resize_step_left;
}


void set_client_resize_step_right(Client* client, int amount) {
    if (client == NULL) return;
    if (amount < 0) amount = 0;
    client->resize_step_right = amount;
}

void inc_client_resize_step_right(Client* client) {
    if (client == NULL) return;
    ++client->resize_step_right; 
}

int get_client_resize_step_right(const Client* client) {
    if (client == NULL) return false;
    return client->resize_step_right;
}

void set_client_position(Client* client, int x, int y) {
    if (client == NULL) return;
    client->x = x, client->y = y;
}

int get_client_x(const Client* client) {
    if (client == NULL) return -1;
    return client->x;
}

int get_client_y(const Client* client) {
    if (client == NULL) return -1;
    return client->y;
}

void set_client_geometry(Client* client, int w, int h, unsigned int border_width) {
    if (client == NULL) return;
    client->w = w, client->h = h;
    client->border_width = border_width;
}

int get_client_width(const Client* client) {
	if (client == NULL) return 0;
    return client->w;
}

int get_client_height(const Client* client) {
	if (client == NULL) return 0;
    return client->h;
}

unsigned int get_client_border_width(const Client* client) {
	if (client == NULL) return 0;
    return client->border_width;
}

void swap_clients_geometry(Client* a, Client* b) {
    if (a == NULL || b == NULL) return;
    int a_x = a->x;
    int a_y = a->y;
    int a_w = a->w;
    int a_h = a->h;
    int a_bw = a->border_width;

    int a_rsd = a->resize_step_down;
    int a_rsu = a->resize_step_up;
    int a_rsl = a->resize_step_left;
    int a_rsr = a->resize_step_right;

    set_client_position(a, b->x, b->y);
    set_client_geometry(a, b->w, b->h, b->border_width);
    set_client_resize_step(
        a, 
        b->resize_step_down,
        b->resize_step_up,
        b->resize_step_left,
        b->resize_step_right
    );

    set_client_position(b, a_x, a_y);
    set_client_geometry(b, a_w, a_h, a_bw);
    set_client_resize_step(b, a_rsd, a_rsu, a_rsl, a_rsr);
}

_Bool client_is_current_focus(const Client* client) {
    if (client == NULL) return false;

    return get_current_focus() == client;
}

void client_set_fullscreen(Client* client, _Bool fullscreen) {
    if (client == NULL) return;
    client->fullscreen = fullscreen;
}

_Bool client_is_fullscreen(const Client* client) {
    if (client == NULL) return false;
    return client->fullscreen; 
}

void client_set_float(Client* client, _Bool should_float) {
    if (client == NULL) return;
    client->floating = should_float;
}

void client_set_minimized(Client* client, _Bool should_minimize) {
    if (client == NULL) return;
    client->minimized = should_minimize;
}

_Bool client_is_float(const Client* client) {
    if (client == NULL) return false;
    return client->floating; 
}

_Bool client_is_minimized(const Client* client) {
    if (client == NULL) return false;
    return client->minimized; 
}

_Bool is_client_mapped(const Client* client) {
    if (client == NULL) return false;
    return client->mapped && !client_state_pending_unmap(client);
}

void client_set_mapped(Client* client, _Bool mapped) {
    if (client == NULL) return;
    client->mapped = mapped;
}

void map_client(Client* client) {
    __map_client(client, true);
}

void __map_client(Client* client, _Bool unused) {
    (void)unused; 

    if (client == NULL 
        || is_client_mapped(client)
        || client_state_pending_map(client) 
        || client_state_pending_unmap(client)
    ) return;

    int workspace = get_client_on_workspace(client);

    Monitor* monitor = get_monitor_from_list_membership(
        get_workspace_monitors(workspace), client
    );

    UnmappedClient* unmapped = ul_find_from_client(
        get_monitor_ul(monitor), client
    );

    if (!unmapped_get_stay_unmapped(unmapped)) {
        client_set_transition_state(client, CLIENT_MAP_PENDING);
        map_window(get_client_win(client));
    }
}

void unmap_client(Client* client, _Bool still_manage) {
    if (client == NULL 
        || !is_client_mapped(client)
        || client_state_pending_map(client) 
        || client_state_pending_unmap(client)
    ) return;

    if (still_manage) 
        inc_pending_unmap(client);

    client_set_transition_state(client, CLIENT_UNMAP_PENDING);
    unmap_window(get_client_win(client));
}

void client_set_on_workspace(Client* client, int workspace) {
    if (client == NULL || workspace < 0 || workspace >= N_WORKSPACES)
        return;
    client->on_workspace = workspace;
}

int get_client_on_workspace(const Client* client) {
    if (client == NULL) return -1;
    return client->on_workspace;
}

void client_set_future_unmap_stay_unmapped(Client* client) {
    if (client == NULL) return;
    client->future_unmap_stay_unmapped = STAY_UNMAPPED;
}

void client_set_future_unmap_can_be_remapped(Client* client) {
    if (client == NULL) return;
    client->future_unmap_stay_unmapped = CAN_BE_MAPPED;
}

_Bool client_get_future_unmap_stay_unmapped(const Client* client) {
    if (client == NULL) return false;
    return client->future_unmap_stay_unmapped;
}

void client_set_should_set_focus(Client* client, _Bool set_focus) {
    if (client == NULL) return;
    client->should_set_focus = set_focus;
}

_Bool client_get_should_set_focus(const Client* client) {
    if (client == NULL) return false;
    return client->should_set_focus;
}

void client_set_was_configured(Client* client, _Bool configured) {
    if (client == NULL) return;
    client->was_configured = configured;
}

_Bool client_get_was_configured(const Client* client) {
    if (client == NULL) return false;
    return client->was_configured;
}

ClientTransitionState client_get_transitition_state(Client* client) {
    if (client == NULL) return STATE_UNKNOWN;
    return client->state;
}

void client_set_transition_state(Client* client, ClientTransitionState state) {
    if (client == NULL) return;
    client->state = state;
}

_Bool client_state_pending_map(const Client* client) {
    if (client == NULL) return false;
    return client->state == CLIENT_MAP_PENDING;
}

_Bool client_state_pending_unmap(const Client* client) {
    if (client == NULL) return false;
    return client->state == CLIENT_UNMAP_PENDING;
}

void client_set_moved(Client* client, _Bool moved) {
    if (client == NULL) return;
    client->moved = moved;
}

_Bool client_get_moved(const Client* client) {
    if (client == NULL) return false;
    return client->moved;
}

void client_set_map_after_unmap_notify(Client* client, _Bool map_after_unmap_notify) {
    if (client == NULL) return;
    client->map_after_unmap_notify = map_after_unmap_notify;
}

void client_set_unmap_after_map_notify(Client* client, _Bool unmap_after_map_notify) {
    if (client == NULL) return;
    client->unmap_after_map_notify = unmap_after_map_notify;
}

_Bool client_get_map_after_unmap_notify(const Client* client) {
    if (client == NULL) return false;
    return client->map_after_unmap_notify;
}

_Bool client_get_unmap_after_map_notify(const Client* client) {
    if (client == NULL) return false;
    return client->unmap_after_map_notify;
}

void client_set_unmapped_from_workspace_switch(Client* client, _Bool unmapped_from_switch) {
    if (client == NULL) return;
    client->unmapped_from_workspace_switch = unmapped_from_switch;
}

_Bool client_get_unmapped_from_workspace_switch(const Client* client) {
    if (client == NULL) return false;
    return client->unmapped_from_workspace_switch;
}

void client_set_mapped_from_workspace_switch(Client* client, _Bool mapped_from_switch) {
    if (client == NULL) return;
    client->mapped_from_workspace_switch = mapped_from_switch;
}

_Bool client_get_mapped_from_workspace_switch(const Client* client) {
    if (client == NULL) return false;
    return client->mapped_from_workspace_switch;
}

_Bool client_is_dock(const Client* client) {
    if (client == NULL) return false;
    return client->dock;
}

void client_set_dock(Client* client, _Bool dock) {
    if (client == NULL) return;
    client->dock = dock;
}
