/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_MONITOR_H
#define ANGEL_MONITOR_H

#include "types.h"
#include "layouts.h"

#include <X11/Xlib.h>

typedef struct ClientList ClientList;
typedef struct MotionTree MotionTree;
typedef struct Client Client;
typedef struct DetachedClient DetachedClient;
typedef struct MinimizedClient MinimizedClient;
typedef struct UnmappedClient UnmappedClient;
typedef struct FloatingClients FloatingClients;
typedef struct MinimizedList MinimizedList;
typedef struct UnmappedClients UnmappedClients;
typedef struct Docks Docks;

typedef struct Monitor {
    Atom name;
    _Bool primary;

    ClientList* cl;
    FloatingClients* fl;
    MinimizedList* ml;
    UnmappedClients* ul;
    MotionTree* mt;
    Client* current_fullscreen;
    Docks* dl;

    int x, y;
    int start_x, start_y;

    int width, height;
    int usable_width, usable_height;

    MinimizedPosition minimized_position;

    Layout layout;

    struct Monitor* next;
} Monitor;

void init_monitor(Monitor*, Atom, _Bool, int, int, int, int);
Monitor* create_monitor(Atom, _Bool, int, int, int, int);
void free_monitor_resources(Monitor*);
void destroy_monitor(Monitor*);

MotionTree* get_monitor_mt(const Monitor*);

Atom get_monitor_name(const Monitor*);
_Bool is_monitor_primary(const Monitor*);
void set_monitor_primary(Monitor*, _Bool);

ClientList* get_monitor_cl(const Monitor*);
FloatingClients* get_monitor_fl(const Monitor*);
MinimizedList* get_monitor_ml(const Monitor*);
UnmappedClients* get_monitor_ul(const Monitor*);
Docks* get_monitor_dl(const Monitor*);

Client* get_monitor_fullscreen(const Monitor*);
void set_monitor_fullscreen(Monitor*, Client*);
_Bool monitor_fullscreen_exists(const Monitor*);

int monitor_x(const Monitor*);
int monitor_y(const Monitor*);
void update_monitor_x(Monitor*, int);
void update_monitor_y(Monitor*, int);

int monitor_width(const Monitor*);
int monitor_height(const Monitor*);
void update_monitor_width(Monitor*, int);
void update_monitor_height(Monitor*, int);

int monitor_usable_width(const Monitor*);
int monitor_usable_height(const Monitor*);

void update_monitor_usable_width(Monitor*, int);
void update_monitor_usable_height(Monitor*, int);

int monitor_start_x(const Monitor*);
int monitor_start_y(const Monitor*);

void update_monitor_start_x(Monitor*, int);
void update_monitor_start_y(Monitor*, int);

MinimizedPosition monitor_minimized_position(const Monitor*);
void set_monitor_minimized_position(Monitor*, MinimizedPosition);

void monitor_move_all_float_to_tiled(const Monitor*);
void monitor_move_all_tiled_to_foat(const Monitor*);

void monitor_move_all_to_minimize(const Monitor*);
void monitor_reattach_all_minimized(const Monitor*);

Layout get_monitor_layout(const Monitor*, int);
void set_monitor_layout(Monitor*, Layout);

void move_client_to_monitor(Client*, Monitor*, Monitor*);
Client* monitor_get_first_client(Monitor*);

void absorb_monitor(Monitor*, Monitor*);
void update_monitor_on_strut_intersect(Monitor*, StrutSide, int, int, int, int);
void move_docks_to_monitor(Monitor*, Docks*, int);

#endif
