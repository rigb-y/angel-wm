/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "geometry.h"
#include "events.h"
#include "angel.h"
#include "windows.h"
#include "keymaps.h"
#include "terminal.h"
#include "manage.h"
#include "client.h"
#include "workspaces.h"
#include "motion_tree.h"
#include "node.h"
#include "types.h"
#include "client_list.h"
#include "focus_stack.h"
#include "pointer.h"
#include "detached.h"
#include "float_list.h"
#include "minimized_client.h"
#include "minimized_list.h"
#include "shell.h"
#include "keyboard.h"
#include "utils.h"
#include "icccm.h"
#include "unmapped_client.h"
#include "unmapped_list.h"
#include "ewmh.h"
#include "monitors.h"
#include "monitor.h"
#include "focus.h"
#include "randr.h"
#include "docks.h"
#include "wspipe.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <stdbool.h>
#include <stdlib.h>

static WM_STATE wm_state = WM_RUNNING;

typedef struct EventMap {
    EventType type;
    EventHandleFn handle_fn;
} EventMap;

static const EventMap event_maps[] = {
    {EnterNotify, handle_enter_notify},
    {LeaveNotify, handle_leave_notify},
    {KeyPress, handle_key_press},
    {MapRequest, handle_map_request},
    {UnmapNotify, handle_unmap_notify},
    {DestroyNotify, handle_destroy_notify},
    {ButtonPress, handle_button_press},
    {ButtonRelease, handle_button_release},
    {MotionNotify, handle_motion_notify}, 
    {MapNotify, handle_map_notify}, 
    {FocusIn, handle_focus_change},
    {FocusOut, handle_focus_change},
    {MappingNotify, handle_mapping_notify},
    {ConfigureRequest, handle_configure_request}, 
};

static Client* MUST_LEAVE = NULL;

static void handle_focus_in(Client*);
static void handle_focus_out(Client*);

static EventHandleFn get_event_handle_fn(EventType);
static void action_motion(const XKeyEvent*, TreeMotionFn, int, int);
static void action_win_switch(const XKeyEvent*, TreeMotionFn);
static void action_change_resize_mode(_Bool);
static void action_resize(ResizeStepFn, int, int);
static KeybindActionFn get_action(const KeyMap*);
static void action_change_workspace_layout(Layout);
static void action_switch_workspace(int);
static void action_move_to_workspace(int, Time);

static void action_motion_from_cl(Monitor*, TreeMotionFn, int, int, int, Time);
static void action_motion_from_fl(Monitor*, int, int, int);
static void action_motion_from_ml(Monitor*, int, int, int, Time);

static void action_change_focus_mode(FocusMode);
static void action_cycle_tiled(_Bool, Time);

_Bool client_is_must_leave(Client* client) {
    if (client == NULL) return false;
    return client == MUST_LEAVE;
}

void set_must_leave(Client* client) {
    MUST_LEAVE = client;
}

void clear_must_leave() {
    MUST_LEAVE = NULL;
}

_Bool wm_running() {
    return wm_state == WM_RUNNING;
}

_Bool wm_restart() {
    return wm_state == WM_RESTART;
}

_Bool wm_quit() {
    return wm_state == WM_QUIT;
}

void handle_nothing(const XEvent* event) { return; }

void handle_enter_notify(const XEvent* event) {
    if (MUST_LEAVE != NULL || IN_RESIZE) return;

    const XCrossingEvent* enter_event = &event->xcrossing;
    Window event_window = enter_event->window;
    Time event_time = enter_event->time;

    Position pp = create_position(
        enter_event->x_root,
        enter_event->y_root
    );

    _Bool pointer_moved = pp_has_moved(&pp);

    pp_set_xy(
        enter_event->x_root,
        enter_event->y_root
    );

    if (enter_event->detail == NotifyInferior 
        || enter_event->mode != NotifyNormal 
        || enter_event->mode == NotifyGrab
        || enter_event->mode == NotifyUngrab
    ) return;

    if (!pointer_moved) return;

    if (event_window == root && enter_event->subwindow != None) {
        event_window = enter_event->subwindow;
    }

    if (!is_managed(event_window)) return;

    Client* client = get_managed(event_window);
    set_current_focus(client, enter_event->time); 
}

void handle_leave_notify(const XEvent* event) {
    if (IN_RESIZE) return;

    const XCrossingEvent* exit_event = &event->xcrossing;
    Window event_window = exit_event->window;

    if (exit_event->detail == NotifyInferior 
        || exit_event->mode != NotifyNormal 
        || exit_event->mode == NotifyGrab
        || exit_event->mode == NotifyUngrab
    ) return;

    if (event_window == root && exit_event->subwindow != None) {
        event_window = exit_event->subwindow;
    }

    if (!is_managed(event_window)) return;

    Client* client = get_managed(exit_event->window);
    if (client_is_must_leave(client)) 
        clear_must_leave();
    
    if (get_client_leave_notify_from_motion(client)) {
        set_client_leave_notify_from_motion(client, false);
        return;
    }

    if (get_client_in_resize_mode(client)) {
        set_client_in_resize_mode(client, false);
    }
}

void handle_key_press(const XEvent* event) {
    const XKeyEvent* key_event = &event->xkey;
    KeyMap keymap = create_keymap(
        get_keysym(key_event->keycode), 
        key_event->state & ~(LockMask | Mod2Mask)
    );

    dispatch_keybind(key_event, &keymap);
}

void handle_button_press(const XEvent* event) {
    if (!is_managed(event->xbutton.window)) return;

    const XButtonEvent* button_event = &event->xbutton;
    Window event_win = button_event->window;

    // Will be non null since is_managed is true
    Client* client = get_managed(event_win);
    int workspace = reconcile_clients_workspace(client);

    if (workspace != get_current_workspace())
        return;

    Monitor* monitor = get_monitor_from_list_membership(
        get_workspace_monitors(workspace),
        client
    );

    DetachedClient* detached = fl_find_from_win(
        get_monitor_fl(monitor),
        event_win
    );

    if (detached != NULL && client_is_float(client) 
        && is_drag_win(button_event->button, button_event->state)
    ) {
        set_move_mode(detached, true);
        set_detached_pointer_xy(
            detached,
            button_event->x_root,
            button_event->y_root
        );

        return;
    }

    else if (detached != NULL && client_is_float(client)
        && is_mouse_resize(button_event->button, button_event->state)
    ) {
        set_mouse_resize_mode(detached, true);
        set_detached_pointer_xy(
            detached,
            button_event->x_root,
            button_event->y_root
        );
        return;
    }

    if (client_is_current_focus(client)) {
        XAllowEvents(dp, ReplayPointer, button_event->time);
        return;
    }

    set_current_focus(client, button_event->time);
    XAllowEvents(dp, ReplayPointer, button_event->time);
}

