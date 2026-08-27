/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "monitor.h"
#include "client_list.h"
#include "float_list.h"
#include "minimized_list.h"
#include "unmapped_list.h"
#include "client.h"
#include "geometry.h"
#include "motion_tree.h"
#include "client_list.h"
#include "float_list.h"
#include "unmapped_list.h"
#include "minimized_list.h"
#include "detached.h"
#include "minimized_client.h"
#include "layouts.h"
#include "workspaces.h"
#include "unmapped_client.h"
#include "docks.h"
#include "utils.h"
#include "ewmh.h"

#include <X11/Xlib.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

static void monitor_move_cl_to_minimize(const Monitor*);
static void monitor_move_fl_to_minimize(const Monitor*);

static void move_client_to_monitor_from_cl(Client*, Monitor*, Monitor*);
static void move_client_to_monitor_from_fl(Client*, Monitor*, Monitor*);
static void move_client_to_monitor_from_ml(Client*, Monitor*, Monitor*);
static void move_client_to_monitor_from_dl(Client*, Monitor*, Monitor*);
static void move_client_to_monitor_from_ul(Client*, Monitor*, Monitor*);

static void absorb_monitor_cl(Monitor*, Monitor*);
static void absorb_monitor_fl(Monitor*, Monitor*);
static void absorb_monitor_ml(Monitor*, Monitor*);
static void absorb_monitor_ul(Monitor*, Monitor*);
static void absorb_monitor_dl(Monitor*, Monitor*);

static void monitor_move_persistent_tiled(Monitor*, Monitor*, int);
static void monitor_move_persistent_float(Monitor*, Monitor*, int);
static void monitor_move_persistent_minimized(Monitor*, Monitor*, int);
static void monitor_move_persistent_unmapped(Monitor*, Monitor*, int);

static void monitor_change_client_state_after_move(Client*, int);
static void monitor_change_fullscreen_state_after_move(Client*, Monitor*, Monitor*);

void init_monitor(
    Monitor* monitor,
    Atom name,
    _Bool primary,
    int x, int y,
    int width, int height
) {
    if (monitor == NULL) return;

    monitor->cl = calloc(1, sizeof(ClientList));
    monitor->fl = calloc(1, sizeof(FloatingClients));
    monitor->ml = calloc(1, sizeof(MinimizedList));
    monitor->ul = calloc(1, sizeof(UnmappedClients));
    monitor->mt = calloc(1, sizeof(MotionTree));
    monitor->dl = calloc(1, sizeof(Docks));
    monitor->current_fullscreen = NULL;

    monitor->x = x;
    monitor->y = y;

    monitor->start_x = x;
    monitor->start_y = y;

    monitor->width = width;
    monitor->height = height;

    monitor->usable_width = width; 
    monitor->usable_height = height;

    monitor->primary = primary;
    monitor->name = name;

    monitor->layout = ANGEL_LAYOUT_UNKNOWN;
    monitor->minimized_position = MINIMIZED_POSITION_UNKNOWN;

    monitor->next = NULL;
}

Monitor* create_monitor(
    Atom name,
    _Bool primary,
    int x, int y,
    int width, int height
) {
    Monitor* monitor;
    if ((monitor = calloc(1, sizeof(Monitor))) == NULL)
        return NULL;

    init_monitor(monitor, name, primary, x, y, width, height);
    return monitor;
}

void free_monitor_resources(Monitor* monitor) {
    if (monitor == NULL) return;

    cl_destroy(monitor->cl);
    free(monitor->cl);

    fl_destroy(monitor->fl);
    free(monitor->fl);

    ml_destroy(monitor->ml);
    free(monitor->ml);

    ul_destroy(monitor->ul);
    free(monitor->ul);

    mt_destroy(monitor->mt);
    free(monitor->mt);

    docks_destroy(monitor->dl);
    free(monitor->dl);
}

