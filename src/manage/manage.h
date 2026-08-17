/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_MANAGE_H
#define ANGEL_MANAGE_H

#include "types.h"

#include <X11/Xlib.h>

typedef struct Client Client;
typedef struct Monitor Monitor;

void start_manage(Window, XWindowAttributes*, Monitor*);
_Bool is_managed(Window);
Client* get_managed(Window);
_Bool end_manage(Window);

FocusStart focus_start_adjacent();
FocusStart focus_start_end();
_Bool set_focus_start(FocusStart);

FocusEnd focus_end_focus_stack();
FocusEnd focus_end_next();
_Bool set_focus_end(FocusEnd);

#endif