void handle_button_release(const XEvent* event) {
    if (!is_managed(event->xbutton.window)) return;

    Window event_window = event->xbutton.window;
    Client* client = get_managed(event_window);
    int workspace = reconcile_clients_workspace(client);

    if (workspace != get_current_workspace())
        return;

    Monitor* monitor = get_monitor_from_list_membership(
        get_workspace_monitors(workspace),
        client
    );

    if (client_is_float(client)) {
        DetachedClient* detached =  fl_find_from_win(
            get_monitor_fl(monitor),
            event_window
        );

        set_move_mode(
            detached,
            false
        );

        set_mouse_resize_mode(
            detached,
            false
        );
    }
}

void handle_motion_notify(const XEvent* event) {
    const XMotionEvent* motion_event = &event->xmotion;

    pp_set_xy(motion_event->x_root, motion_event->y_root);

    if (!is_managed(event->xmotion.window) 
        || !client_is_float(get_managed(event->xmotion.window))
    ) return;

    Client* client = get_managed(event->xmotion.window);
    int workspace = reconcile_clients_workspace(client);

    if (workspace != get_current_workspace())
        return;

    Monitor* monitor = get_monitor_from_list_membership(
        get_workspace_monitors(workspace),
        client
    );

    DetachedClient* detached = fl_find_from_win(
        get_monitor_fl(monitor),
        event->xmotion.window
    );

    Position new_pos;
    int dx, dy; 
    if (get_move_mode(detached) || get_mouse_resize_mode(detached)) {
        new_pos = get_pointer_pos();

        dx = pos_x(&new_pos) - get_detached_pointer_x(detached);
        dy = pos_y(&new_pos) - get_detached_pointer_y(detached);
        set_detached_pointer_xy(detached, pos_x(&new_pos), pos_y(&new_pos));
    }

    if (get_move_mode(detached)) {
        move_floater(monitor, detached, dx, dy);
    } 

    else if (get_mouse_resize_mode(detached)) {
        resize_floater(monitor, detached, dx, dy);
    }
}

void handle_focus_change(const XEvent* event) {
    const XFocusChangeEvent* focus_change_event = &event->xfocus;
    Window win = focus_change_event->window;

    if (!is_managed(win) 
        || focus_change_event->mode == NotifyGrab 
        || focus_change_event->mode == NotifyUngrab
        || focus_change_event->detail == NotifyPointer
    ) return;

    Client* client = get_managed(win);
    switch (focus_change_event->type) {
        case FocusIn:
            handle_focus_in(client);
            break;
        case FocusOut:
            if (focus_change_event->detail != NotifyInferior)
                handle_focus_out(client);
    }
}

void handle_focus_in(Client* client) {
    if (client == NULL) return;

    handle_focus_state_changes(client, NULL);
}

void handle_focus_out(Client* client) {
    if (client == NULL) return;

    handle_focus_state_changes(NULL, client);
}

void handle_mapping_notify(const XEvent* event) {
    const XMappingEvent* mapping_event = &event->xmapping;
    if (mapping_event->request == MappingPointer) return;

    refresh_keyboard(mapping_event);
    ungrab_all_root_passive_grabs();
    get_keycodes();
    establish_root_passive_key_grabs();
}

void handle_configure_request(const XEvent* event) {
    const XConfigureRequestEvent* configure_request_event 
        = &event->xconfigurerequest;

    Window win = configure_request_event->window;
    if (!is_managed(win) || (get_managed(win) && client_is_dock(get_managed(win)))) {
        Client* client = get_managed(win);
        XWindowChanges changes = {
            .x = configure_request_event->x,
            .y = configure_request_event->y,
            .width = configure_request_event->width,
            .height = configure_request_event->height,
            .border_width = configure_request_event->border_width
        };
        unsigned int mask = CWX | CWY | CWWidth | CWHeight | CWBorderWidth;
        XConfigureWindow(dp, win, mask, &changes);
        set_window_border_width(win, configure_request_event->border_width);

        if (client != NULL && is_client_mapped(client)) {
            Monitors* monitors = get_workspace_monitors(
                get_client_on_workspace(client)
            );

            Monitor* monitor = get_monitor_from_list_membership(
                monitors,
                client
            );

            Monitor* moved_to = move_client_to_monitor_with_position(
                monitors,
                monitor,
                client
            );

            arrange_monitors(CurrentTime, NO_JUSTIFY_FOCUS);
        }

        return;
    }

    Client* managed = get_managed(win);
    if (!client_is_dock(managed)) {
        send_synthetic_configure(
            get_client_win(managed),
            get_client_x(managed),
            get_client_y(managed),
            get_client_width(managed),
            get_client_height(managed),
            get_client_border_width(managed)
        );

        return;
    }
}

void handle_screen_change_notify(const XEvent* event) {
    XRRUpdateConfiguration((XEvent*)event);
    update_monitors();
    arrange_monitors(CurrentTime, JUSTIFY_FOCUS);
}

void handle_rr_notify(const XEvent* event) {
    XRRNotifyEvent* notify_event = (XRRNotifyEvent*)event;

    switch (notify_event->subtype) {
        case RRNotify_OutputChange:
        case RRNotify_CrtcChange:
        case RRNotify_ResourceChange:
            update_monitors();
            arrange_monitors(CurrentTime, JUSTIFY_FOCUS);
            return;
    }
}

KeybindActionFn get_action(const KeyMap* keymap) {
    KeybindActionFn action = action_find(keymap, SEARCH_REMAPS);
    if (action != NULL) return action;

    return action_find(keymap, SEARCH_DEFAULTS);
}

void dispatch_keybind(const XKeyEvent* key_event, const KeyMap* keymap) {
    if (key_event == NULL || keymap == NULL) return;

    const char* command = command_find(*keymap);
    if (command != NULL) {
        exec_command(command);
        return;
    }

    KeybindActionFn action = get_action(keymap);
    if (action != NULL)
        action(key_event);
}

