/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "windows.h"
#include "colors.h"
#include "logging.h"
#include "cursors.h"
#include "keyboard.h"
#include "client.h"
#include "manage.h"
#include "workspaces.h"

#include <X11/Xlib.h>
#include <stdlib.h>

static _Bool is_mapped_and_override_redirect(Window);
static Window bottom_most_overlay();
static Window get_top_most_managed();

XWindowAttributes* get_win_attrs(Window w) {
    if (w == None) return NULL;

    XWindowAttributes* attrs;
    if ((attrs = calloc(1, sizeof(XWindowAttributes))) == NULL)
        return NULL;

    XGetWindowAttributes(dp, w, attrs);

    return attrs;
}

char* get_window_name(Window window) {
    if (window == None) return NULL;

    char* name;
    XFetchName(dp, window, &name);
    return name;
}

void give_window_focus(Window win, Time time) {
    XSetInputFocus(dp, win, RevertToParent, time);
}

Window get_window_focus() {
    Window win_return = None;
    int revert_to_return = RevertToNone;
    XGetInputFocus(dp, &win_return, &revert_to_return);

    return win_return;
}

void set_window_border_width(Window win, unsigned int width) {
    if (win == None || width < 0) return;

    XSetWindowBorderWidth(dp, win, width);
}

void set_root_background_solid(char* spec) {
    Color color = get_color(spec);
    if (color.bad) {
        lerr("Bad root fill color");
        return;
    }
    XSetWindowBackground(dp, root, color.xcolor.pixel);
    XClearWindow(dp, root);
    XFlush(dp);
}

void set_window_border_color(Window win, Color color) {
    if (win == None) return;

    if (color.bad) {
        lerr("Bad window border color");
        return;
    }
    XSetWindowBorder(dp, win, color.xcolor.pixel);
}

void map_window(Window win) {
    if (win == None) return;

    XMapWindow(dp, win);
}

void unmap_window(Window win) {
    if (win == None) return;

    XUnmapWindow(dp, win);
}

void kill_client(Window win) {
    if (win == None) return;

    XKillClient(dp, win);
}

void start_event_watch(Window win) {
    if (win == None) return;

    XSelectInput(dp, win, TOP_LEVEL_EVENT_MASK);
}

void grab_left_click(Window win) {
    if (win == None) return;

    XGrabButton(
        dp,
        Button1,
        AnyModifier,
        win,
        false,
        ButtonPressMask,
        GrabModeSync,
        GrabModeAsync,
        None,
        None
    );
}

void ungrab_left_click(Window win) {
    if (win == None) return;

    XUngrabButton(
        dp, 
        Button1,
        AnyModifier,
        win
    );
}

void enable_drag(Window win) {
    if (win == None) return;

    XGrabButton(
        dp, 
        Button1,
        get_modifier(),
        win, 
        false,
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
        GrabModeAsync,
        GrabModeAsync, 
        None,
        cursors.four_df_resize
    );
}

void enable_mouse_resize(Window win) {
    if (win == None) return;

    XGrabButton(
        dp, 
        Button3,
        get_modifier(),
        win, 
        false,
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
        GrabModeAsync,
        GrabModeAsync, 
        None,
        cursors.horizontal_resize
    );
}

void disable_mouse_resize(Window win) {
    if (win == None) return;

    XUngrabButton(
        dp, 
        Button3,
        get_modifier(),
        win
    );
}

void disable_drag(Window win) {
    if (win == None) return;

    XUngrabButton(
        dp, 
        Button1,
        get_modifier(),
        win
    );
}

void set_root_cursor(Cursor cursor) {
    if (cursor == None) return;

    XDefineCursor(dp, root, cursor);
}

void raise_window(Window win) {
    if (win == None) return;

    Window under = bottom_most_overlay();
    if (under == None) {
        XRaiseWindow(dp, win);
        return;
    }

    XWindowChanges changes = {
        .sibling = under,
        .stack_mode = Below
    };

    XConfigureWindow(
        dp,
        win,
        CWStackMode | CWSibling,
        &changes
    );
}

void lower_window(Window win) {
    if (win == None) return;

    XLowerWindow(dp, win);
}

_Bool is_mapped_and_override_redirect(Window win) {
    if (win == None) return false;

    XWindowAttributes attrs;
    if (!XGetWindowAttributes(dp, win, &attrs))
        return false;

    return attrs.map_state == IsViewable 
        && attrs.override_redirect == true;
}

Window bottom_most_overlay() {
    Window root_return;
    Window parent_return;
    Window* children;
    unsigned int n_children;

    // Returns windows from bottom to top
    if (!XQueryTree(
            dp,
            root,
            &root_return,
            &parent_return,
            &children,
            &n_children
        )
    ) return None;

    Window bmo = None;
    unsigned int bmo_i = 0;
    for (unsigned int i = 0; i < n_children; ++i) {
        if (is_mapped_and_override_redirect(children[i])) {
            XFree(children);
            bmo = children[i];
            bmo_i = i;
            break;
        }
    }

    if (bmo == None) {
        XFree(children);
        return None;
    }

    // Move any managed that are somehow above
    // the lowest overlay below the overlay
    for (unsigned int i = bmo_i+1; i < n_children; ++i) {
        if (!is_managed(children[i]))
            continue;

        XWindowChanges changes = {
            .stack_mode = Below,
            .sibling = bmo
        };

        XConfigureWindow(
            dp,
            children[i],
            CWStackMode | CWSibling,
            &changes
        );
    }

    XFree(children);
    return bmo;
}

Window get_top_most_managed() {
    Window root_return;
    Window parent_return;
    Window* children;
    unsigned int n_children;

    // Returns windows from bottom to top
    if (!XQueryTree(
            dp,
            root,
            &root_return,
            &parent_return,
            &children,
            &n_children
        )
    ) return None;

    Window top_most = None;
    for (unsigned int i = 0; i < n_children; ++i)
        if (is_managed(children[i]) 
            && get_client_on_workspace(
                get_managed(children[i])
            ) == get_current_workspace()
        ){
            top_most = children[i];
        } 

    XFree(children);
    return top_most;
}
