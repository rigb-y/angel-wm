/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "icccm.h"
#include "angel.h"
#include "angel_strings.h"
#include "utils.h"
#include "atoms.h"
#include "ewmh.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdbool.h>

#define FLAG_UNKNOWN 0L

typedef enum SizeHintField {
    SHF_UNKNOWN,
    SHF_MIN_WIDTH,
    SHF_MAX_WIDTH,
    SHF_MIN_HEIGHT,
    SHF_MAX_HEIGHT,
    SHF_WIDTH_INC,
    SHF_HEIGHT_INC,
    SHF_BASE_WIDTH,
    SHF_BASE_HEIGHT,
    SHF_MIN_ASPECT,
    SHF_MAX_ASPECT,
    SHF_WIDTH,
    SHF_HEIGHT
} SizeHintField;

typedef struct SHFFlagPair {
    SizeHintField field;
    long flag;
} SHFFlagPair;

static const SHFFlagPair shf_to_flag[] = {
    {SHF_MIN_WIDTH, PMinSize},
    {SHF_MIN_HEIGHT, PMinSize},
    {SHF_MAX_WIDTH, PMaxSize},
    {SHF_MAX_HEIGHT, PMaxSize},
    {SHF_WIDTH_INC, PResizeInc},
    {SHF_HEIGHT_INC, PResizeInc},
    {SHF_BASE_WIDTH, PBaseSize},
    {SHF_BASE_HEIGHT, PBaseSize},
    {SHF_MIN_ASPECT, PAspect},
    {SHF_MAX_ASPECT, PAspect},
    {SHF_WIDTH, PSize},
    {SHF_HEIGHT, PSize},
};

static long convert_shf_to_flag(SizeHintField);
static int get_size_hint(Window, SizeHintField);
static Point get_aspect_hint(Window, SizeHintField);

_Bool is_state_valid(int state) {
    return state == NormalState 
        || state == WithdrawnState 
        || state == IconicState;
}

long convert_shf_to_flag(SizeHintField field) {
    for (int I = 0; I < ARRAY_SIZE(shf_to_flag); ++I) 
        if (shf_to_flag[I].field == field)
            return shf_to_flag[I].flag;
    return FLAG_UNKNOWN;
}

XClassHint* get_class_hints(Window win) {
    if (win == None) return NULL;

    XClassHint* hints;
    if ((hints = XAllocClassHint()) == NULL)
        return NULL;

    Status st;
    if ((st = XGetClassHint(dp, win, hints)) == 0)
        return NULL;

    return hints;
}

const char* get_window_class_name(Window win) {
    if (win == None) return NULL;

    XClassHint* hints;
    if ((hints = get_class_hints(win)) == NULL)
        return NULL;

    if (hints->res_class == NULL) {
        free_class_hints(hints);
        return NULL;
    }

    string str_name = create_string();
    string_append_chars(
        &str_name,
        hints->res_class,
        get_raw_str_size(hints->res_class)
    );

    const char* name = string_data_copy(&str_name); 
    string_destroy(&str_name);
    free_class_hints(hints);

    return name;
}

XSizeHints* get_size_hints(Window win, long* mask) {
    if (win == None) return NULL;

    *mask = 0;

    XSizeHints* hints; 
    if ((hints = XAllocSizeHints()) == NULL) 
        return NULL;

    Status st; 
    if ((st = XGetWMNormalHints(dp, win, hints, mask)) == 0) {
        XFree(hints);
        return NULL;
    }

    return hints;
}