void handle_map_request(const XEvent* event) {
    const XMapRequestEvent* map_request = &event->xmaprequest;
    Window window = map_request->window;

    if (scan_other_workspaces_for_window(window))
        return;

    if (!is_managed(window)) 
        start_manage(window, get_win_attrs(window), NULL);

    Client* client = get_managed(window);

    if (client_is_dock(client)) {
        map_client(client);
        return;
    }

    client_set_should_set_focus(client, true);
    map_client(client);
}

void handle_unmap_notify(const XEvent* event) {
    const XUnmapEvent* unmap_event = &event->xunmap;
    Window win = unmap_event->window;

    if (!is_managed(win) || unmap_event->event != root) return;

    Client* client = get_managed(win);
    int ws = reconcile_clients_workspace(client);

    Monitors* monitors = get_workspace_monitors(ws);
    Monitor* monitor = get_monitor_from_list_membership(
        monitors,
        client
    );

    dec_mapped_in_workspace(ws);
    client_set_transition_state(client, CLIENT_UNMAPPED);

    if (client_pending_unmaps(client) > 0) {
        dec_pending_unmap(client);

        UnmappedClient* unmapped = detach_unmapped(client);
        if (client_get_future_unmap_stay_unmapped(client))
            unmapped_set_stay_unmapped(unmapped, STAY_UNMAPPED);

        ul_push(get_monitor_ul(monitor), unmapped);

        client_set_mapped(client, false);
        client_set_was_configured(client, false);

        if (ws == get_current_workspace() || client_get_moved(client)) {
            arrange_monitors(CurrentTime, JUSTIFY_FOCUS);
            client_set_moved(client, false);
        }

        if (client_get_map_after_unmap_notify(client)) {
            map_client(client);
            client_set_map_after_unmap_notify(client, false);
        }

        return;
    }

    // end_manage adjusts focus
    if (end_manage(win)) {
        // XDeleteProperty(dp, win, wm_state_atom);

        if (ws == get_current_workspace())
            arrange_monitor(monitor, CurrentTime, NO_JUSTIFY_FOCUS);
    }
}

void handle_map_notify(const XEvent* event) {
    const XMapEvent* map_event = &event->xmap;
    Window win = map_event->window;

    if (!is_managed(win) || map_event->event != root) return;

    Client* client = get_managed(win);
    int workspace = reconcile_clients_workspace(client);

    Monitors* monitors = get_workspace_monitors(workspace);
    Monitor* monitor = get_monitor_from_list_membership(monitors, client);

    client_set_mapped(client, true);
    client_set_transition_state(client, CLIENT_MAPPED);

    UnmappedClient* unmapped = ul_find_from_client(
        get_monitor_ul(monitor),
        client
    );

    inc_mapped_in_workspace(workspace);

    // Reset
    client_set_unmapped_from_workspace_switch(client, false);

    if (unmapped != NULL && client_is_dock(client)) {
        ul_reattach_dock(get_monitor_ul(monitor), unmapped);

        Monitor* moved_to = move_client_to_monitor_with_position(
            monitors,
            monitor,
            client
        );

        arrange_monitors(CurrentTime, NO_JUSTIFY_FOCUS);
        return;
    }

    if (unmapped != NULL)
        ul_reattach(get_monitor_ul(monitor), unmapped);

    if (workspace != get_current_workspace() 
        || client_get_unmap_after_map_notify(client)
    ) {
        client_set_unmap_after_map_notify(client, false);
        client_set_unmapped_from_workspace_switch(client, true);

        unmap_client(client, MANAGE_CONT);
        return;
    }
    
    if (get_current_focus() != NULL && 
        get_current_focus() == get_monitor_fullscreen(monitor)
    ) {
        client_set_fullscreen(get_current_focus(), false);
        set_monitor_fullscreen(monitor, NULL);

        set_current_focus(
            client,
            CurrentTime
        );

        client_set_fullscreen(client, true);
        set_monitor_fullscreen(monitor, client);
        arrange_monitor(monitor, CurrentTime, NO_JUSTIFY_FOCUS);
        raise_all_floaters(monitor);

        return;
    }

    if (get_focus_before_workspace_switch(get_current_workspace()) == client 
        || client_get_should_set_focus(client)
    ) {
        if (!monitor_fullscreen_exists(monitor))
            set_current_focus(
                client,
                CurrentTime
            );

        else
            set_current_focus(
                get_monitor_fullscreen(monitor),
                CurrentTime
            );

        if (get_focus_before_workspace_switch(get_current_workspace()) == client)
            set_focus_before_workspace_switch(
                get_current_workspace(), NULL
            );

        client_set_should_set_focus(client, false);
        arrange_monitor(monitor, CurrentTime, NO_JUSTIFY_FOCUS);

        return;
    }

    arrange_monitor(monitor, CurrentTime, NO_JUSTIFY_FOCUS);

    if (get_mapped_in_current_workspace() 
        == count_viewable_clients_in_current_workspace()
        && get_current_focus() == NULL
        && get_focus_before_workspace_switch(
            get_current_workspace()
        ) == NULL
    ) {
        set_current_focus(get_next_focus(monitor, NO_FS_REMOVE), CurrentTime);
    }
}

void handle_destroy_notify(const XEvent* event) {
    const XDestroyWindowEvent* destroy_event = &event->xdestroywindow;
    Window win = destroy_event->window;
    if (!is_managed(win)) return;

    Client* client = get_managed(win);
    int workspace = get_client_on_workspace(client);
    Monitor* monitor = get_monitor_from_list_membership(
        get_workspace_monitors(workspace),
        client
    );

    if (end_manage(win))
        arrange_monitor(monitor, CurrentTime, NO_JUSTIFY_FOCUS);
}

void action_noop(const XKeyEvent*) { return; }

void action_close_win(const XKeyEvent* key_event) {
    if (get_current_workspace() != reconcile_clients_workspace(get_current_focus()))
        return;

    Client* focus = get_current_focus();

    if (focus == NULL || 
        get_client_win(focus) == root || 
        !is_managed(get_client_win(focus)) ||
        !is_client_mapped(focus)
    ) return;

    Window focused_win = get_client_win(focus);

    if (!window_supports_delete(focused_win)) {
        kill_client(focused_win); 
        return;
    }

    send_delete(focused_win, key_event->time);
}

