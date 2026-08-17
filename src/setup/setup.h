/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_SETUP_H
#define ANGEL_SETUP_H

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#define ANGEL_DEFAULT_DISPLAY NULL
#define DEFAULT_ERROR_STR_LEN 1024
#define ROOT_EVENT_MASK SubstructureRedirectMask  \
                        | SubstructureNotifyMask

#define ROOT_RR_EVENT_MASK RRScreenChangeNotifyMask \
                            | RRCrtcChangeNotifyMask \
                            | RROutputChangeNotifyMask \
                            | RRResourceChangeNotifyMask

Display* initialize(char*);
Window get_root_window();
void select_events_for_window(Window, long);
void select_rr_events_for_window(Window, int);

void manage_existing(Window*, unsigned int);

#endif
