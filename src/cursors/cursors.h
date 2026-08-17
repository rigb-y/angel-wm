/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_CURSORS_H
#define ANGEL_CURSORS_H

#include <X11/Xlib.h>

typedef struct Cursors Cursors;
extern Cursors cursors;

typedef struct Cursors {
    Cursor standard_pointer;
    Cursor text_insertion;
    Cursor busy;
    Cursor hand_shape; 
    Cursor crosshair;
    Cursor four_df_resize;
    Cursor horizontal_resize;
    Cursor vertical_resize;
    Cursor top_resize;
    Cursor bottom_resize;
    Cursor left_resize;
    Cursor right_resize;
    Cursor top_left_resize;
    Cursor top_right_resize;
    Cursor bottom_left_resize;
    Cursor bottom_right_resize;
} Cursors;

void set_cursors();

#endif
