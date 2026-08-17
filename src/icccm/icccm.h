/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_ICCCM_H
#define ANGEL_ICCCM_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

typedef struct Point {
    int x, y;
} Point;

_Bool is_state_valid(int);

XClassHint* get_class_hints(Window); 
const char* get_window_class_name(Window);

XSizeHints* get_size_hints(Window, long*);
int get_min_width_hint(Window);
int get_max_width_hint(Window);
int get_min_height_hint(Window);
int get_max_height_hint(Window);
int get_width_inc_hint(Window);
int get_height_inc_hint(Window);
int get_base_width_hint(Window);
int get_base_height_hint(Window);
int get_width_hint(Window);
int get_height_hint(Window);
Point get_min_aspect_hint(Window);
Point get_max_aspect_hint(Window);

XWMHints* get_wm_hints(Window);
_Bool get_input_hint(Window);
_Bool get_urgency_hint(Window);
XID get_window_group(Window);

void free_class_hints(XClassHint*);

_Bool window_supports_protocl(Window, Atom);
_Bool window_supports_delete(Window);
_Bool window_supports_take_focus(Window);

void send_client_message(Window, Atom, Time);
void send_delete(Window, Time);
void send_take_focus(Window, Time);

void send_synthetic_configure(Window, int, int, int, int, int);
void send_event(Window, XEvent*);
void set_wm_state(Window, long);
int get_wm_state(Window);
_Bool win_is_transient(Window, Window*);

#endif