void action_open_term(const XKeyEvent* key_event) {
    Monitor* monitor = reconcile_monitor_from_focus(
        get_current_workspace(),
        get_current_focus()
    );

    spawn_terminal();
}

void action_monitor_motion(Monitors* monitors, Monitor* monitor, int x_sign, int y_sign, Time time) {
    if (monitor == NULL) return;

    Monitor* next_monitor = NULL;
    if (x_sign == +1 && y_sign == 0)
        next_monitor = get_monitor_to_right(monitors, monitor);

    else if (x_sign == -1 && y_sign == 0) 
        next_monitor = get_monitor_to_left(monitors, monitor);

    else if (x_sign ==0 && y_sign == +1)
        next_monitor = get_monitor_below(monitors, monitor);

    else if (x_sign == 0 && y_sign == -1)
        next_monitor = get_monitor_above(monitors, monitor);

    if (next_monitor == NULL || next_monitor == monitor)
        return;

    Client* monitor_first = monitor_get_first_client(next_monitor);
    if (monitor_first != NULL)
        set_current_focus(monitor_first, time);
}

void action_motion_from_cl(Monitor* monitor, TreeMotionFn move_to_fn, int workspace, int x_sign, int y_sign, Time time) {
    if (monitor == NULL || move_to_fn == NULL || !workspace_is_valid(workspace)) 
        return;

    MotionTree* curr_motion_tree = get_monitor_mt(monitor);

    MTNode* node_carrying_focus = mt_find_node_carrying_client(
        curr_motion_tree, get_current_focus()
    );

    MTNode* move_to_node = move_to_fn(
        curr_motion_tree,
        node_carrying_focus
    );

    Client* move_to_client = get_node_client(move_to_node);

    if (move_to_node == NULL 
        || move_to_node == mt_root(curr_motion_tree)
    ) {
        action_monitor_motion(
            get_workspace_monitors(workspace),
            monitor,
            x_sign,
            y_sign,
            time
        );
    }

    if (move_to_node == mt_root(curr_motion_tree))
        move_to_node = NULL;

    MinimizedPosition pos = get_minimized_position(monitor);
    if (move_to_node == NULL 
        && !ml_empty(get_monitor_ml(monitor)) 
        && ((y_sign == +1 && pos == MINIMIZED_BOTTOM)
            || (y_sign == -1 && pos == MINIMIZED_TOP)
            || (x_sign == +1 && pos == MINIMIZED_RIGHT)
            || (x_sign == -1 && pos == MINIMIZED_LEFT))
    ) {
        set_current_focus(
            get_client_from_minimized(
                ml_head(
                    get_monitor_ml(monitor)
                )
            ), 
            time
        );

        return;
    }

    if (move_to_node == NULL || move_to_node->client == NULL) 
        return;

    set_current_focus(
        move_to_node->client,
        time
    );
}

void action_motion_from_fl(Monitor* monitor, int workspace, int x_sign, int y_sign) {
    if (monitor == NULL || !workspace_is_valid(workspace)) 
        return;

    move_floater(
        monitor,
        fl_find_from_client(
            get_monitor_fl(monitor),
            get_current_focus()
        ),

        get_floating_window_keyboard_movement_step() * x_sign,
        get_floating_window_keyboard_movement_step() * y_sign
    );
}

void action_motion_from_ml(Monitor* monitor, int workspace, int x_sign, int y_sign, Time time) {
    if (monitor == NULL || !workspace_is_valid(workspace))
        return;

    MinimizedPosition pos = get_minimized_position(monitor);
    if (y_sign == -1 && pos == MINIMIZED_BOTTOM
        || y_sign == +1 && pos == MINIMIZED_TOP
        || x_sign == -1 && pos == MINIMIZED_RIGHT
        || x_sign == +1 && pos == MINIMIZED_LEFT
    ) {
        set_current_focus(
            get_next_focus(monitor, NO_FS_REMOVE),
            time
        );

        return;
    }

    MinimizedClient* minimized = ml_find_from_client(
        get_monitor_ml(monitor),
        get_current_focus()
    );

    MinimizedPosition position = get_minimized_position(monitor);
    _Bool move_to_next = ((x_sign == +1 || x_sign == -1)
        && (position == MINIMIZED_BOTTOM || position == MINIMIZED_TOP))
        || ((y_sign == +1 || y_sign == -1)
        && (position == MINIMIZED_LEFT || position == MINIMIZED_RIGHT));

    _Bool positive_motion = (x_sign == +1 || y_sign == +1);

    MinimizedClient* next_minimized = positive_motion
        ? minimized->next
        : minimized->prev;

    if (move_to_next == false)
        next_minimized = NULL;

    if (next_minimized != NULL)
        set_current_focus(
            get_client_from_minimized(next_minimized),
            time
        );
}

void action_motion(const XKeyEvent* key_event, TreeMotionFn move_to_fn, int x_sign, int y_sign) {
    if (key_event == NULL 
        || get_current_focus() == NULL 
        || move_to_fn == NULL
    ) return;

    Client* focus = get_current_focus();
    int workspace = reconcile_clients_workspace(focus);
    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    // Client is in client list, this action should move focus between clients.
    if (cl_find_client(get_monitor_cl(monitor), focus))
        action_motion_from_cl(
            monitor,
            move_to_fn,
            workspace,
            x_sign,
            y_sign,
            key_event->time
        );

    // Client is in float list, this action should actually move the client.
    else if (client_is_float(focus))
        action_motion_from_fl(
            monitor,
            workspace,
            x_sign,
            y_sign
        );

    // Client is minimized
    else if (client_is_minimized(focus))
        action_motion_from_ml(
            monitor,
            workspace,
            x_sign,
            y_sign,
            key_event->time
        );
}

void action_win_down(const XKeyEvent* key_event) {
    action_motion(key_event, mt_below_of, 0, +1);
}

void action_win_up(const XKeyEvent* key_event) {
    action_motion(key_event, mt_above_of, 0, -1);
}

void action_win_left(const XKeyEvent* key_event) {
    action_motion(key_event, mt_left_of, -1, 0);
}

void action_win_right(const XKeyEvent* key_event) {
    action_motion(key_event, mt_right_of, +1, 0);
}

