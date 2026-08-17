/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "monitors.h"
#include "monitor.h"
#include "client.h"
#include "client_list.h"
#include "detached.h"
#include "float_list.h"
#include "minimized_client.h"
#include "minimized_list.h"
#include "unmapped_client.h"
#include "unmapped_list.h"
#include "workspaces.h"
#include "utils.h"
#include "types.h"
#include "pointer.h"
#include "geometry.h"
#include "layouts.h"
#include "ewmh.h"
#include "focus.h"
#include "docks.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

static _Bool monitor_number_valid(const Monitors*, int);

static void monitors_cancel_cl_pending_unmaps(const Monitors*);
static void monitors_cancel_fl_pending_unmaps(const Monitors*);
static void monitors_cancel_ml_pending_unmaps(const Monitors*);
static void monitors_cancel_dl_pending_unmaps(const Monitors*);
static void monitors_cancel_cancel_cl_pending_unmaps(const Monitors*);
static void monitors_cancel_cancel_fl_pending_unmaps(const Monitors*);
static void monitors_cancel_cancel_ml_pending_unmaps(const Monitors*);
static void monitors_cancel_cancel_dl_pending_unmaps(const Monitors*);

static int monitors_get_left_position_extremes(
    const Monitors*,
    MinOrMaxFn,
    MonitorPositionInfoFn
);

static int monitors_get_right_position_extremes(
    const Monitors*,
    MonitorPositionInfoFn,
    MonitorGeometryInfoFn
);

static Monitor* get_adjacent_monitor(
    const Monitors*,
    const Monitor*,
    int, 
    IntCompareFn,
    IntCompareFn,
    MonitorGeometryInfoFn,
    MonitorGeometryInfoFn
);

_Bool monitor_number_valid(const Monitors* monitors, int k) {
    return k >= 0 && k < number_of_monitors(monitors);
}

int get_monitors_size(const Monitors* monitors) {
    if (monitors == NULL) return 0;
    return monitors->n;
}

Monitor* monitors_head(const Monitors* monitors) {
    if (monitors == NULL) return NULL;
    return monitors->head;
}

Monitor* get_monitor_with_name(const Monitors* monitors, Atom name) {
    if (monitors == NULL || name == None) return NULL;

    Monitor* curr = monitors->head;
    while (curr != NULL && curr->name != name)
        curr = curr->next;

    return curr;
}

_Bool find_monitor(const Monitors* monitors, const Monitor* monitor) {
    if (monitors == NULL || monitor == NULL) return false;
    
    Monitor* curr = monitors->head;
    while (curr != NULL) {
        if (curr == monitor)
            return true;
        curr = curr->next;
    }
    
    return false;
}

void push_monitor(Monitors* monitors, Monitor* monitor) {
    if (monitors == NULL 
        || monitor == NULL 
        || find_monitor(monitors, monitor)
    ) return;

    if (monitors->head == NULL) {
        monitors->head = monitor;
        monitors->head->next = NULL;

        ++monitors->n;
        return;
    }

    Monitor* parent = monitors->head;
    while (parent->next != NULL) {
        parent = parent->next;
    }

    parent->next = monitor;
    monitor->next = NULL;
    ++monitors->n;
}

Monitor* get_monitor_parent(const Monitors* monitors, const Monitor* monitor) {
    if (monitors == NULL 
        || monitor == NULL 
        || !find_monitor(monitors, monitor)
    ) return NULL;

    if (monitors->head == monitor) return NULL;

    Monitor* parent = monitors->head;
    while (parent->next != monitor) {
        parent = parent->next;
    }

    return parent;
}

void remove_monitor(Monitors* monitors , Monitor* monitor) {
    if (monitors == NULL 
        || monitor == NULL 
        || !find_monitor(monitors, monitor)
    ) return;

    if (monitors->head == monitor) {
        monitors->head = monitors->head->next;
        monitor->next = NULL;

        --monitors->n;
        return;
    }

    Monitor* parent = get_monitor_parent(monitors, monitor);
    if (parent == NULL)
        return;

    parent->next = monitor->next;
    monitor->next = NULL;
    --monitors->n;
}

