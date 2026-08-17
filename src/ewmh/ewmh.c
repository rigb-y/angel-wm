/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "ewmh.h"
#include "angel.h"
#include "atoms.h"
#include "workspaces.h"
#include "docks.h"

#include <X11/Xatom.h>
#include <stdbool.h>

void ewmh_store_workspace_num(Window win, unsigned long workspace) {
    if (win == None || !workspace_is_valid(workspace)) return;

    Atom type = XA_CARDINAL;
    int format = 32;
    int nelements = 1;

    ewmh_change_property(
        win,
        workspace,
        net_wm_desktop,
        type,
        format,
        nelements
    );
}

int ewmh_get_workspace_num(Window win) {
    if (win == None) return -1; 

    unsigned long* workspace = NULL;

    int format = 32;
    long offset = 0;
    long length = 1; /* multiple of 32-bits */
    unsigned long expected_items = 1;

    int ws = ewmh_read_property(
        win,
        net_wm_desktop,
        XA_CARDINAL,
        &workspace,
        format,
        offset,
        length,
        expected_items
    ) == true ? (int)workspace[0]: -2;

    XFree(workspace);
    return ws;
}

void store_monitor_name_property(Window win, Atom name) {
    if (win == None || name == None) return;

    unsigned long data = name;
    Atom type = XA_ATOM;
    int format = 32;
    int nelements = 1;

    ewmh_change_property(
        win, 
        data,
        monitor_name_property,
        type,
        format,
        nelements
    );
}

Atom get_monitor_name_property(Window win) {
    if (win == None) return None;

    unsigned long* property = NULL;
    Atom type = XA_ATOM;
    int format = 32;
    long offset = 0;
    long length = 1;
    unsigned long expected_items = 1;

    Atom ret = ewmh_read_property(
        win,
        monitor_name_property,
        type,
        &property,
        format, 
        offset,
        length,
        expected_items
    ) == true ? (Atom)property[0] : None;

    XFree(property);
    return ret;
}

Atom ewmh_get_window_type(Window win) {
    if (win == None) return None;

    unsigned long* property = NULL;
    Atom type = XA_ATOM;
    int format = 32;
    long offset = 0;
    long length = 1;
    unsigned long expected_items = 1;

    Atom ret = ewmh_read_property(
        win,
        net_wm_window_type,
        type,
        &property,
        format,
        offset,
        length,
        expected_items
    ) == true ? (Atom)property[0] : None;

    XFree(property);
    return ret;
}

_Bool ewmh_window_is_dock(Window win) {
    if (win == None) return false;

    return ewmh_get_window_type(win) 
        == net_wm_window_type_dock;
}

Strut ewmh_get_dock_strut(Window win) {
    if (win == None) return (Strut){0};

    Strut strut = {0};

    unsigned long* strut_property = NULL;
    Atom type = XA_CARDINAL;
    int format = 32;
    long offset = 0;
    long length = 4;
    unsigned long expected_items = 4;

    _Bool st = ewmh_read_property(
        win,
        net_wm_strut,
        type,
        &strut_property,
        format,
        offset,
        length,
        expected_items
    );

    if (st != false) {
        fill_strut(
            &strut,
            strut_property[0],
            strut_property[1],
            strut_property[2],
            strut_property[3]
        );
    }

    XFree(strut_property);
    length = 12;
    expected_items = 12;

    unsigned long* strut_partial_property = NULL;

    st = ewmh_read_property(
        win,
        net_wm_strut_partial,
        type,
        &strut_partial_property,
        format,
        offset,
        length,
        expected_items
    );

    if (st == false) {
        XFree(strut_partial_property);
        return strut;
    }

    fill_strut_partial(
        &strut, 
        strut_partial_property[0],
        strut_partial_property[1],
        strut_partial_property[2],
        strut_partial_property[3],
        strut_partial_property[4],
        strut_partial_property[5],
        strut_partial_property[6],
        strut_partial_property[7],
        strut_partial_property[8],
        strut_partial_property[9],
        strut_partial_property[10],
        strut_partial_property[11]
    );

    XFree(strut_partial_property);
    return strut;
}

void ewmh_advertise_number_of_desktops() {
    unsigned long data = N_WORKSPACES;
    Atom type = XA_CARDINAL;
    int format = 32;
    int n_elements = 1;

    ewmh_change_property(
        root,
        data,
        net_number_of_desktops,
        type,
        format,
        n_elements
    );
}

int ewmh_get_current_desktop() {
    unsigned long* data = NULL;
    Atom type = XA_CARDINAL;
    int format = 32;
    int offset = 0;
    int length = 1;
    int expected = 1;

    _Bool st = ewmh_read_property(
        root,
        net_current_desktop,
        type,
        &data,
        format,
        offset,
        length,
        expected
    );

    if (!st) {
        XFree(data);
        return 0;
    }

    int ws = (int)data[0];
    XFree(data);
    return ws;
}

void ewmh_set_current_desktop(int desktop) {
    if (!workspace_is_valid(desktop)) return;

    unsigned long data = desktop;
    Atom type = XA_CARDINAL;
    int format = 32;
    int n_elements = 1;

    ewmh_change_property(
        root,
        data,
        net_current_desktop,
        type,
        format,
        n_elements
    );
}

// win == None is okay.
void ewmh_set_active_window(Window win) {
    if (win == None) return;

    unsigned long data = win;
    Atom type = XA_WINDOW;
    int format = 32;
    int n_elements = 1;

    ewmh_change_property(
        root,
        data,
        net_active_window,
        type,
        format,
        n_elements
    );
}

void ewmh_change_property(
    Window win,
    unsigned long data,
    Atom property,
    Atom type,
    int format,
    int nelements
) {
    if (win == None || property == None || type == None) 
        return;

    XChangeProperty(
        dp, 
        win, 
        property,
        type,
        format, 
        PropModeReplace,
        (unsigned char*)&data,
        nelements
    );
}

_Bool ewmh_read_property(
    Window win,
    Atom property,
    Atom type,
    unsigned long** data,
    int format,
    long offset,
    long length,
    unsigned long expected_items
) {
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char* prop;

    if (win == None || property == None || type == None) 
        return false;

    Status st = XGetWindowProperty(
        dp, 
        win, 
        property,
        offset,
        length,
        false,
        type,
        &actual_type,
        &actual_format,
        &nitems,
        &bytes_after,
        &prop
    );

    if (st != Success
        || actual_type != type
        || actual_format != format 
        || nitems != expected_items
        || bytes_after != 0
    ) { 

        return false;
    }

    *data = ((unsigned long*)prop);
    return true;
}