void destroy_monitor(Monitor* monitor) {
    if (monitor == NULL) return;

    free_monitor_resources(monitor);
    monitor->cl = NULL;
    monitor->fl = NULL;
    monitor->ml = NULL;
    monitor->ul = NULL;
    monitor->mt = NULL;
    monitor->dl = NULL;

    monitor->current_fullscreen = NULL;
    monitor->minimized_position = MINIMIZED_POSITION_UNKNOWN;
    monitor->layout = ANGEL_LAYOUT_UNKNOWN;

    monitor->x = 0;
    monitor->y = 0;

    monitor->start_x = 0;
    monitor->start_y = 0;

    monitor->width = 0;
    monitor->height = 0;

    monitor->usable_width = 0;
    monitor->usable_height = 0;

    monitor->name = None;
    monitor->primary = false;

    monitor->next = NULL;
}

MotionTree* get_monitor_mt(const Monitor* monitor) {
    if (monitor == NULL) return NULL; 
    return monitor->mt;
}

Atom get_monitor_name(const Monitor* monitor) {
    if (monitor == NULL) return None;
    return monitor->name;
}

_Bool is_monitor_primary(const Monitor* monitor) {
    if (monitor == NULL) return false;
    return monitor->primary;
}

void set_monitor_primary(Monitor* monitor, _Bool primary) {
    if (monitor == NULL) return;
    monitor->primary = primary;
}

ClientList* get_monitor_cl(const Monitor* monitor) {
    if (monitor == NULL) return NULL;
    return monitor->cl;
}

FloatingClients* get_monitor_fl(const Monitor* monitor) {
    if (monitor == NULL) return NULL;
    return monitor->fl;
}

MinimizedList* get_monitor_ml(const Monitor* monitor) {
    if (monitor == NULL) return NULL;
    return monitor->ml;
}

UnmappedClients* get_monitor_ul(const Monitor* monitor) {
    if (monitor == NULL) return NULL;
    return monitor->ul;
}

Docks* get_monitor_dl(const Monitor* monitor) {
    if (monitor == NULL) return NULL;
    return monitor->dl;
}

int monitor_x(const Monitor* monitor) {
    if (monitor == NULL) return 0;
    return monitor->x;
}

int monitor_y(const Monitor* monitor) {
    if (monitor == NULL) return 0;
    return monitor->y;
}

void update_monitor_x(Monitor* monitor, int x) {
    if (monitor == NULL) return;
    monitor->x = x;
}

void update_monitor_y(Monitor* monitor, int y) {
    if (monitor == NULL) return;
    monitor->y = y;
}

int monitor_width(const Monitor* monitor) {
    if (monitor == NULL) return 0;
    return monitor->width;
}

int monitor_height(const Monitor* monitor) {
    if (monitor == NULL) return 0;
    return monitor->height;
}

void update_monitor_width(Monitor* monitor, int w) {
    if (monitor == NULL) return;
    monitor->width = w;
}

void update_monitor_height(Monitor* monitor, int h) {
    if (monitor == NULL) return;
    monitor->height = h;
}

Client* get_monitor_fullscreen(const Monitor* monitor) {
    if (monitor == NULL) return NULL;
    return monitor->current_fullscreen;
}

void set_monitor_fullscreen(Monitor* monitor, Client* client) {
    if (monitor == NULL) return;
    monitor->current_fullscreen = client;
}

_Bool monitor_fullscreen_exists(const Monitor* monitor) {
    if (monitor == NULL) return false;
    return get_monitor_fullscreen(monitor) != NULL;
}

int monitor_usable_width(const Monitor* monitor) {
    if (monitor == NULL) return 0;
    return monitor->usable_width;
}

int monitor_usable_height(const Monitor* monitor) {
    if (monitor == NULL) return 0;
    return monitor->usable_height;
}

void update_monitor_usable_width(Monitor* monitor, int width) {
    if (monitor == NULL) return;
    monitor->usable_width = width;
}