void action_win_switch(const XKeyEvent* key_event, TreeMotionFn fn) {
    if (key_event == NULL || get_current_focus() == NULL) return;

    Client* focus = get_current_focus();
    int workspace = reconcile_clients_workspace(focus);

    if (get_current_workspace() != workspace)
        return;

    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);
    MotionTree* curr_motion_tree = get_monitor_mt(monitor);

    MTNode* node_carrying_focus = mt_find_node_carrying_client(
        curr_motion_tree,
        focus
    );

    MTNode* node_next_to = fn(curr_motion_tree, node_carrying_focus);

    if (!node_next_to) return;

    Client* a = node_carrying_focus->client;
    Client* b = node_next_to->client;

    if (reconcile_clients_workspace(a) != reconcile_clients_workspace(b)) return;

    cl_swap_clients(get_monitor_cl(monitor), a, b);
    arrange_monitor(monitor, key_event->time, JUSTIFY_FOCUS);
    set_current_focus(a, key_event->time);
    set_client_leave_notify_from_motion(a, true);
    
    set_must_leave(b);
}

void action_win_move_down(const XKeyEvent* key_event) {
    action_win_switch(key_event, mt_below_of);
}

void action_win_move_up(const XKeyEvent* key_event) {
    action_win_switch(key_event, mt_above_of);
}

void action_win_move_left(const XKeyEvent* key_event) {
    action_win_switch(key_event, mt_left_of);
}

void action_win_move_right(const XKeyEvent* key_event) {
    action_win_switch(key_event, mt_right_of);
}

void action_change_resize_mode(_Bool mode) {
    if (get_current_focus() == NULL) return;

    Client* client = get_current_focus();
    if (get_current_workspace() != reconcile_clients_workspace(client))
        return;

    int old_border_width = get_client_border_width(client);

    set_client_in_resize_mode(client, mode);

    int new_border_width = get_border_width(client);
    int new_width = (get_client_width(client) + 2 * old_border_width) 
        - 2 * new_border_width;
    int new_height = (get_client_height(client) + 2 * old_border_width) 
        - 2 * new_border_width;
    resize_client(client, new_width, new_height);
}

void action_enter_resize(const XKeyEvent* key_event) {
    if (IN_RESIZE) {
        action_exit_resize(key_event);
        return;
    }

    if (get_current_focus() == NULL 
        || client_is_minimized(get_current_focus())
    ) return;

    action_change_resize_mode(true);
    IN_RESIZE = true;
}

void action_exit_resize(const XKeyEvent* key_event) {
    if (!IN_RESIZE) {
        action_enter_resize(key_event);
        return;
    }

    if (get_current_focus() == NULL 
        || client_is_minimized(get_current_focus())
    ) return;

    action_change_resize_mode(false);
    IN_RESIZE = false;
}

void action_resize(ResizeStepFn step_fn, int x_sign, int y_sign) {
    if (get_current_focus() == NULL
        || !get_client_in_resize_mode(get_current_focus())
    ) return; 

    Client* focus = get_current_focus();
    int workspace = reconcile_clients_workspace(focus);

    if (get_current_workspace() != workspace)
        return;

    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    if (cl_find_client(get_monitor_cl(monitor), focus)) {
        step_fn(get_current_focus());
        arrange_monitor(monitor, CurrentTime, JUSTIFY_FOCUS);
    }

    else {
        DetachedClient* detached = fl_find_from_client(
            get_monitor_fl(monitor),
            focus
        );

        int dw = get_window_width_resize_inc(get_client_win(focus)) * x_sign;
        int dh = get_window_height_resize_inc(get_client_win(focus)) * y_sign;

        resize_floater(monitor, detached, dw, dh);
    }
}

void action_resize_down(const XKeyEvent* key_event) {
    action_resize(inc_client_resize_step_down, 0, +1);
}

void action_resize_up(const XKeyEvent* key_event) {
    action_resize(inc_client_resize_step_up, 0, -1);
}

void action_resize_left(const XKeyEvent* key_event) {
    action_resize(inc_client_resize_step_left, -1, 0);
}

void action_resize_right(const XKeyEvent* key_event) {
    action_resize(inc_client_resize_step_right, +1, 0);
}

void action_toggle_fullscreen(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();

    if (focus == NULL)
        return;

    int workspace = get_client_on_workspace(focus);
    _Bool fullscreen = client_is_fullscreen(focus);
    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    Client* current_fullscreen = get_monitor_fullscreen(monitor);
    if (focus == NULL 
        || get_current_workspace() != reconcile_clients_workspace(focus)
        || current_fullscreen != NULL && focus != current_fullscreen
    ) return;

    client_set_fullscreen(focus, !fullscreen);

    if (!fullscreen) {
        set_monitor_fullscreen(monitor, focus);
        arrange_monitor(monitor, CurrentTime, NO_JUSTIFY_FOCUS);
        return;
    }

    set_monitor_fullscreen(monitor, NULL);
    arrange_monitor(monitor, CurrentTime, NO_JUSTIFY_FOCUS);
    raise_all_floaters(monitor);
}

void action_toggle_float(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    int workspace = reconcile_clients_workspace(focus);

    if (get_current_workspace() != workspace)
        return;

    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    if (focus == NULL 
        || monitor_fullscreen_exists(monitor)
    ) return; 

    FloatingClients* current_fl = get_monitor_fl(monitor); 
    _Bool floating = client_is_float(focus);

    if (!floating) {
        DetachedClient* detached = detach_client(focus);
        fl_push(get_monitor_fl(monitor), detached);
    }

    else {
        DetachedClient* detached = fl_find_from_client(current_fl, focus);
        fl_reattach_into_cl(current_fl, detached);
    }

    arrange_monitors(CurrentTime, JUSTIFY_FOCUS);
}

void action_toggle_floaters_focus(const XKeyEvent* key_event) {
    if (get_current_focus() == NULL) return;

    Client* focus = get_current_focus();
    int workspace = reconcile_clients_workspace(focus);

    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    if (get_current_workspace() != workspace)
        return;

    if (cl_find_client(get_monitor_cl(monitor), focus) 
        && !fl_empty(get_monitor_fl(monitor))
    ) {
        set_current_focus(
            get_client_from_detached(
                fl_head(get_monitor_fl(monitor))
            ), 
            key_event->time
        );
    }

    // Current focus is a floater
    else if (!cl_empty(get_monitor_cl(monitor))){
        Client* next_non_float = fs_next_non_floating(
            get_workspace_fs(workspace)
        );

        if (next_non_float == NULL) {
            Client* focus = focus_from_cl_head_or_tail(
                get_workspace_monitors(workspace),
                monitor,
                workspace
            );

            set_current_focus(focus, key_event->time);
            return;
        }
        set_current_focus(next_non_float, key_event->time);
    }
}