Monitor* get_monitor_k(const Monitors* monitors, int k) {
    if (monitors == NULL || !monitor_number_valid(monitors, k)) 
        return NULL;

    Monitor* curr = monitors->head;
    int ell = 0;
    while (curr != NULL && ell != k) {
        ++ell;
        curr = curr->next;
    }

    return curr;
}

void remove_monitor_k(Monitors* monitors, int k) {
    Monitor* monitor_k = get_monitor_k(monitors, k);
    if (monitor_k != NULL)
        remove_monitor(monitors, monitor_k);
}

int number_of_monitors(const Monitors* monitors) {
    if (monitors == NULL) return 0;

    return monitors->n;
}

void destroy_monitors(Monitors* monitors) {
    if (monitors == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        Monitor* tmp = curr;
        curr = curr->next;
        destroy_monitor(tmp);
        free(tmp);
    }

    monitors->head = NULL;
    monitors->n = 0;
}

Monitor* get_primary_monitor(const Monitors* monitors) {
    if (monitors == NULL) return NULL;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        if (is_monitor_primary(curr))
            return curr;
        curr = curr->next;
    }

    return NULL;
}

Monitor* get_first_monitor_that_is_not(const Monitors* monitors, const Monitor* is) {
    if (monitors == NULL) return NULL;

    Monitor* curr = monitors->head;
    while (curr != NULL && curr == is)
        curr = curr->next;

    return curr;
}

_Bool monitors_absorb_monitor(Monitors* monitors, Monitor* absorbee) {
    if (monitors == NULL || absorbee == NULL) return true;

    Monitor* absorber = get_primary_monitor(monitors);
    if (absorber == absorbee)
        absorber = get_monitor_from_pointer_position(monitors);

    if (absorber == absorbee)
        absorber = get_first_monitor_that_is_not(monitors, absorbee);

    if (absorber == NULL)
        return false;

    absorb_monitor(absorber, absorbee);

    return true;
}

Monitor* get_monitor_from_position(const Monitors* monitors, int x, int y) {
    if (monitors == NULL) return NULL;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        int mx = monitor_x(curr);
        int my = monitor_y(curr);
        int mw = monitor_width(curr);
        int mh = monitor_height(curr);

        int x_range = mx + mw;
        int y_range = my + mh;

        if ((x >= mx && x <= x_range) 
            && (y >= my && y <= y_range)
        ) return curr;

        curr = curr->next;
    }

    return NULL;
}

Monitor* get_monitor_from_pointer_position(const Monitors* monitors) {
    if (monitors == NULL) return NULL;
    Position pointer_pos = get_pointer_pos();
    return get_monitor_from_position(
        monitors,
        pos_x(&pointer_pos),
        pos_y(&pointer_pos)
    );
}

Monitor* get_monitor_from_client(const Monitors* monitors, const Client* client) {
    if (monitors == NULL || client == NULL) return NULL;
    return get_monitor_from_position(
        monitors,
        get_client_x(client),
        get_client_y(client)
    );
}

Atom get_monitor_name_from_position(const Monitors* monitors, int x, int y) {
    if (monitors == NULL) return None;

    Monitor* monitor = get_monitor_from_position(monitors, x, y);
    return monitor != NULL 
        ? get_monitor_name(monitor) 
        : None;
}

Monitor* get_monitor_from_list_membership(const Monitors* monitors, const Client* client) {
    if (monitors == NULL || client == NULL) return NULL;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        if (cl_find_client(get_monitor_cl(curr), client)
            || fl_find_from_client(get_monitor_fl(curr), client)
            || ml_find_from_client(get_monitor_ml(curr), client)
            || ul_find_from_client(get_monitor_ul(curr), client)
            || docks_find_from_client(get_monitor_dl(curr), client)
        ) return curr;

        curr = curr->next;
    }

    return NULL;
}

int get_min_geometry_of_monitors(MonitorGeometryInfoFn ginfo) {
    const Monitors* monitors = get_workspace_monitors(0);
    if (monitors == NULL) 
        return 0;

    Monitor* curr = monitors->head;
    if (curr == NULL) 
        return 0;

    int min_width = INT_MAX;
    while (curr != NULL) {
        min_width = min(
            min_width,
            ginfo(curr)
        );

        curr = curr->next;
    }

    return min_width;
}

int get_min_width_of_monitors() {
    return get_min_geometry_of_monitors(monitor_width);
}

int get_min_height_of_monitors() {
    return get_min_geometry_of_monitors(monitor_height);
}