int get_size_hint(Window win, SizeHintField field) {
    if (win == None) return 0;

    long mask = 0;

    XSizeHints* hints; 
    if ((hints = get_size_hints(win, &mask)) == NULL)
        return -1;

    long flag;
    if ((flag = convert_shf_to_flag(field)) == FLAG_UNKNOWN) {
        XFree(hints);
        return -1;
    }

    if (!(mask & flag)) {
        XFree(hints);
        return -1;
    }
    
    int ret = 0;
    switch (field) {
        case SHF_UNKNOWN:
        case SHF_MIN_ASPECT:
        case SHF_MAX_ASPECT:
            ret = -1;
            break;

        case SHF_MIN_WIDTH:
			ret = hints->min_width;
			break;

        case SHF_MAX_WIDTH:
			ret = hints->max_width;
			break;
        case SHF_MIN_HEIGHT: 
			ret = hints->min_height;
			break;
        case SHF_MAX_HEIGHT: 
			ret = hints->max_height;
			break;
        case SHF_WIDTH_INC: 
			ret = hints->width_inc;
			break;
        case SHF_HEIGHT_INC: 
			ret = hints->height_inc;
			break;
        case SHF_BASE_WIDTH: 
			ret = hints->base_width;
			break;
        case SHF_BASE_HEIGHT: 
			ret = hints->base_height;
			break;
        case SHF_WIDTH: 
			ret = hints->width;
			break;
        case SHF_HEIGHT: 
			ret = hints->height;
			break;
    }

    XFree(hints);
    return ret;
}

Point get_aspect_hint(Window win, SizeHintField field) {
    if (win == None) return (Point){0};

    long mask = 0; long flag;
    if ((flag = convert_shf_to_flag(field)) == FLAG_UNKNOWN) {
        return (Point){.x = -1, .y = -1};
    }

    XSizeHints* hints; 
    if ((hints = get_size_hints(win, &mask)) == NULL) 
        return (Point){.x = -1, .y = -1};


    if (!(mask & flag)) {
        XFree(hints);
        return (Point){.x = -1, .y = -1};
    }

    Point point = {0};

    switch (field) {
        case SHF_MIN_ASPECT:
            point.x = hints->min_aspect.x;
            point.y = hints->min_aspect.y;
            break;

        case SHF_MAX_ASPECT:
            point.x = hints->max_aspect.x;
            point.y = hints->max_aspect.y;
            break;

        default: 
            point.x = -1;
            point.y = -1;
    }

    XFree(hints);
    return point;
}

int get_min_width_hint(Window win) {
    if (win == None) return 0;

    return get_size_hint(win, SHF_MIN_WIDTH);
}

int get_max_width_hint(Window win) {
    if (win == None) return 0;

    return get_size_hint(win, SHF_MAX_WIDTH);
}

int get_min_height_hint(Window win) {
    if (win == None) return 0;

    return get_size_hint(win, SHF_MIN_HEIGHT);
}

int get_max_height_hint(Window win) {
    if (win == None) return 0;

    return get_size_hint(win, SHF_MAX_HEIGHT);
}

int get_width_inc_hint(Window win) {
    if (win == None) return 0;

    return get_size_hint(win, SHF_WIDTH_INC);
}

int get_height_inc_hint(Window win) {
    if (win == None) return 0;

    return get_size_hint(win, SHF_HEIGHT_INC);
}

int get_base_width_hint(Window win) {
    if (win == None) return 0;

    return get_size_hint(win, SHF_BASE_WIDTH);
}

int get_base_height_hint(Window win) {
    if (win == None) return 0;

    return get_size_hint(win, SHF_BASE_HEIGHT);
}

int get_width_hint(Window win) {
    if (win == None) return 0;

    return get_size_hint(win, SHF_WIDTH);
}

int get_height_hint(Window win) {
    if (win == None) return 0;

    return get_size_hint(win, SHF_HEIGHT);
}

Point get_min_aspect_hint(Window win) {
    if (win == None) return (Point){0};

    return get_aspect_hint(win, SHF_MIN_ASPECT);
}

Point get_max_aspect_hint(Window win) {
    if (win == None) return (Point){0};

    return get_aspect_hint(win, SHF_MAX_ASPECT);
}

XWMHints* get_wm_hints(Window win) {
    if (win == None) return NULL;

    XWMHints* wm_hints;
    if ((wm_hints = XGetWMHints(dp, win)) == NULL)
        return NULL;

    return wm_hints;
}

_Bool get_input_hint(Window win) {
    if (win == None) return false;

    XWMHints* wm_hints;
    if ((wm_hints = get_wm_hints(win)) == NULL)
        return true;

    _Bool ret = wm_hints->flags & InputHint 
        ? wm_hints->input 
        : true;

    XFree(wm_hints);
    return ret;
}

