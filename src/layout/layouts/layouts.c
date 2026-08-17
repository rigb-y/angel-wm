/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "layouts.h"
#include "geometry.h"
#include "types.h"
#include "utils.h"

#include <stdbool.h>

Layout default_workspace_layout = ANGEL_MASTER_LEFT; 

static const Layout accepted_layouts[] = {
    ANGEL_SIMPLE_VERTICAL,
    ANGEL_SIMPLE_HORIZONTAL,
    ANGEL_MASTER_LEFT,
    ANGEL_MASTER_RIGHT,
    ANGEL_MONOCLE,
    ANGEL_MASTER_LEFT_MONOCLE,
    ANGEL_MASTER_RIGHT_MONOCLE,
    ANGEL_MASTER_MASTER_LEFT,
    ANGEL_MASTER_MASTER_RIGHT,
};

_Bool is_layout_accepted(Layout layout) {
    for (int I = 0; I < ARRAY_SIZE(accepted_layouts); ++I)
        if (layout == accepted_layouts[I])
            return true;
    return false;
}

typedef struct LayoutLayoutFnPair {
    Layout layout;
    LayoutFn fn;
} LayoutLayoutFnPair;

static const LayoutLayoutFnPair layout_to_layout_fn[] = {
    {ANGEL_SIMPLE_VERTICAL, simple_tile_vertical},
    {ANGEL_SIMPLE_HORIZONTAL, simple_tile_horizontal},
    {ANGEL_MASTER_LEFT, tile_master_left},
    {ANGEL_MASTER_RIGHT, tile_master_right},
    {ANGEL_MONOCLE, tile_monocle},
    {ANGEL_MASTER_LEFT_MONOCLE, tile_master_left_monocle},
    {ANGEL_MASTER_RIGHT_MONOCLE, tile_master_right_monocle},
    {ANGEL_MASTER_MASTER_LEFT, tile_master_master_left},
    {ANGEL_MASTER_MASTER_RIGHT, tile_master_master_right},
    {ANGEL_LAYOUT_UNKNOWN, tile_noop}
};

LayoutFn get_layout_fn(Layout layout) {
    for (int I = 0; I < ARRAY_SIZE(layout_to_layout_fn); ++I)
        if (layout_to_layout_fn[I].layout == layout)
            return layout_to_layout_fn[I].fn;
    return tile_noop;
}
