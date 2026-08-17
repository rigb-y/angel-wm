/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_WINDOWS_H
#define ANGEL_WINDOWS_H

#include <X11/Xlib.h>

#define TOP_LEVEL_EVENT_MASK StructureNotifyMask \
                             | EnterWindowMask \
                             | LeaveWindowMask \
                             | PointerMotionMask \
                             | PropertyChangeMask \
                             | FocusChangeMask \

typedef struct Color Color;

XWindowAttributes* get_win_attrs(Window);
char* get_window_name(Window);
void give_window_focus(Window, Time);
Window get_window_focus();

void set_window_border_width(Window, unsigned int);
void set_window_border_color(Window, Color);

void set_root_background_solid(char*);
void set_root_cursor(Cursor);

void map_window(Window);
void unmap_window(Window);
void kill_client(Window);

void start_event_watch(Window);
void grab_left_click(Window);
void ungrab_left_click(Window);

void enable_drag(Window);
void disable_drag(Window);

void raise_window(Window);
void lower_window(Window);

void enable_mouse_resize(Window);
void disable_mouse_resize(Window);

#endif
