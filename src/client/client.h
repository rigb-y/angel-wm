/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_CLIENT_H
#define ANGEL_CLIENT_H

#include <X11/Xlib.h>

#define MANAGE_CONT true
#define MANAGE_ENDED false
#define NO_LONGER_UNMAPPED false

typedef struct Color Color;

typedef enum ClientTransitionState {
    CLIENT_UNMAPPED, CLIENT_UNMAP_PENDING,
    CLIENT_MAPPED, CLIENT_MAP_PENDING, STATE_UNKNOWN
} ClientTransitionState;

typedef struct Client {
    Window win;
    int x,y;

    // Not including border width
    int w,h;

    unsigned int border_width;

    struct Client* next;
    struct Client* fs_next;

    _Bool in_resize_mode;

    int resize_step_down;
    int resize_step_up;
    int resize_step_left;
    int resize_step_right;

    _Bool leave_notify_from_motion;

    _Bool fullscreen;
    _Bool floating;
    _Bool minimized;

    _Bool mapped;
    _Bool future_unmap_stay_unmapped;
    _Bool should_set_focus;
    int pending_unmap;

    int on_workspace;

    _Bool was_configured;

    ClientTransitionState state;

    _Bool moved;
    _Bool map_after_unmap_notify;
    _Bool unmap_after_map_notify;
    _Bool unmapped_from_workspace_switch;
    _Bool mapped_from_workspace_switch;

    _Bool dock;
} Client;

void init_client(Client*, Window);

int client_pending_unmaps(Client*);
void inc_pending_unmap(Client*);
void dec_pending_unmap(Client*);

void refresh_client_attributes(Client*);

Window get_client_win(const Client*);

void set_client_border(Client*, unsigned int, Color);

void set_client_in_resize_mode(Client*, _Bool);
void set_client_in_resize_mode_no_border_change(Client*, _Bool);
_Bool get_client_in_resize_mode(const Client*);

void set_client_leave_notify_from_motion(Client*, _Bool);
_Bool get_client_leave_notify_from_motion(const Client*);

void clear_client_resize_step(Client*);
void set_client_resize_step(Client*, int, int, int, int);

void set_client_resize_step_down(Client*, int);
void inc_client_resize_step_down(Client*);
int get_client_resize_step_down(const Client*);

void set_client_resize_step_up(Client*, int);
void inc_client_resize_step_up(Client*);
int get_client_resize_step_up(const Client*);

void set_client_resize_step_left(Client*, int);
void inc_client_resize_step_left(Client*);
int get_client_resize_step_left(const Client*);

void set_client_resize_step_right(Client*, int);
void inc_client_resize_step_right(Client*);
int get_client_resize_step_right(const Client*);

void set_client_position(Client*, int, int);
int get_client_x(const Client*);
int get_client_y(const Client*);

void set_client_geometry(Client*, int, int, unsigned int);
int get_client_width(const Client*);
int get_client_height(const Client*);
unsigned int get_client_border_width(const Client*);

void swap_clients_geometry(Client*, Client*);

_Bool client_is_current_focus(const Client*);

void client_set_fullscreen(Client*, _Bool);
void client_set_float(Client*, _Bool);
void client_set_minimized(Client*, _Bool);

_Bool client_is_fullscreen(const Client*);
_Bool client_is_float(const Client*);
_Bool client_is_minimized(const Client*);

_Bool is_client_mapped(const Client*);
void map_client(Client*);
void __map_client(Client*, _Bool);
void unmap_client(Client*, _Bool);

void client_set_on_workspace(Client*, int);
int get_client_on_workspace(const Client*);

void client_set_mapped(Client*, _Bool);

void client_set_future_unmap_stay_unmapped(Client*);
void client_set_future_unmap_can_be_remapped(Client*);

_Bool client_get_future_unmap_stay_unmapped(const Client*);

void client_set_should_set_focus(Client*, _Bool);
_Bool client_get_should_set_focus(const Client*);

void client_set_was_configured(Client*, _Bool);
_Bool client_get_was_configured(const Client*);

ClientTransitionState client_get_transitition_state(Client*);
void client_set_transition_state(Client*, ClientTransitionState);

_Bool client_state_pending_map(const Client*);
_Bool client_state_pending_unmap(const Client*);

void client_set_moved(Client*, _Bool);
_Bool client_get_moved(const Client*);

void client_set_map_after_unmap_notify(Client*, _Bool);
void client_set_unmap_after_map_notify(Client*, _Bool);

_Bool client_get_map_after_unmap_notify(const Client*);
_Bool client_get_unmap_after_map_notify(const Client*);

void client_set_unmapped_from_workspace_switch(Client*, _Bool);
_Bool client_get_unmapped_from_workspace_switch(const Client*);

void client_set_mapped_from_workspace_switch(Client*, _Bool);
_Bool client_get_mapped_from_workspace_switch(const Client*);

_Bool client_is_dock(const Client*);
void client_set_dock(Client*, _Bool);

#endif