Client* monitors_search_for_client(const Monitors* monitors, Window win, Monitor** monitor_return) {
    if (monitors == NULL) return NULL;

    Monitor* phony_mr = NULL;
    if (monitor_return == NULL) 
        monitor_return = &phony_mr;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        Client* cl_search = cl_find_client_from_win(
            get_monitor_cl(curr),
            win
        );

        if (cl_search != NULL) {
            *monitor_return = curr;
            return cl_search;
        }

        Client* fl_search = get_client_from_detached(
            fl_find_from_win(get_monitor_fl(curr), win)
        );

        if (fl_search != NULL) {
            *monitor_return = curr;
            return fl_search;
        }

        Client* ml_search = get_client_from_minimized(
            ml_find_from_win(get_monitor_ml(curr), win)
        );

        if (ml_search != NULL) {
            *monitor_return = curr;
            return ml_search;
        }

        Client* ul_search = get_client_from_unmapped(
            ul_find_from_win(get_monitor_ul(curr), win)
        );

        if (ul_search != NULL) {
            *monitor_return = curr;
            return ul_search;
        }

        Client* dl_search = get_client_from_dock(
            docks_find_from_win(get_monitor_dl(curr), win)
        );

        if (dl_search != NULL) {
            *monitor_return = curr;
            return dl_search;
        }

        curr = curr->next;
    }

    return NULL;
}

int count_viewable_clients_in_monitors(const Monitors* monitors) {
    if (monitors == NULL) return 0;

    Monitor* curr = monitors->head;
    int count = 0;
    while (curr != NULL) {
        count += cl_size(get_monitor_cl(curr))
            + ml_size(get_monitor_ml(curr))
            + fl_size(get_monitor_fl(curr));

        curr = curr->next;
    }
    return count;
}

void monitors_cancel_cl_pending_unmaps(const Monitors* monitors) {
    if (monitors == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        cl_cancel_pending_unmaps(get_monitor_cl(curr));
        curr = curr->next;
    }
}

void monitors_cancel_fl_pending_unmaps(const Monitors* monitors) {
    if (monitors == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        fl_cancel_pending_unmaps(get_monitor_fl(curr));
        curr = curr->next;
    }
}

void monitors_cancel_ml_pending_unmaps(const Monitors* monitors) {
    if (monitors == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        ml_cancel_pending_unmaps(get_monitor_ml(curr));
        curr = curr->next;
    }
}

void monitors_cancel_dl_pending_unmaps(const Monitors* monitors) {
    if (monitors == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        dl_cancel_pending_unmaps(get_monitor_dl(curr));
        curr = curr->next;
    }
}

void monitors_cancel_pending_unmaps(const Monitors* monitors) {
    if (monitors == NULL) return;

    monitors_cancel_cl_pending_unmaps(monitors);
    monitors_cancel_fl_pending_unmaps(monitors);
    monitors_cancel_ml_pending_unmaps(monitors);
    monitors_cancel_dl_pending_unmaps(monitors);
}

void monitors_cancel_cancel_cl_pending_unmaps(const Monitors* monitors) {
    if (monitors == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        cl_cancel_cancel_pending_unmaps(get_monitor_cl(curr));
        curr = curr->next;
    }
}

void monitors_cancel_cancel_fl_pending_unmaps(const Monitors* monitors) {
    if (monitors == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        fl_cancel_cancel_pending_unmaps(get_monitor_fl(curr));
        curr = curr->next;
    }
}

void monitors_cancel_cancel_ml_pending_unmaps(const Monitors* monitors) {
    if (monitors == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        ml_cancel_cancel_pending_unmaps(get_monitor_ml(curr));
        curr = curr->next;
    }
}

void monitors_cancel_cancel_dl_pending_unmaps(const Monitors* monitors) {
    if (monitors == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        dl_cancel_cancel_pending_unmaps(get_monitor_dl(curr));
        curr = curr->next;
    }
}

void monitors_cancel_cancel_pending_unmaps(const Monitors* monitors) {
    if (monitors == NULL) return;

    monitors_cancel_cancel_cl_pending_unmaps(monitors);
    monitors_cancel_cancel_fl_pending_unmaps(monitors);
    monitors_cancel_cancel_ml_pending_unmaps(monitors);
    monitors_cancel_cancel_dl_pending_unmaps(monitors);
}