void update_monitor_usable_height(Monitor* monitor, int height) {
    if (monitor == NULL) return;
    monitor->usable_height = height;
}

int monitor_start_x(const Monitor* monitor) {
    if (monitor == NULL) return 0;
    return monitor->start_x;
}

int monitor_start_y(const Monitor* monitor) {
    if (monitor == NULL) return 0;
    return monitor->start_y;
}

void update_monitor_start_x(Monitor* monitor, int x) {
    if (monitor == NULL) return;
    monitor->start_x = x;
}

void update_monitor_start_y(Monitor* monitor, int y) {
    if (monitor == NULL) return;
    monitor->start_y = y;
}

MinimizedPosition monitor_minimized_position(const Monitor* monitor) {
    if (monitor == NULL) return MINIMIZED_POSITION_UNKNOWN;
    return monitor->minimized_position;
}

void set_monitor_minimized_position(Monitor* monitor, MinimizedPosition minimized_position) {
    if (monitor == NULL ||
        (minimized_position != MINIMIZED_BOTTOM
         && minimized_position != MINIMIZED_TOP
         && minimized_position != MINIMIZED_LEFT
         && minimized_position != MINIMIZED_RIGHT
    )) return;

    monitor->minimized_position = minimized_position;
}

void monitor_move_all_float_to_tiled(const Monitor* monitor) {
    if (monitor == NULL) return;

    FloatingClients* fl = get_monitor_fl(monitor);
    DetachedClient* curr = fl_head(fl);
    while (curr != NULL) {
        DetachedClient* tmp = curr;
        curr = curr->next;

        fl_reattach_into_cl(fl, tmp);
    }
}

void monitor_move_all_tiled_to_foat(const Monitor* monitor) {
    if (monitor == NULL) return;

    Client* curr = cl_head(get_monitor_cl(monitor));
    while (curr != NULL) {
        Client* tmp = curr;
        curr = curr->next;

        DetachedClient* detached = detach_client(tmp);
        fl_push(get_monitor_fl(monitor), detached);
    }
}

void monitor_move_cl_to_minimize(const Monitor* monitor) {
    if (monitor == NULL) return;

    ClientList* cl = get_monitor_cl(monitor);
    MinimizedList* ml = get_monitor_ml(monitor);
    Client* curr = cl_head(cl);
    while (curr != NULL) {
        Client* tmp = curr;
        curr = curr->next;

        MinimizedClient* minimized = minimize_client(tmp);
        ml_push(ml, minimized);
    }
}

void monitor_move_fl_to_minimize(const Monitor* monitor) {
    if (monitor == NULL) return;

    FloatingClients* fl = get_monitor_fl(monitor);
    MinimizedList* ml = get_monitor_ml(monitor);
    DetachedClient* curr = fl_head(fl);
    while (curr != NULL) {
        DetachedClient* tmp = curr;
        curr = curr->next;

        Client* client = get_client_from_detached(tmp);

        MinimizedClient* minimized = minimize_client(client);
        ml_push(ml, minimized);
    }
}

void monitor_move_all_to_minimize(const Monitor* monitor) {
    if (monitor == NULL) return;

    monitor_move_cl_to_minimize(monitor);
    monitor_move_fl_to_minimize(monitor);
}

void monitor_reattach_all_minimized(const Monitor* monitor) {
    if (monitor == NULL) return;

    MinimizedList* ml = get_monitor_ml(monitor);
    MinimizedClient* curr = ml_head(ml);
    while (curr != NULL) {
        MinimizedClient* tmp = curr;
        curr = curr->next;

        ml_reattach(ml, tmp);
    }
}

Layout get_monitor_layout(const Monitor* monitor, int workspace) {
    if (monitor == NULL || !workspace_is_valid(workspace)) return ANGEL_LAYOUT_UNKNOWN;

    Layout layout = monitor->layout;
    Layout fallback = get_workspace_layout(workspace);
    return layout != ANGEL_LAYOUT_UNKNOWN
        ? layout 
        : fallback != ANGEL_LAYOUT_UNKNOWN 
        ? fallback 
        : default_workspace_layout;
}

