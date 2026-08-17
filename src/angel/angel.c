/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "logging.h"
#include "setup.h"
#include "winscan.h"
#include "types.h"
#include "cleanup.h"
#include "error.h"
#include "geometry.h"
#include "events.h"
#include "keyboard.h"
#include "workspaces.h"
#include "cursors.h"
#include "manage.h"
#include "client_list.h"
#include "pointer.h"
#include "utils.h"
#include "defaults.h"
#include "focus_stack.h"
#include "config_parser.h"
#include "atoms.h"
#include "randr.h"
#include "ewmh.h"
#include "wspipe.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

Display* dp = NULL;
Window root = 0;

_Bool IN_RESIZE = false;

static int set_display();
static int set_error_handler_and_claim_root();
static int prologue();
static void epilogue();

int angel_manage_windows(int argc, char** argv) {
    if (prologue() == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    evloop();

    if (wm_restart()) {
        XCloseDisplay(dp);
        execvp(argv[0], argv);

        perror("execvp");
        return EXIT_FAILURE;
    }

    epilogue();

    return EXIT_SUCCESS;
}

int set_display() {
    dp = initialize(ANGEL_DEFAULT_DISPLAY);

    if (dp == NULL) {
        lerr("Failed to get default display");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int set_error_handler_and_claim_root() {
    set_error_handler(error_handler);

    root = get_root_window();
    select_events_for_window(root, ROOT_EVENT_MASK);
    select_rr_events_for_window(root, ROOT_RR_EVENT_MASK);

    XSync(dp, false);

    if (wm_present) {
        lerr("Window Manager already present\n");     
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int prologue() {
    set_io_error_handler(io_error_handler);

    if (set_display() == EXIT_FAILURE)
        return EXIT_FAILURE;

    if (!ensure_randr_exists_and_get_bases(
        &randr_bases.event_base,
        &randr_bases.error_base
    )) {
        XCloseDisplay(dp);
        lerr("Randr extension unavailable");
        return EXIT_FAILURE;
    }

    if (set_error_handler_and_claim_root() == EXIT_FAILURE)
        return EXIT_FAILURE;

    init_workspaces();
    fill_monitors();
    dump_monitor_info();
    init_keyboard();
    set_cursors();
    set_atoms();
    set_defaults();
    parse_config();
    create_pipe();

    // After command binds are created
    establish_root_passive_key_grabs();

    int ws = ewmh_get_current_desktop();
    ewmh_advertise_number_of_desktops();
    ewmh_set_current_desktop(ws);
    set_current_workspace(ws);
    write_ws_state(ws);

    Window root_return, parent_return;
    Window* children;
    unsigned int nchildren;

    if (scan_existing_windows(
        root, &root_return,
        &parent_return, &children, 
        &nchildren
    ) == STATUS_FAIL) {
        XCloseDisplay(dp);
        return EXIT_FAILURE;
    }

    // Pass ownership of children to this call,
    // it is responsible for freeing this resource.
    manage_existing(children, nchildren);

    // Map existing unmapped
    map_current_workspace();

    // Because windows may be already mapped, don't rely on
    // MapNotify to arrange tiles.
    arrange_monitors(CurrentTime, NO_JUSTIFY_FOCUS);

    // Give window focus doesn't generate focus change events
    // if the window already has focus, so we change the 
    // state here.
    Client* initial_focus = get_next_focus(
        get_primary_monitor_or_fallback(
            get_workspace_monitors(get_current_workspace()
        )),
        NO_FS_REMOVE
    );

    set_current_focus(initial_focus, CurrentTime);
    handle_focus_state_changes(initial_focus, NULL);

    Position pointer_pos = get_pointer_pos();
    pp_set_xy(pos_x(&pointer_pos), pos_y(&pointer_pos));

    return EXIT_SUCCESS;
}

void epilogue() {
    cleanup();
}