void monitors_set_cl_mapped_or_unmapped_from_workspace_switch(const Monitors* monitors, SetMappedFromSwitchFn fn, _Bool flag) {
    if (monitors == NULL || fn == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        cl_set_mapped_or_unmapped_from_workspace_switch(
            get_monitor_cl(curr),
            fn,
            flag
        );
        curr = curr->next;
    }
}

void monitors_set_fl_mapped_or_unmapped_from_workspace_switch(const Monitors* monitors, SetMappedFromSwitchFn fn, _Bool flag) {
    if (monitors == NULL || fn == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        fl_set_mapped_or_unmapped_from_workspace_switch(
            get_monitor_fl(curr),
            fn,
            flag
        );
        curr = curr->next;
    }
}

void monitors_set_ml_mapped_or_unmapped_from_workspace_switch(const Monitors* monitors, SetMappedFromSwitchFn fn, _Bool flag) {
    if (monitors == NULL || fn == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        ml_set_mapped_or_unmapped_from_workspace_switch(
            get_monitor_ml(curr),
            fn,
            flag
        );
        curr = curr->next;
    }
}

void monitors_set_ul_mapped_or_unmapped_from_workspace_switch(const Monitors* monitors, SetMappedFromSwitchFn fn, _Bool flag) {
    if (monitors == NULL || fn == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        ul_set_mapped_or_unmapped_from_workspace_switch(
            get_monitor_ul(curr),
            fn,
            flag
        );
        curr = curr->next;
    }
}

void monitors_set_dl_mapped_or_unmapped_from_workspace_switch(const Monitors* monitors, SetMappedFromSwitchFn fn, _Bool flag) {
    if (monitors == NULL || fn == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        dl_set_mapped_or_unmapped_from_workspace_switch(
            get_monitor_dl(curr),
            fn,
            flag
        );
        curr = curr->next;
    }
}

void monitors_set_all_unmapped_from_workspace_switch(const Monitors* monitors, _Bool flag) {
    monitors_set_cl_mapped_or_unmapped_from_workspace_switch(
        monitors,
        client_set_unmapped_from_workspace_switch,
        flag
    );

    monitors_set_fl_mapped_or_unmapped_from_workspace_switch(
        monitors,
        client_set_unmapped_from_workspace_switch,
        flag
    );

    monitors_set_ml_mapped_or_unmapped_from_workspace_switch(
        monitors,
        client_set_unmapped_from_workspace_switch,
        flag
    );

    monitors_set_ul_mapped_or_unmapped_from_workspace_switch(
        monitors,
        client_set_unmapped_from_workspace_switch,
        flag
    );

    monitors_set_dl_mapped_or_unmapped_from_workspace_switch(
        monitors,
        client_set_unmapped_from_workspace_switch,
        flag
    );
}

void monitors_set_all_mapped_from_workspace_switch(const Monitors* monitors, _Bool flag) {
    if (monitors == NULL) return;

    monitors_set_cl_mapped_or_unmapped_from_workspace_switch(
        monitors,
        client_set_mapped_from_workspace_switch,
        flag
    );

    monitors_set_fl_mapped_or_unmapped_from_workspace_switch(
        monitors,
        client_set_mapped_from_workspace_switch,
        flag
    );

    monitors_set_ml_mapped_or_unmapped_from_workspace_switch(
        monitors,
        client_set_mapped_from_workspace_switch,
        flag
    );

    monitors_set_ul_mapped_or_unmapped_from_workspace_switch(
        monitors,
        client_set_mapped_from_workspace_switch,
        flag
    );

    monitors_set_dl_mapped_or_unmapped_from_workspace_switch(
        monitors,
        client_set_mapped_from_workspace_switch,
        flag
    );
}

void monitors_cancel_ul_pending_maps(const Monitors* monitors) {
    if (monitors == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        ul_cancel_pending_maps(get_monitor_ul(curr));
        curr = curr->next;
    }
}

void monitors_cancel_cancel_ul_pending_maps(const Monitors* monitors) {
    if (monitors == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        ul_cancel_cancel_pending_maps(get_monitor_ul(curr));
        curr = curr->next;
    }
}