void set_monitor_layout(Monitor* monitor, Layout layout) {
    if (monitor == NULL || !is_layout_accepted(layout)) return;
    monitor->layout = layout;
}

void move_client_to_monitor_from_cl(Client* client, Monitor* from, Monitor* to) {
    if (client == NULL || from == NULL || to == NULL || from == to)
        return;

    cl_remove(get_monitor_cl(from), &client, NO_FREE);
    cl_append(get_monitor_cl(to), client);
}

void move_client_to_monitor_from_fl(Client* client, Monitor* from, Monitor* to) {
    if (client == NULL || from == NULL || to == NULL || from == to)
        return;

    DetachedClient* detached = fl_find_from_client(
        get_monitor_fl(from),
        client
    );

    fl_remove(get_monitor_fl(from), detached);
    fl_push(get_monitor_fl(to), detached);
}

void move_client_to_monitor_from_ml(Client* client, Monitor* from, Monitor* to) {
    if (client == NULL || from == NULL || to == NULL || from == to)
        return;

    MinimizedClient* minimized = ml_find_from_client(
        get_monitor_ml(from),
        client
    );

    ml_remove(get_monitor_ml(from), minimized);
    ml_push(get_monitor_ml(to), minimized);
}

void move_client_to_monitor_from_dl(Client* client, Monitor* from, Monitor* to) {
    if (client == NULL || from == NULL || to == NULL || from == to)
        return;

    Dock* dock = docks_find_from_client(
        get_monitor_dl(from),
        client
    );

    docks_remove(get_monitor_dl(from), dock);
    docks_push(get_monitor_dl(to), dock);
}

void move_client_to_monitor_from_ul(Client* client, Monitor* from, Monitor* to) {
    if (client == NULL || from == NULL || to == NULL || from == to)
        return;

    UnmappedClient* unmapped = ul_find_from_client(
        get_monitor_ul(from),
        client
    );

    ul_remove(get_monitor_ul(from), unmapped);
    ul_push(get_monitor_ul(to), unmapped);
}

void move_client_to_monitor(Client* client, Monitor* from, Monitor* to) {
    if (client == NULL 
        || from == NULL 
        || to == NULL 
        || from == to 
        || !is_client_mapped(client)
    ) return;

    if (client_is_float(client))
        move_client_to_monitor_from_fl(client, from, to);
    else if (client_is_minimized(client))
        move_client_to_monitor_from_ml(client, from, to);
    else if (client_is_dock(client))
        move_client_to_monitor_from_dl(client, from, to);
    else if (!is_client_mapped(client))
        move_client_to_monitor_from_ul(client, from, to);
    else
        move_client_to_monitor_from_cl(client, from, to);
}

Client* monitor_get_first_client(Monitor* monitor) {
    if (monitor == NULL) return NULL;

    Client* client = cl_head(get_monitor_cl(monitor));
    if (client != NULL)
        return client;

    client = get_client_from_detached(
        fl_head(
            get_monitor_fl(monitor)
        )
    );

    if (client != NULL)
        return client;

    client = get_client_from_minimized(
        ml_head(
            get_monitor_ml(monitor)
        )
    );

    if (client != NULL)
        return client;

    return NULL;
}

void absorb_monitor_cl(Monitor* to, Monitor* from) {
    if (to == NULL || from == NULL || to == from)
        return;

    ClientList* from_cl = get_monitor_cl(from);
    ClientList* to_cl = get_monitor_cl(to);

    Client* curr = cl_head(from_cl);
    while (curr != NULL) {
        Client* tmp = curr;
        curr = curr->next;
        cl_remove(from_cl, &tmp, NO_FREE);
        cl_push(to_cl, tmp);
    }
}

