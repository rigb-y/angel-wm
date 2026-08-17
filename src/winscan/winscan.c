/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "winscan.h"

Status scan_existing_windows(
    Window root,
    Window* root_return,
    Window* parent_return,
    Window** children_return,
    unsigned int* nchildren_return
) {
    return XQueryTree(
        dp,
        root,
        root_return,
        parent_return,
        children_return,
        nchildren_return
    );
}