void action_switch_float_focus(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    int workspace = reconcile_clients_workspace(focus);
    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    if (get_current_workspace() != workspace 
        || monitor_fullscreen_exists(monitor)
    ) return;

    if (!client_is_float(focus) 
        || fl_size(get_monitor_fl(monitor)) < 2
    ) return;

    DetachedClient* next = fl_find_from_client(
        get_monitor_fl(monitor),
        focus
    )->next;

    next = next == NULL 
        ? fl_head(get_monitor_fl(monitor)) 
        : next;

    set_current_focus(get_client_from_detached(next), key_event->time);
}

void action_minimize_window(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    int workspace = reconcile_clients_workspace(focus);

    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    if (get_current_workspace() != workspace)
        return;

    if (focus == get_monitor_fullscreen(monitor)) return;

    if (!client_is_minimized(focus)) {
        MinimizedClient* minimized = minimize_client(focus);

        ml_push(get_monitor_ml(monitor), minimized);

        arrange_monitor(monitor, CurrentTime, NO_JUSTIFY_FOCUS);
        set_current_focus(get_next_focus(monitor, FS_REMOVE), key_event->time);
    }

    else {
        MinimizedClient* minimized = ml_find_from_client(
            get_monitor_ml(monitor),
            focus
        );

        ml_reattach(get_monitor_ml(monitor), minimized);
        arrange_monitor(monitor, CurrentTime, JUSTIFY_FOCUS);
    }
}

void action_toggle_minimize_focus(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    int workspace = reconcile_clients_workspace(focus);

    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    if (get_current_workspace() != workspace
        || ml_empty(get_monitor_ml(monitor))
        || monitor_fullscreen_exists(monitor)
    ) return;

    if (!client_is_minimized(focus)) {
        set_current_focus(
            get_client_from_minimized(
                ml_head(get_monitor_ml(monitor))
            ),
            key_event->time
        );
        return;
    }

    set_current_focus(get_next_focus(monitor, NO_FS_REMOVE), key_event->time);
}

void action_change_workspace_layout(Layout layout) {
    Client* focus = get_current_focus();
    if (focus == NULL) return;

    int workspace = reconcile_clients_workspace(focus);
    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    set_monitor_layout(monitor, layout);
    arrange_monitor(monitor, CurrentTime, JUSTIFY_FOCUS);
}

void action_tile_master_left(const XKeyEvent* key_event) {
    action_change_workspace_layout(ANGEL_MASTER_LEFT);
}

void action_tile_master_right(const XKeyEvent* key_event) {
    action_change_workspace_layout(ANGEL_MASTER_RIGHT);
}

void action_tile_simple_vertical(const XKeyEvent* key_event) {
    action_change_workspace_layout(ANGEL_SIMPLE_VERTICAL);
}

void action_tile_simple_horizontal(const XKeyEvent* key_event) {
    action_change_workspace_layout(ANGEL_SIMPLE_HORIZONTAL);
}

void action_tile_monocle(const XKeyEvent* key_event) {
    action_change_workspace_layout(ANGEL_MONOCLE);
}

void action_tile_master_left_monocle(const XKeyEvent* key_event) {
    action_change_workspace_layout(ANGEL_MASTER_LEFT_MONOCLE);
}

void action_tile_master_right_monocle(const XKeyEvent* key_event) {
    action_change_workspace_layout(ANGEL_MASTER_RIGHT_MONOCLE);
}

void action_tile_master_master_left(const XKeyEvent* key_event) {
    action_change_workspace_layout(ANGEL_MASTER_MASTER_LEFT);
}

void action_tile_master_master_right(const XKeyEvent* key_event) {
    action_change_workspace_layout(ANGEL_MASTER_MASTER_RIGHT);
}

void action_switch_workspace(int workspace) {
    if (workspace < 0 || workspace >= N_WORKSPACES 
        || workspace == get_current_workspace()
    ) return;

    int ws_before_switch = get_current_workspace();

    Client* focus = get_current_focus();
    Monitor* focus_monitor = reconcile_monitor_from_focus(
        ws_before_switch,
        focus
    );

    move_persistent_to_workspace(ws_before_switch, workspace);

    if (focus != NULL && get_client_on_workspace(focus) != ws_before_switch) {
        set_current_focus(
            get_next_focus(
                focus_monitor,
                FS_REMOVE
            ),
            CurrentTime
        );
    }

    set_focus_before_workspace_switch(
        ws_before_switch, get_current_focus()
    );

    workspace_set_all_unmapped_from_workspace_switch(
        ws_before_switch,
        true
    );

    workspace_cancel_cancel_pending_unmaps(ws_before_switch);
    workspace_cancel_pending_maps(ws_before_switch);
    unmap_current_workspace();

    set_current_workspace(workspace);
    ewmh_set_current_desktop(workspace);

    workspace_set_all_mapped_from_workspace_switch(
        workspace,
        true
    );

    workspace_cancel_cancel_pending_maps(workspace);
    workspace_cancel_pending_unmaps(workspace);
    map_current_workspace();

    arrange_monitors(CurrentTime, JUSTIFY_FOCUS);

    write_ws_state(workspace);
}

void action_move_to_workspace(int workspace, Time time) {
    if (workspace < 0 || workspace >= N_WORKSPACES) 
        return;

    Client* focus = get_current_focus();
    if (focus == NULL) return;

    Monitor* monitor = reconcile_monitor_from_focus(get_current_workspace(), focus);

    move_client_to_workspace(focus, workspace);
    client_set_moved(focus, true);
    ewmh_store_workspace_num(get_client_win(focus), (unsigned long) workspace);
    client_set_was_configured(focus, false);
    unmap_client(focus, MANAGE_CONT);
    set_current_focus(get_next_focus(monitor, FS_REMOVE), time);
}

