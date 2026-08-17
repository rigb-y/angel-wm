/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "geometry.h"
#include "pointer.h"

#include <X11/Xlib.h>

PointerPosition pointer_pos = {0};

void pp_set_x(int x) {
    pointer_pos.x = x;
}

void pp_set_y(int y) {
    pointer_pos.y = y;
}

void pp_set_xy(int x, int y) {
    pp_set_x(x);
    pp_set_y(y);
}

int pp_get_x() {
    return pointer_pos.x;
}

int pp_get_y() {
    return pointer_pos.y;
}

_Bool pp_has_moved(const Position* pos) {
    return pointer_pos.x != pos_x(pos) 
        || pointer_pos.y != pos_y(pos);
}

Position get_pointer_pos() {
    int x_root = 0, y_root = 0;
    int x_child = 0, y_child = 0;
    unsigned int mask_return = 0;
    Window root_ret, child_ret;

    XQueryPointer(
        dp,
        root,
        &root_ret, &child_ret,
        &x_root, &y_root,
        &x_child, &y_child,
        &mask_return
    );

    return create_position(x_root, y_root);
}
