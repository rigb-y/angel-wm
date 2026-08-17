/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "setup.h"
#include "windows.h"
#include "manage.h"
#include "workspaces.h"
#include "monitors.h"
#include "ewmh.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <stdlib.h>

/**
 * @brief Establishes connection to X Server on display
 *
 * @param display the name of the display
 * @return Display* pointer to display or NULL
 *
 * @note If display is NULL, XOpenDisplay uses the DISPLAY
 * environment variable
 */
Display* initialize(char* display) {
    return XOpenDisplay(display);
}

/**
 * @brief Gets the root window
 *
 * @param dp pointer to display
 * @return Window the root window
 */
Window get_root_window() {
    return XDefaultRootWindow(dp);
}

/**
 * @brief Sets event mask for a window
 *
 * @param dp the display pointer
 * @param w the window of interest
 * @param mask the event mask
 * @return void
 */
void select_events_for_window(Window w, long mask) {
    XSelectInput(dp, w, mask);
}

void select_rr_events_for_window(Window w, int mask) {
    XRRSelectInput(dp, w, mask);
}

void manage_existing(Window* children, unsigned int n_children) {
    for (unsigned int i = 0; i < n_children; ++i) {
        XWindowAttributes* attrs = get_win_attrs(children[i]);
        if (attrs == NULL 
            || attrs->class == InputOnly
            || attrs->override_redirect
            || get_managed(children[i]) != NULL
            || (attrs->map_state != IsViewable 
                && !workspace_is_valid(ewmh_get_workspace_num(children[i]))
                )
        ) {
            XFree(attrs);
            continue;
        }

        // attrs is now owned by the client
        start_manage(children[i], attrs, get_primary_monitor(
            get_workspace_monitors(
                get_current_workspace())
            )
        );
    }

    XFree(children);
}