void action_switch_to_workspace_one(const XKeyEvent* key_event) {
    if (get_current_workspace() == 0) return;
    action_switch_workspace(0);
}

void action_switch_to_workspace_two(const XKeyEvent* key_event) {
    if (get_current_workspace() == 1) return;
    action_switch_workspace(1);
}

void action_switch_to_workspace_three(const XKeyEvent* key_event) {
    if (get_current_workspace() == 2) return;
    action_switch_workspace(2);
}

void action_switch_to_workspace_four(const XKeyEvent* key_event) {
    if (get_current_workspace() == 3) return;
    action_switch_workspace(3);
}

void action_switch_to_workspace_five(const XKeyEvent* key_event) {
    if (get_current_workspace() == 4) return;
    action_switch_workspace(4);
}

void action_switch_to_workspace_six(const XKeyEvent* key_event) {
    if (get_current_workspace() == 5) return;
    action_switch_workspace(5);
}

void action_switch_to_workspace_seven(const XKeyEvent* key_event) {
    if (get_current_workspace() == 6) return;
    action_switch_workspace(6);
}

void action_switch_to_workspace_eight(const XKeyEvent* key_event) {
    if (get_current_workspace() == 7) return;
    action_switch_workspace(7);
}

void action_switch_to_workspace_nine(const XKeyEvent* key_event) {
    if (get_current_workspace() == 8) return;
    action_switch_workspace(8);
}

void action_switch_to_workspace_ten(const XKeyEvent* key_event) {
    if (get_current_workspace() == 9) return;
    action_switch_workspace(9);
}

void action_move_to_workspace_one(const XKeyEvent* key_event) {
    if (get_current_workspace() == 0) return;
    action_move_to_workspace(0, key_event->time);
}

void action_move_to_workspace_two(const XKeyEvent* key_event) {
    if (get_current_workspace() == 1) return;
    action_move_to_workspace(1, key_event->time);
}

void action_move_to_workspace_three(const XKeyEvent* key_event) {
    if (get_current_workspace() == 2) return;
    action_move_to_workspace(2, key_event->time);
}

void action_move_to_workspace_four(const XKeyEvent* key_event) {
    if (get_current_workspace() == 3) return;
    action_move_to_workspace(3, key_event->time);
}

void action_move_to_workspace_five(const XKeyEvent* key_event) {
    if (get_current_workspace() == 4) return;
    action_move_to_workspace(4, key_event->time);
}

void action_move_to_workspace_six(const XKeyEvent* key_event) {
    if (get_current_workspace() == 5) return;
    action_move_to_workspace(5, key_event->time);
}

void action_move_to_workspace_seven(const XKeyEvent* key_event) {
    if (get_current_workspace() == 6) return;
    action_move_to_workspace(6, key_event->time);
}

void action_move_to_workspace_eight(const XKeyEvent* key_event) {
    if (get_current_workspace() == 7) return;
    action_move_to_workspace(7, key_event->time);
}

void action_move_to_workspace_nine(const XKeyEvent* key_event) {
    if (get_current_workspace() == 8) return;
    action_move_to_workspace(8, key_event->time);
}

void action_move_to_workspace_ten(const XKeyEvent* key_event) {
    if (get_current_workspace() == 9) return;
    action_move_to_workspace(9, key_event->time);
}

void action_restart_manager(const XKeyEvent* key_event) {
    wm_state = WM_RESTART;
}

void action_quit_manager(const XKeyEvent* key_event) {
    wm_state = WM_QUIT;
}

void action_unmap_all_workspace_windows(const XKeyEvent* key_event) {
    monitors_cl_set_future_unmap_stay_unmapped(
        get_workspace_monitors(get_current_workspace())
    );

    unmap_current_workspace();
}

void action_map_all_workspace_windows(const XKeyEvent* key_event) {
    monitors_ul_set_all_can_be_mapped(
        get_workspace_monitors(get_current_workspace())
    );

    map_current_workspace();
}

void action_unmap_window(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    client_set_future_unmap_stay_unmapped(focus);
    unmap_client(focus, MANAGE_CONT);
}

void action_map_latest_unmap(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    Monitor* monitor = reconcile_monitor_from_focus(get_current_workspace(), focus);

    UnmappedClient* latest_unmap = ul_head(get_monitor_ul(monitor));
    if (latest_unmap == NULL) return;

    unmapped_set_stay_unmapped(latest_unmap, CAN_BE_MAPPED);
    Client* client = get_client_from_unmapped(latest_unmap);
    client_set_future_unmap_can_be_remapped(client);

    map_client(client);
}

void action_gap_dec(const XKeyEvent* key_event) {
    set_gap(max(get_gap()-get_gap_inc(),0));
    arrange_monitors(key_event->time, NO_JUSTIFY_FOCUS);
}

void action_gap_inc(const XKeyEvent* key_event) {
    set_gap(get_gap()+get_gap_inc());
    arrange_monitors(key_event->time, NO_JUSTIFY_FOCUS);
}

void action_float_all_tiled(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    if (focus == NULL) return;

    int workspace = reconcile_clients_workspace(focus);
    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    monitor_move_all_float_to_tiled(monitor);
    arrange_monitor(monitor, key_event->time, NO_JUSTIFY_FOCUS);
}

void action_tile_all_float(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    if (focus == NULL) return;

    int workspace = reconcile_clients_workspace(focus);
    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    monitor_move_all_tiled_to_foat(monitor);
    arrange_monitor(monitor, key_event->time, NO_JUSTIFY_FOCUS);
}

void action_minimize_all(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    if (focus == NULL) return;

    int workspace = reconcile_clients_workspace(focus);
    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    monitor_move_all_to_minimize(monitor);
    arrange_monitor(monitor, key_event->time, NO_JUSTIFY_FOCUS);
}

void action_unminimize_all(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    if (focus == NULL) return;

    int workspace = reconcile_clients_workspace(focus);
    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    monitor_reattach_all_minimized(monitor);
    arrange_monitor(monitor, key_event->time, NO_JUSTIFY_FOCUS);
}

void action_minimize_left(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    if (focus == NULL) return;

    int workspace = reconcile_clients_workspace(focus);
    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    set_monitor_minimized_position(monitor, MINIMIZED_LEFT);
    arrange_monitor(monitor, key_event->time, NO_JUSTIFY_FOCUS);
}