_Bool get_urgency_hint(Window win) {
    if (win == None) return false;

    XWMHints* wm_hints;
    if ((wm_hints = get_wm_hints(win)) == NULL)
        return false;

    _Bool ret = wm_hints->flags & XUrgencyHint;

    XFree(wm_hints);
    return ret;
}

XID get_window_group(Window win) {
    if (win == None) return None;

    XWMHints* wm_hints;
    if ((wm_hints = get_wm_hints(win)) == NULL)
        return false;

    XID ret;
    if (!(wm_hints->flags & WindowGroupHint))
        ret = 0;

    ret = wm_hints->window_group;

    XFree(wm_hints);
    return ret;
}

void free_class_hints(XClassHint* class_hints) {
    if (class_hints == NULL) return;

    XFree(class_hints->res_name);
    XFree(class_hints->res_class);
    XFree(class_hints);
}

_Bool window_supports_protocl(Window win, Atom atom) {
    if (win == None) return false;

    Status st;
    Atom* atom_returns = NULL;
    int count = 0;
    if ((st = XGetWMProtocols(dp, win, &atom_returns, &count)) == 0)
        return false;

    for (int i = 0; i < count; ++i) {
        if (atom_returns[i] == atom) {
            XFree(atom_returns);
            return true;
        }
    }

    XFree(atom_returns);
    return false;
}

_Bool window_supports_delete(Window win) {
    if (win == None) return false;

    return window_supports_protocl(win, wm_delete_window);
}

_Bool window_supports_take_focus(Window win) {
    if (win == None) return false;

    return window_supports_protocl(win, wm_take_focus);
}

void send_client_message(Window win, Atom atom, Time time) {
    if (win == None || atom == None) return;

    XEvent event = {0};

    event.xclient.type = ClientMessage;
    event.xclient.window = win;
    event.xclient.message_type = wm_protocols;
    event.xclient.format = 32;
    event.xclient.data.l[0] = atom;
    event.xclient.data.l[1] = time != (Time)NULL ? time : CurrentTime;

    XSendEvent(dp, win, false, NoEventMask, &event);
}

void send_delete(Window win, Time time) {
    if (win == None) return;

    send_client_message(win, wm_delete_window, time);
}

void send_take_focus(Window win, Time time) {
    if (win == None) return;

    send_client_message(win, wm_take_focus, time);
}

void send_synthetic_configure(Window win, int x, int y, int width, int height, int border_width) {
    if (win == None) return;

    XEvent event = {0};
    event.type = ConfigureNotify;

    event.xconfigure.x = x;
    event.xconfigure.y = y;
    event.xconfigure.width = width;
    event.xconfigure.height = height;
    event.xconfigure.border_width = border_width;

    event.xconfigure.event = win;
    event.xconfigure.window = win;
    event.xconfigure.above = None;
    event.xconfigure.override_redirect = false;

    send_event(win, &event);
}

void send_event(Window win, XEvent* event) {
    if (win == None) return;

    XSendEvent(dp, win, false, StructureNotifyMask, event);
}

void set_wm_state(Window win, long state) {
    if (win == None 
        || !is_state_valid(state)
    ) return;

    long data[2] = {
        state,
        None
    };

    XChangeProperty(
        dp,
        win,
        wm_state_atom,
        wm_state_atom,
        32,
        PropModeReplace,
        (unsigned char*)data,
        2
    );
}

int get_wm_state(Window win) {
    if (win == None) return WithdrawnState;

    unsigned long* data = NULL;
    int format = 32;
    int offset = 0;
    int length = 1;
    int expected = 1;

    _Bool st = ewmh_read_property(
        win,
        wm_state_atom,
        wm_state_atom,
        &data,
        format,
        offset,
        length,
        expected
    );

    if (st == false) {
        XFree(data);
        return NormalState;
    }

    int state = (int)data[0];
    XFree(data);
    return state;
}

_Bool win_is_transient(Window win, Window* parent_return) {
    if (win == None || parent_return == NULL) return false;

    return XGetTransientForHint(dp, win, parent_return);
}