void monitors_map_unmap_cl(const Monitors* monitors, MapClientFn map_fn, _Bool manage_cont) {
    if (monitors == NULL || map_fn == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        cl_map_unmap(get_monitor_cl(curr), map_fn, manage_cont);
        curr = curr->next;
    }
}

void monitors_map_unmap_fl(const Monitors* monitors, MapClientFn map_fn, _Bool manage_cont) {
    if (monitors == NULL || map_fn == NULL) return;
    
    Monitor* curr = monitors->head;
    while (curr != NULL) {
        fl_map_unmap(get_monitor_fl(curr), map_fn, manage_cont);
        curr = curr->next;
    }
}

void monitors_map_unmap_ml(const Monitors* monitors, MapClientFn map_fn, _Bool manage_cont) {
    if (monitors == NULL || map_fn == NULL) return;
    
    Monitor* curr = monitors->head;
    while (curr != NULL) {
        ml_map_unmap(get_monitor_ml(curr), map_fn, manage_cont);
        curr = curr->next;
    }
}

void monitors_map_unmap_ul(const Monitors* monitors, MapClientFn map_fn, _Bool manage_cont) {
    if (monitors == NULL || map_fn == NULL) return;
    
    Monitor* curr = monitors->head;
    while (curr != NULL) {
        ul_map_unmap(get_monitor_ul(curr), map_fn, manage_cont);
        curr = curr->next;
    }
}

void monitors_map_unmap_dl(const Monitors* monitors, MapClientFn map_fn, _Bool manage_cont) {
    if (monitors == NULL || map_fn == NULL) return;
    
    Monitor* curr = monitors->head;
    while (curr != NULL) {
        dl_map_unmap(get_monitor_dl(curr), map_fn, manage_cont);
        curr = curr->next;
    }
}

void monitors_cl_set_future_unmap_stay_unmapped(const Monitors* monitors) {
    if (monitors == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        cl_set_future_unmap_stay_unmapped(
            get_monitor_cl(curr)
        );

        curr = curr->next;
    }
}

void monitors_ul_set_all_can_be_mapped(const Monitors* monitors) {
    if (monitors == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        ul_set_all_can_be_mapped(
            get_monitor_ul(curr)
        );

        curr = curr->next;
    }
}

Client* monitors_focus_from_cl_head_or_tail(const Monitors* monitors, int workspace) {
    if (monitors == NULL) return NULL;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        ClientList* cl = get_monitor_cl(curr);
        Client* next = get_monitor_layout(curr, workspace) == ANGEL_MONOCLE
            ? cl_tail(cl)
            : cl_head(cl);

        if (next != NULL)
            return next;

        curr = curr->next;
    }

    return NULL;
}

Client* monitors_focus_from_fl(const Monitors* monitors) {
    if (monitors == NULL) return NULL;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        Client* next = get_client_from_detached(
            fl_head(get_monitor_fl(curr))
        );

        if (next != NULL)
            return next;

        curr = curr->next;
    }

    return NULL;
}

Client* monitors_focus_from_ml(const Monitors* monitors) {
    if (monitors == NULL) return NULL;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        Client* next = get_client_from_minimized(
            ml_head(get_monitor_ml(curr))
        );

        if (next != NULL)
            return next;

        curr = curr->next;
    }

    return NULL;
}

int monitors_get_left_position_extremes(
    const Monitors* monitors,
    MinOrMaxFn compare,
    MonitorPositionInfoFn position_info
) {
    if (monitors == NULL 
        || monitors->head == NULL 
        || compare == NULL 
        || position_info == NULL
    ) return 0;

    int extreme = position_info(monitors->head);
    Monitor* curr = monitors->head->next;
    while (curr != NULL) {
        extreme = compare(extreme, position_info(curr));
        curr = curr->next;
    }

    return extreme;
}

int monitors_get_smallest_x(const Monitors* monitors) {
    if (monitors == NULL) return 0;
    return monitors_get_left_position_extremes(
        monitors,
        min,
        monitor_x
    );
}

int monitors_get_smallest_y(const Monitors* monitors) {
    if (monitors == NULL) return 0;
    return monitors_get_left_position_extremes(
        monitors,
        min,
        monitor_y
    );
}