void action_minimize_right(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    if (focus == NULL) return;

    int workspace = reconcile_clients_workspace(focus);
    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    set_monitor_minimized_position(monitor, MINIMIZED_RIGHT);
    arrange_monitor(monitor, key_event->time, NO_JUSTIFY_FOCUS);
}

void action_minimize_top(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    if (focus == NULL) return;

    int workspace = reconcile_clients_workspace(focus);
    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    set_monitor_minimized_position(monitor, MINIMIZED_TOP);
    arrange_monitor(monitor, key_event->time, NO_JUSTIFY_FOCUS);
}

void action_minimize_bottom(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    if (focus == NULL) return;

    int workspace = reconcile_clients_workspace(focus);
    Monitor* monitor = reconcile_monitor_from_focus(workspace, focus);

    set_monitor_minimized_position(monitor, MINIMIZED_BOTTOM);
    arrange_monitor(monitor, key_event->time, NO_JUSTIFY_FOCUS);
}

void action_inc_minimized_size(const XKeyEvent* key_event) {
    set_minimized_height(get_minimized_height()+get_minimized_height_inc());
    arrange_monitors(key_event->time, NO_JUSTIFY_FOCUS);
}

void action_dec_minimized_size(const XKeyEvent* key_event) {
    set_minimized_height(max(get_minimized_height()-get_minimized_height_inc(),0));
    arrange_monitors(key_event->time, NO_JUSTIFY_FOCUS);
}

void action_move_win_monitor(GetAdjacentMonitorFn get_adjacent, Time time) {
    if (get_adjacent == NULL)
        return;

    Client* focus = get_current_focus();
    if (focus == NULL || client_is_float(focus)) return;

    Monitor* curr_monitor = reconcile_monitor_from_focus(
        get_current_workspace(),
        focus
    );

    _Bool was_fullscreen = get_monitor_fullscreen(curr_monitor) == focus;

    Monitor* move_to_monitor = get_adjacent(
        get_workspace_monitors(get_current_workspace()),
        curr_monitor
    );

    if (move_to_monitor == NULL || curr_monitor == move_to_monitor) 
        return;

    move_client_to_monitor(focus, curr_monitor, move_to_monitor);
    store_monitor_name_property(
        get_client_win(focus),
        get_monitor_name(move_to_monitor)
    );

    if (was_fullscreen) {
        set_monitor_fullscreen(curr_monitor, NULL);
        set_monitor_fullscreen(move_to_monitor, focus);
    }

    arrange_monitor(curr_monitor, time, NO_JUSTIFY_FOCUS);
    arrange_monitor(move_to_monitor, time, NO_JUSTIFY_FOCUS);
}

void action_move_win_monitor_down(const XKeyEvent* key_event) {
    action_move_win_monitor(
        get_monitor_below,
        key_event->time
    );
}

void action_move_win_monitor_up(const XKeyEvent* key_event) {
    action_move_win_monitor(
        get_monitor_above,
        key_event->time
    );
}

void action_move_win_monitor_left(const XKeyEvent* key_event) {
    action_move_win_monitor(
        get_monitor_to_left,
        key_event->time
    );
}

void action_move_win_monitor_right(const XKeyEvent* key_event) {
    action_move_win_monitor(
        get_monitor_to_right,
        key_event->time
    );
}

void action_change_focus_mode(FocusMode mode) {
    if (!focus_mode_valid(mode)) return;
    set_focus_mode(mode);
}

void action_change_to_focus_mode_focus(const XKeyEvent* key_event) {
    action_change_focus_mode(FOCUS_MODE_FOCUS);
}

void action_change_to_focus_mode_pointer(const XKeyEvent* key_event) {
    action_change_focus_mode(FOCUS_MODE_POINTER);
}

static void action_cycle_tiled(_Bool forward, Time time) {
    Client* focus = get_current_focus();
    if (focus == NULL) return;

    Monitor* focus_monitor = reconcile_monitor_from_focus(
        get_current_workspace(),
        focus
    );

    if (cl_find_client(get_monitor_cl(focus_monitor), focus) == NULL)
        return;

    if (forward) {
        Client* next = focus->next;
        if (next == NULL)
            next = cl_head(get_monitor_cl(focus_monitor));

        if (next != NULL && next != focus)
            set_current_focus(next, time);
    }

    else {
        Client* prev = cl_find_parent(get_monitor_cl(focus_monitor), focus);
        if (prev == NULL)
            prev = cl_tail(get_monitor_cl(focus_monitor));

        if (prev != NULL && prev != focus)
            set_current_focus(prev, time);
    }
}

void action_cycle_tiled_forward(const XKeyEvent* key_event) {
    action_cycle_tiled(true, key_event->time);
}

void action_cycle_tiled_backward(const XKeyEvent* key_event) {
    action_cycle_tiled(false, key_event->time);
}

void action_new_focus_start_adjacent(const XKeyEvent* key_event) {
    set_focus_start(FOCUS_START_ADJACENT);
}

void action_new_focus_start_end(const XKeyEvent* key_event) {
    set_focus_start(FOCUS_START_END);
}

void action_next_focus_on_close_use_stack(const XKeyEvent* key_event) {
    set_focus_end(FOCUS_END_FOCUS_STACK);
}

void action_next_focus_on_close_next(const XKeyEvent* key_event) {
    set_focus_end(FOCUS_END_NEXT);
}

void action_toggle_persistent(const XKeyEvent* key_event) {
    Client* focus = get_current_focus();
    if (focus == NULL) return;

    client_set_persistent(focus, !client_persistent(focus));
}

EventHandleFn get_event_handle_fn(EventType event_type) {
    for (int i = 0; i < ARRAY_SIZE(event_maps); ++i) {
        if (event_maps[i].type == event_type)
            return event_maps[i].handle_fn;
    }
    return handle_nothing;
}

void evloop() {
    while (wm_running()) {
        // write_ws_state(get_current_workspace());

        XEvent event;
        XNextEvent(dp, &event);
        EventHandleFn handle_fn = get_event_handle_fn (
            event.type
        );

        if (handle_fn != handle_nothing)
            handle_fn(&event);

        if (event.type == randr_bases.event_base + RRScreenChangeNotify)
            handle_screen_change_notify(&event);
        else if (event.type == randr_bases.event_base + RRNotify)
            handle_rr_notify(&event);
    }
}
