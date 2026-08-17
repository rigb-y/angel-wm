/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_EWMH_H
#define ANGEL_EWMH_H

#include <X11/Xlib.h>

typedef struct Strut Strut;

void ewmh_store_workspace_num(Window, unsigned long);
int ewmh_get_workspace_num(Window);
void ewmh_change_property(
    Window,
    unsigned long,
    Atom,
    Atom,
    int,
    int
);
_Bool ewmh_read_property(
    Window,
    Atom,
    Atom,
    unsigned long**,
    int,
    long,
    long,
    unsigned long
);
void store_monitor_name_property(Window, Atom);
Atom get_monitor_name_property(Window);
Atom ewmh_get_window_type(Window);
Strut ewmh_get_dock_strut(Window);
_Bool ewmh_window_is_dock(Window);

void ewmh_advertise_number_of_desktops(); 
int ewmh_get_current_desktop();
void ewmh_set_current_desktop(int);
void ewmh_set_active_window(Window);

#endif
