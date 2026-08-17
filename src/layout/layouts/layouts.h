/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_LAYOUTS_H
#define ANGEL_LAYOUTS_H

#include "types.h"

typedef enum Layout {
    ANGEL_SIMPLE_VERTICAL,
    ANGEL_SIMPLE_HORIZONTAL,
    ANGEL_MASTER_LEFT,
    ANGEL_MASTER_RIGHT,
    ANGEL_MONOCLE,
    ANGEL_MASTER_LEFT_MONOCLE,
    ANGEL_MASTER_RIGHT_MONOCLE,
    ANGEL_MASTER_MASTER_LEFT,
    ANGEL_MASTER_MASTER_RIGHT,
    ANGEL_LAYOUT_UNKNOWN
} Layout;

extern Layout default_workspace_layout; 

_Bool is_layout_accepted(Layout);
LayoutFn get_layout_fn(Layout);

#endif