void absorb_monitor_fl(Monitor* to, Monitor* from) {
    if (to == NULL || from == NULL || to == from)
        return;

    FloatingClients* from_fl = get_monitor_fl(from);
    FloatingClients* to_fl = get_monitor_fl(to);

    DetachedClient* curr = fl_head(from_fl);
    while (curr != NULL) {
        DetachedClient* tmp = curr;
        curr = curr->next;
        fl_remove(from_fl, tmp);
        fl_push(to_fl, tmp);
    }
}

void absorb_monitor_ml(Monitor* to, Monitor* from) {
    if (to == NULL || from == NULL || to == from)
        return;

    MinimizedList* from_ml = get_monitor_ml(from);
    MinimizedList* to_ml = get_monitor_ml(to);

    MinimizedClient* curr = ml_head(from_ml);
    while (curr != NULL) {
        MinimizedClient* tmp = curr;
        curr = curr->next;
        ml_remove(from_ml, tmp);
        ml_push(to_ml, tmp);
    }
}

void absorb_monitor_ul(Monitor* to, Monitor* from) {
    if (to == NULL || from == NULL || to == from)
        return;

    UnmappedClients* from_ul = get_monitor_ul(from);
    UnmappedClients* to_ul = get_monitor_ul(to);

    UnmappedClient* curr = ul_head(from_ul);
    while (curr != NULL) {
        UnmappedClient* tmp = curr;
        curr = curr->next;
        ul_remove(from_ul, tmp);
        ul_push(to_ul, tmp);
    }
}

void absorb_monitor_dl(Monitor* to, Monitor* from) {
    if (to == NULL || from == NULL || to == from)
        return;

    Docks* from_dl = get_monitor_dl(from);
    Docks* to_dl = get_monitor_dl(to);

    Dock* curr = from_dl->head;
    while (curr != NULL) {
        Dock* tmp = curr;
        curr = curr->next;
        docks_remove(from_dl, tmp);
        docks_push(to_dl, tmp);
    }
}

void absorb_monitor(Monitor* to, Monitor* from) {
    if (to == NULL || from == NULL || to == from)
        return;

    absorb_monitor_cl(to, from);
    absorb_monitor_fl(to, from);
    absorb_monitor_ml(to, from);
    absorb_monitor_ul(to, from);
    absorb_monitor_dl(to, from);
}

void update_monitor_on_strut_intersect(
    Monitor* monitor,
    StrutSide strut_side,
    int x, int y,
    int w, int h
) {
    if (monitor == NULL) return;

    int msx = monitor_start_x(monitor);
    int msy = monitor_start_y(monitor);
    int muw = monitor_usable_width(monitor);
    int muh = monitor_usable_height(monitor);

    int x_maxl = max(msx, x);
    int x_minr = min(msx + muw, x + w);

    int y_maxt = max(msy, y);
    int y_minb = min(msy + muh, y + h);

    if (x_maxl >= x_minr || y_maxt >= y_minb)
        return;

    int iw = x_minr - x_maxl;
    int ih = y_minb - y_maxt;

    switch (strut_side) {
        case STRUT_SIDE_LEFT:
            if (x_maxl == msx) {
                update_monitor_start_x(monitor, msx + iw);
                update_monitor_usable_width(monitor, muw - iw);
            }
            break;

        case STRUT_SIDE_RIGHT:
            if (x_minr == msx + muw)
                update_monitor_usable_width(monitor, muw - iw);
            break;

        case STRUT_SIDE_TOP:
            if (y_maxt == msy) {
                update_monitor_start_y(monitor, msy + ih);
                update_monitor_usable_height(monitor, muh - ih);
            }
            break;

        case STRUT_SIDE_BOTTOM:
            if (y_minb == msy + muh) 
                update_monitor_usable_height(monitor, muh - ih);
            break;
    }
}