int monitors_get_right_position_extremes(
    const Monitors* monitors,
    MonitorPositionInfoFn position_info,
    MonitorGeometryInfoFn geometry_info
) {
    if (monitors == NULL 
        || position_info == NULL 
        || geometry_info == NULL 
        || monitors->head == NULL
    ) return 0;

    int largest = position_info(monitors->head) 
        + geometry_info(monitors->head);

    Monitor* curr = monitors->head->next;
    while (curr != NULL) {
        largest = max(
            largest,
            position_info(curr) 
            + geometry_info(curr)
        );

        curr = curr->next;
    }

    return largest;
}

int monitors_get_largest_x(const Monitors* monitors) {
    if (monitors == NULL) return 0;
    return monitors_get_right_position_extremes(
        monitors,
        monitor_x,
        monitor_width
    );
}

int monitors_get_largest_y(const Monitors* monitors) {
    if (monitors == NULL) return 0;
    return monitors_get_right_position_extremes(
        monitors,
        monitor_y,
        monitor_height
    );
}

Monitor* get_adjacent_monitor(
    const Monitors* monitors,
    const Monitor* monitor,
    int curr_xy_start, 
    IntCompareFn cxy_mxy_compare,
    IntCompareFn cxy_currxy_compare,
    MonitorGeometryInfoFn main_concern,
    MonitorGeometryInfoFn sub_concern
) {
    if (monitors == NULL || monitor == NULL) return NULL;

    int curr_xy = curr_xy_start;
    int curr_xy_difference = INT_MAX;

    int monitor_mc = main_concern(monitor);
    int monitor_sc = sub_concern(monitor);

    Monitor* ret = NULL;
    Monitor* curr = monitors->head;

    while (curr != NULL) {
        if (curr == monitor) {
            curr = curr->next;
            continue;
        }

        int curr_mc = main_concern(curr);
        int curr_sc = sub_concern(curr);

        if (curr_mc == curr_xy) {
            int xy_diff = abs(monitor_sc - curr_sc);
            if (xy_diff < curr_xy_difference) {
                curr_xy = curr_mc;
                curr_xy_difference = xy_diff;
                ret = curr;
            }
        }

        else if (cxy_mxy_compare(curr_mc, monitor_mc) 
            && cxy_currxy_compare(curr_mc, curr_xy)
        ) {
            curr_xy = curr_mc;
            curr_xy_difference = abs(monitor_sc - curr_sc);
            ret = curr;
        }

        curr = curr->next;
    }

    return ret;
}

Monitor* get_monitor_to_right(const Monitors* monitors, const Monitor* monitor) {
    if (monitors == NULL || monitor == NULL) return NULL;
    return get_adjacent_monitor(
        monitors,
        monitor,
        INT_MAX,
        greater,
        less,
        monitor_x,
        monitor_y
    );

}

Monitor* get_monitor_to_left(const Monitors* monitors, const Monitor* monitor) {
    if (monitors == NULL || monitor == NULL) return NULL;
    return get_adjacent_monitor(
        monitors,
        monitor,
        INT_MIN,
        less,
        greater,
        monitor_x,
        monitor_y
    );
}

Monitor* get_monitor_above(const Monitors* monitors, const Monitor* monitor) {
    if (monitors == NULL || monitor == NULL) return NULL;
    return get_adjacent_monitor(
        monitors,
        monitor,
        INT_MIN,
        less,
        greater,
        monitor_y,
        monitor_x
    );
}

Monitor* get_monitor_below(const Monitors* monitors, const Monitor* monitor) {
    if (monitors == NULL || monitor == NULL) return NULL;
    return get_adjacent_monitor(
        monitors,
        monitor,
        INT_MAX,
        greater,
        less,
        monitor_y,
        monitor_x
    );
}

Monitor* reconcile_client_starting_monitor(const Monitors* monitors, const Client* client) {
    if (client == NULL) return NULL;

    Atom name = get_monitor_name_property(
        get_client_win(client)
    );

    Monitor* fall_back = NULL;
    Client* focus = get_current_focus();

    if (focus_mode_pointer() || focus == NULL)
        fall_back = get_monitor_from_pointer_position(monitors);

    else if (focus_mode_focus())
        fall_back = reconcile_monitor_from_focus(
            get_client_on_workspace(focus),
            focus
        );

    Monitor* from_name = get_monitor_with_name(monitors, name);
    return from_name != NULL 
        ? from_name 
        : fall_back;
}

