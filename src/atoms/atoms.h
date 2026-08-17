/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_ATOMS_H
#define ANGEL_ATOMS_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>

extern Atom wm_protocols;
extern Atom wm_delete_window;
extern Atom wm_take_focus;
extern Atom wm_state_atom;
extern Atom net_wm_desktop;
extern Atom monitor_name_property;
extern Atom net_wm_window_type;
extern Atom net_wm_window_type_dock;
extern Atom net_wm_strut;
extern Atom net_wm_strut_partial;
extern Atom net_number_of_desktops;
extern Atom net_current_desktop;
extern Atom net_active_window;

void set_atoms();

#endif