void move_docks_to_monitor(Monitor* monitor, Docks* docks, int to_workspace) {
    if (monitor == NULL || docks == NULL) return;

    Docks* to_docks = get_monitor_dl(monitor);
    if (to_docks == NULL)
        return;

    Dock* curr = docks->head;
    while (curr != NULL) {
        if (!get_dock_on_all_workspaces(curr)) {
            curr = curr->next;
            continue;
        }

        Dock* tmp = curr;
        curr = curr->next;
        docks_remove(docks, tmp);
        docks_push(to_docks, tmp);
        client_set_on_workspace(
            get_client_from_dock(tmp),
            to_workspace
        );
    }
}

void monitor_change_client_state_after_move(Client* client, int to_workspace) {
    if (client == NULL || !workspace_is_valid(to_workspace)) return;

    client_set_on_workspace(client, to_workspace);
    ewmh_store_workspace_num(get_client_win(client), (unsigned long) to_workspace);
    client_set_was_configured(client, false);
}

void monitor_change_fullscreen_state_after_move(Client* client, Monitor* from, Monitor* to) {
    if (client == NULL || from == NULL || to == NULL || from == to) return;

    if (get_monitor_fullscreen(from) != client)
        return;

    set_monitor_fullscreen(from, NULL);
    set_monitor_fullscreen(to, client);
}

void monitor_move_persistent_tiled(Monitor* from, Monitor* to, int to_workspace) {
    if (from == NULL || to == NULL || from == to) return;

    Client* curr = cl_head(get_monitor_cl(from));
    while (curr != NULL) {
        if (!client_persistent(curr)) {
            curr=curr->next;
            continue;
        }


        move_client_to_monitor(curr, from, to);
        monitor_change_fullscreen_state_after_move(curr, from, to);
        monitor_change_client_state_after_move(curr, to_workspace);

        curr = curr->next;
    }
}

void monitor_move_persistent_float(Monitor* from, Monitor* to, int to_workspace) {
    if (from == NULL || to == NULL || from == to) return;

    DetachedClient* curr = fl_head(get_monitor_fl(from));
    while (curr != NULL) {
        Client* client = get_client_from_detached(curr);
        if (!client_persistent(client)) {
            curr=curr->next;
            continue;
        }

        move_client_to_monitor(client, from, to);
        monitor_change_fullscreen_state_after_move(client, from, to);
        monitor_change_client_state_after_move(client, to_workspace);

        curr = curr->next;
    }
}

void monitor_move_persistent_minimized(Monitor* from, Monitor* to, int to_workspace) {
    if (from == NULL || to == NULL || from == to) return;

    MinimizedClient* curr = ml_head(get_monitor_ml(from));
    while (curr != NULL) {
        Client* client = get_client_from_minimized(curr);
        if (!client_persistent(client)) {
            curr=curr->next;
            continue;
        }

        move_client_to_monitor(client, from, to);
        monitor_change_fullscreen_state_after_move(client, from, to);
        monitor_change_client_state_after_move(client, to_workspace);

        curr = curr->next;
    }
}

void monitor_move_persistent_unmapped(Monitor* from, Monitor* to, int to_workspace) {
    if (from == NULL || to == NULL || from == to) return;

    UnmappedClient* curr = ul_head(get_monitor_ul(from));
    while (curr != NULL) {
        Client* client = get_client_from_unmapped(curr);
        if (!client_persistent(client)) {
            curr=curr->next;
            continue;
        }

        move_client_to_monitor(client, from, to);
        monitor_change_fullscreen_state_after_move(client, from, to);
        monitor_change_client_state_after_move(client, to_workspace);

        curr = curr->next;
    }
}

void monitor_move_persistent(Monitor* from, Monitor* to, int to_workspace) {
    if (from == NULL || to == NULL || from == to) return;

    monitor_move_persistent_tiled(from, to, to_workspace);
    monitor_move_persistent_float(from, to, to_workspace);
    monitor_move_persistent_minimized(from, to, to_workspace);
    monitor_move_persistent_unmapped(from, to, to_workspace);
}