Atom find_removed_monitor(Monitors* monitors, XRRMonitorInfo* info, int n) {
    if (monitors == NULL || info == NULL) return None;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        _Bool found = false;
        for (int i = 0; i < n; ++i) {
            if (get_monitor_name(curr) == info[i].name) {
                found = true;
                break;
            }
        }
        
        if (!found)
            return get_monitor_name(curr);

        curr = curr->next;
    }

    return None;
}

Monitor* find_new_monitor(Monitors* monitors, XRRMonitorInfo* info, int n) {
    if (monitors == NULL || info == NULL) return NULL; 

    for (int i = 0; i < n; ++i) {
        Monitor* curr = monitors->head;
        _Bool found = false;
        while (curr != NULL) {
            if (get_monitor_name(curr) == info[i].name) {
                found = true;
                break;
            } 

            curr = curr->next;
        }

        if (!found)
            return create_monitor(
                info[i].name,
                info[i].primary,
                info[i].x, 
                info[i].y,
                info[i].width,
                info[i].height
            );
    }

    return NULL;
}

void monitors_update_internal_data(Monitors* monitors, XRRMonitorInfo* info, int n) {
    if (monitors == NULL || info == NULL) return;

    for (int i = 0; i < n; ++i) {
        Monitor* monitor = get_monitor_with_name(monitors, info[i].name);
        if (monitor == NULL) continue;

        update_monitor_x(monitor, info[i].x);
        update_monitor_y(monitor, info[i].x);
        update_monitor_width(monitor, info[i].width);
        update_monitor_height(monitor, info[i].height);

        update_monitor_start_x(monitor, info[i].x);
        update_monitor_start_y(monitor, info[i].x);

        update_monitor_usable_width(monitor, info[i].width);
        update_monitor_usable_height(monitor, info[i].height);

        set_monitor_primary(monitor, info[i].primary);
    }
}

void adjust_monitors_for_strut(Monitors* monitors, StrutSide strut_side, int x, int y, int w, int h) {
    if (monitors == NULL) return;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        update_monitor_on_strut_intersect(
            curr,
            strut_side,
            x, y,
            w, h
        );

        curr = curr->next;
    }
}

Monitor* move_client_to_monitor_with_position(
    Monitors* monitors,
    Monitor* client_monitor,
    Client* client
) {
    if (monitors == NULL || client == NULL) return NULL;

    int cx = get_client_x(client);
    int cy = get_client_y(client);
    int cw = get_client_width(client);
    int ch = get_client_height(client);

    int max_area = INT_MIN;
    Monitor* best_monitor = NULL;

    Monitor* curr = monitors->head;
    while (curr != NULL) {
        int mx = monitor_x(curr);
        int my = monitor_y(curr);
        int mw = monitor_width(curr);
        int mh = monitor_height(curr);

        int xmax_l = max(mx, cx);
        int xmin_r = min(mx + mw, cx + cw);

        int ymax_t = max(my, cy);
        int ymin_b = min(my + mh, cy + ch);

        if (xmax_l > xmin_r || ymax_t > ymin_b) {
            curr = curr->next;
            continue;
        }

        int iw = xmin_r - xmax_l;
        int ih = ymin_b - ymax_t;

        int area = max(0, iw * ih);
        if (area > max_area) {
            best_monitor = curr;
            max_area = area;
        }

        curr = curr->next;
    }

    if (best_monitor != NULL)
        move_client_to_monitor(
            client,
            client_monitor,
            best_monitor
        );

    return best_monitor != NULL 
        ? best_monitor 
        : client_monitor;
}

void move_persistent_docks_to_workspace(int from, int to) {
    if (!workspace_is_valid(from)
        || !workspace_is_valid(to)
    ) return;

    Monitors* from_monitors = get_workspace_monitors(from);
    Monitors* to_monitors = get_workspace_monitors(to);

    Monitor* from_curr = from_monitors->head;
    while (from_curr != NULL) {
        Docks* docks = get_monitor_dl(from_curr);
        if (docks_empty(docks)) {
            from_curr = from_curr->next;
            continue;
        }

        Monitor* symmetric = get_symmetric_monitor(from_curr, to);
        move_docks_to_monitor(symmetric, docks, to);

        from_curr = from_curr->next;
    }
}
