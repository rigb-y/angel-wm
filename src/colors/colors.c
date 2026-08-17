/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "colors.h"
#include "screen.h"

void set_color(Color* color, XColor xcolor, Status st) {
    color->xcolor = xcolor;
    color->bad = (st == 0);
}

Colormap get_default_color_map(int screen_number) {
    return XDefaultColormap(dp, screen_number);
}

Color get_color(const char* spec) {
    Colormap default_color_map = get_default_color_map(get_default_screen_number());

    XColor xcolor;
    Status xpc_s = XParseColor(dp, default_color_map, spec, &xcolor);
    Status xac_s = XAllocColor(dp, default_color_map, &xcolor);

    Color color = {0};
    set_color(&color, xcolor, xpc_s + xac_s);

    return color;
}
