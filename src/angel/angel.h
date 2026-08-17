/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_ANGEL_H
#define ANGEL_ANGEL_H

#include <X11/Xlib.h>

extern _Bool IN_RESIZE;

typedef enum WM_STATE {
    WM_RUNNING,
    WM_RESTART,
    WM_QUIT
} WM_STATE;

typedef struct Client Client;

extern Display* dp;
extern Window root;

int angel_manage_windows(int, char**);

#endif
