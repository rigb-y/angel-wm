/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_COLORS_H
#define ANGEL_COLORS_H

#include <X11/Xlib.h>

typedef struct Color {
    XColor xcolor; 
    _Bool bad;
} Color;

void set_color(Color*, XColor, Status);

Colormap get_default_color_map(int);
Color get_color(const char*);

#endif
