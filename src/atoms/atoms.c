/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "atoms.h"
#include "angel.h"

#include <X11/Xlib.h>
#include <stdbool.h>

Atom wm_protocols;
Atom wm_delete_window;
Atom wm_take_focus;
Atom wm_state_atom;
Atom net_wm_desktop;
Atom monitor_name_property;
Atom net_wm_window_type;
Atom net_wm_window_type_dock;
Atom net_wm_strut;
Atom net_wm_strut_partial;
Atom net_number_of_desktops;
Atom net_current_desktop;
Atom net_active_window;

void set_atoms() {
    wm_protocols = XInternAtom(dp, "WM_PROTOCOLS", false);
    wm_delete_window = XInternAtom(dp, "WM_DELETE_WINDOW", false);
    wm_take_focus = XInternAtom(dp, "WM_TAKE_FOCUS", false);
    wm_state_atom = XInternAtom(dp, "WM_STATE", false);
    net_wm_desktop = XInternAtom(dp, "_NET_WM_DESKTOP", false);
    monitor_name_property = XInternAtom(dp, "_WM_MONITOR_NAME", false);
    net_wm_window_type = XInternAtom(dp, "_NET_WM_WINDOW_TYPE", false);
    net_wm_window_type_dock = XInternAtom(dp, "_NET_WM_WINDOW_TYPE_DOCK", false);
    net_wm_strut = XInternAtom(dp, "_NET_WM_STRUT", false);
    net_wm_strut_partial = XInternAtom(dp, "_NET_WM_STRUT_PARTIAL", false);
    net_number_of_desktops = XInternAtom(dp, "_NET_NUMBER_OF_DESKTOPS", false);
    net_current_desktop = XInternAtom(dp, "_NET_CURRENT_DESKTOP", false);
    net_active_window = XInternAtom(dp, "_NET_ACTIVE_WINDOW", false);
}
