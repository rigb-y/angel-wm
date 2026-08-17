/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "cursors.h"

#include <X11/cursorfont.h>

Cursors cursors = {0};

void set_cursors() {
    cursors.standard_pointer = XCreateFontCursor(dp, XC_left_ptr);
    cursors.text_insertion = XCreateFontCursor(dp, XC_xterm);
    cursors.busy = XCreateFontCursor(dp, XC_watch);
    cursors.hand_shape  = XCreateFontCursor(dp, XC_hand1);
    cursors.crosshair = XCreateFontCursor(dp, XC_crosshair);
    cursors.four_df_resize = XCreateFontCursor(dp, XC_fleur);
    cursors.horizontal_resize = XCreateFontCursor(dp, XC_sb_h_double_arrow);
    cursors.vertical_resize = XCreateFontCursor(dp, XC_sb_v_double_arrow);
    cursors.top_resize = XCreateFontCursor(dp, XC_top_side);
    cursors.bottom_resize = XCreateFontCursor(dp, XC_bottom_side);
    cursors.left_resize = XCreateFontCursor(dp, XC_left_side);
    cursors.right_resize = XCreateFontCursor(dp, XC_right_side);
    cursors.top_left_resize = XCreateFontCursor(dp, XC_top_left_corner);
    cursors.top_right_resize = XCreateFontCursor(dp, XC_top_right_corner);
    cursors.bottom_left_resize = XCreateFontCursor(dp, XC_bottom_left_corner);
    cursors.bottom_right_resize = XCreateFontCursor(dp, XC_bottom_right_corner);
}
