/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "cleanup.h"
#include "workspaces.h"
#include "keymaps.h"
#include "wspipe.h"
#include "keyboard.h"

#include <X11/Xlib.h>

void cleanup() {
    teardown_workspaces();
    destroy_keymaps();
    ungrab_all_root_passive_grabs();
    destroy_pipe();
    XCloseDisplay(dp);
}
