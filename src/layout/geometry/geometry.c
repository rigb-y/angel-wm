/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "geometry.h"
#include "client.h"
#include "client_list.h"
#include "motion_tree.h"
#include "workspaces.h"
#include "node.h"
#include "colors.h"
#include "logging.h"
#include "utils.h"
#include "types.h"
#include "windows.h"
#include "focus_stack.h"
#include "utils.h"
#include "detached.h"
#include "float_list.h"
#include "layouts.h"
#include "minimized_client.h"
#include "minimized_list.h"
#include "icccm.h"
#include "monitor.h"
#include "monitors.h"
#include "pointer.h"
#include "docks.h"
#include "windows.h"
#include "pointer.h"

#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define MASTER_LEFT true
#define MASTER_RIGHT false

#define SIMPLE_HORIZONTAL true
#define SIMPLE_VERTICAL false

#define STACK_RIGHT true
#define STACK_LEFT false

#define TOO_BIG 1
#define TOO_SMALL 2

#define MEANINGLESS_BOOL false

FullscreenGeometry fs_geometry = {0};
Geometry geometry = {0};
MonocleGeometry monocle_geometry = {0};

static void clear_layout_enter_events();

/* Master Geometry */
typedef struct MasterGeometry {
    Client* master;
    _Bool master_side;
    int master_width_no_border;
    int master_height_no_border;
    int width_remain;
    int height_remain;
} MasterGeometry;

static void set_master_geometry(MasterGeometry*, Client*, _Bool, int, int, int, int);
static MasterGeometry create_master_geometry(Client*, _Bool, int, int, int, int);

typedef struct ResizeSteps {
    int step_down, step_up;
    int step_left, step_right;
} ResizeSteps;

/* ResizeSteps statics */
static void set_resize_steps(ResizeSteps*, int, int, int, int);
static ResizeSteps create_resize_steps(int, int, int, int);

typedef struct Stackee {
    Client* client;
    int width_no_border;
    int height_no_border;
    unsigned int border_width;
    struct Stackee* next;
} Stackee;

static void set_stackee(Stackee*, Client*, int, int, unsigned int);
static Stackee* create_stackee(Client*, int, int, unsigned int);

/* Stackee Geometry */
typedef struct StackeeGeometry {
    Stackee* stackee_head;
    int n_clients;
    int screen_width_remainder;
    int screen_height_remainder;
    _Bool simple_direction;
} StackeeGeometry;

static void stackee_geometry_push(StackeeGeometry*, Client*, int, int, unsigned int);
static Stackee* get_stackee_n(const StackeeGeometry*, int);

static void set_stackee_geometry(StackeeGeometry*, _Bool, int, int, int);
static StackeeGeometry create_stackee_geometry(Client*, _Bool, int, int, int, int, int);
static _Bool stackee_valid_n(const StackeeGeometry*, int);
static void destroy_stackee_geometry(StackeeGeometry*);
static int get_stackee_width(const StackeeGeometry*, int);
static int get_adjusted_stackee_width(const StackeeGeometry*, int);
static int get_stackee_height(const StackeeGeometry*, int);
static int get_adjusted_stackee_height(const StackeeGeometry*, int);
static int get_stackee_screen_width_remainder(const StackeeGeometry*);
static int get_stackee_screen_height_remainder(const StackeeGeometry*);
static ResizeSteps get_stackee_max_geometry_after_horizontal_resize(Client*, _Bool);

static _Bool fast_tile(Monitor*, Client*, int, int, int, int);
static void tile_monocle_custom(Monitor*, Client*, SubTileFn, _Bool, int, int, int, int);
static void tile_noop_custom(Monitor*, Client*, SubTileFn, _Bool, int, int, int, int); 

/* Simple stack statics */
static void simple_tile(Monitor*, _Bool, Client*, int, int, int, int);
static StackeeGeometry get_simple_tile_geometry(_Bool, Client*, int, int);
static void simple_change_position(Position*, const Client*, _Bool);
static void simple_initial_default_configure(Monitor*, _Bool, Client*, int, int, int, int);
static int simple_get_min_geometry_of_stackees(const Client*, GeometryInfoFn);
static _Bool simple_adjust_on_resize_breaks_other_stackees(Monitor*, Client*, _Bool, int);
static int simple_get_vertical_resize(Monitor*, Client*, _Bool);
static int simple_get_horizontal_resize(Monitor*, Client*, _Bool);
static void simple_reconfigure_stackees(Monitor*, Client*, _Bool, int, int);
static void simple_adjust_stackees(Monitor*, _Bool, Client*);
static void simple_tile_vertical_custom(Monitor*, Client*, SubTileFn, _Bool, int, int, int, int);
static void simple_tile_horizontal_custom(Monitor*, Client*, SubTileFn, _Bool, int, int, int, int);
static int simple_get_resize_share_size(int, int, int);

static int stackee_hor_resize_breaks_master(Monitor*, Client*, const ResizeSteps*, int, _Bool);
static void update_resize_steps_on_master_break(ResizeSteps*, int, _Bool);
static int get_stackee_resize_for_master(ResizeSteps*, _Bool);

/* Master stack statics */
static void tile_master(Monitor*, _Bool);
static void tile_master_custom(Monitor*, Client*, SubTileFn, _Bool, int, int, int, int);
static MasterGeometry get_master_geometry(
    Monitor*,
    Client*,
    _Bool,
    int, int,
    ResizeSetStepFn,
    ResizeStepInfoFn,
    ResizeSetStepFn,
    ResizeStepInfoFn
);
static void configure_master_and_insert(Monitor*, const MasterGeometry*, int, int, int);
static int count_clients_to_right(const Client*);

static int get_true_width_for_minimized(int, int, int);

static _Bool width_hint_in_bounds(Monitor*, const Client*, int);
static _Bool height_hint_in_bounds(Monitor*, const Client*, int);
static int reconcile_floaters_width(Monitor*, const Client*, int);
static int reconcile_floaters_height(Monitor*, const Client*, int);
static int get_floater_base_width(const Client*);
static int get_floater_base_height(const Client*);
static int get_floater_preferred_width(Monitor*, const Client*);
static int get_floater_preferred_height(Monitor*, const Client*);

static _Bool geometry_out_of_bounds(Monitor*, int, int);
static _Bool police_gap(Monitor*);
static _Bool validate_border_width(int*);

static ReconcileMinimizedFn get_reconcile_minimized_fn(MinimizedPosition);
static void reconcile_minimized_top_or_bottom(Monitor*, int, int);
static void reconcile_minimized_left_or_right(Monitor*, int, int);
static void reconcile_minimized_bottom(Monitor*, int);
static void reconcile_minimized_top(Monitor*, int);
static void reconcile_minimized_left(Monitor*, int);
static void reconcile_minimized_right(Monitor*, int);

static void clamp_minimized_height_for_left_right(Monitor*, int);
static void clamp_minimized_height_for_top_bottom(Monitor*, int);
static void clamp_minimized_height(Monitor*, int);

static void reserve_space_for_nonparital_struts(const Strut*, int, int);
static void reserve_space_for_partial_struts(const Strut*, int, int);

void set_min_gap_while_policing(int gap) {
    if (gap < 0) return;
    geometry.min_gap_while_policing = (unsigned int) gap;
}

unsigned int get_min_gap_while_policing() {
    return geometry.min_gap_while_policing;
}

void set_fallback_minimized_position(MinimizedPosition minimized_position) {
    if (minimized_position != MINIMIZED_LEFT 
        && minimized_position != MINIMIZED_RIGHT
        && minimized_position != MINIMIZED_BOTTOM
        && minimized_position != MINIMIZED_TOP
    ) {
        lerr("Bad minimized position. Adjusting to bottom");
        geometry.fallback_minimized_position = MINIMIZED_BOTTOM;
        return;
    }
    geometry.fallback_minimized_position = minimized_position;
}

MinimizedPosition get_fallback_minimized_position() {
    return geometry.fallback_minimized_position;
}

void set_fs_geometry(int border_width, const char* border_color) {
    if (border_width < 0) border_width = 0;

    Color color = get_color(border_color);
    if (color.bad) {
        lerr("Bad fullscreen border color. Adjusting to green");
        color = get_color("green");
    }

    fs_geometry.border_width = border_width;
    fs_geometry.border_color = color;
}

unsigned int get_fs_border_width() {
    return fs_geometry.border_width;
}

Color get_fs_border_color() {
    return fs_geometry.border_color;
}

int get_fs_x(Monitor* monitor) {
    if (monitor == NULL) return 0;
    return monitor_x(monitor);
}

int get_fs_y(Monitor* monitor) {
    if (monitor == NULL) return 0;
    return monitor_y(monitor);
}

int get_fs_width(Monitor* monitor) {
    if (monitor == NULL) return 0;
    return monitor_width(monitor)
        - 2 * fs_geometry.border_width;
}

int get_fs_height(Monitor* monitor) {
    if (monitor == NULL) return 0;
    return monitor_height(monitor)
        - 2 * fs_geometry.border_width;
}

void set_monocle_space_between(int space) {
    if (space < 0) space = 0;
    monocle_geometry.space_between = space;
}

int get_monocle_space_between() {
    return monocle_geometry.space_between;
}

void set_master_geometry(
    MasterGeometry* mg,
    Client* master,
    _Bool master_side,
    int width,
    int height,
    int width_remain,
    int height_remain
) {
    mg->master = master;
    mg->master_side = master_side;
    mg->master_width_no_border = width;
    mg->master_height_no_border = height;
    mg->width_remain = width_remain;
    mg->height_remain = height_remain;
}

MasterGeometry create_master_geometry(
        Client* client,
        _Bool master_side,
        int width,
        int height,
        int width_remain,
        int height_remain
) {
    MasterGeometry mg = {0};
    set_master_geometry(
        &mg,
        client,
        master_side,
        width,
        height,
        width_remain,
        height_remain
    );
    return mg;
}

void set_stackee(Stackee* stackee, Client* client, int width_no_border, int height_no_border, unsigned int border_width) {
    if (stackee == NULL || client == NULL) return;
    stackee->width_no_border = width_no_border;
    stackee->height_no_border = height_no_border;
    stackee->border_width = border_width;
    stackee->client = client;
    stackee->next = NULL;
}

Stackee* create_stackee(Client* client, int width_no_border, int height_no_border, unsigned int border_width) {
    Stackee* stackee;
    if ((stackee = calloc(1, sizeof(Stackee))) == NULL)
        return NULL;

    set_stackee(stackee, client, width_no_border, height_no_border, border_width);
    return stackee;
}

void stackee_geometry_push(StackeeGeometry* sg, Client* client, int width_no_border, int height_no_border, unsigned int border_width) {
    if (client == NULL) return;

    Stackee* stackee;
    if ((stackee = create_stackee(client, width_no_border, height_no_border, border_width)) == NULL)
        return;

    if (sg->stackee_head == NULL) {
        sg->stackee_head = stackee;
        sg->stackee_head->next = NULL;
        return;
    }

    Stackee* curr = sg->stackee_head;
    while (curr->next != NULL)
        curr = curr->next;

    curr->next = stackee;
    stackee->next = NULL;
}

_Bool stackee_valid_n(const StackeeGeometry* sg, int n) {
    return sg != NULL && n > 0 && n <= sg->n_clients;
}

Stackee* get_stackee_n(const StackeeGeometry* sg, int n) {
    if (sg == NULL || !stackee_valid_n(sg, n)) return NULL;

    int k = 1;
    Stackee* curr = sg->stackee_head;
    while (curr != NULL && k++ != n)
        curr=curr->next;

    return curr;
}

void set_stackee_geometry(
    StackeeGeometry* sg,
    _Bool simple_direction,
    int n_clients, 
    int width_remainder,
    int height_remainder
) {
    if (sg == NULL) return;
    sg->n_clients = n_clients;
    sg->simple_direction = simple_direction;
    sg->screen_width_remainder = width_remainder;
    sg->screen_height_remainder = height_remainder;
}

StackeeGeometry create_stackee_geometry(
    Client* start,
    _Bool simple_direction,
    int n_clients,
    int usable_width,
    int usable_height,
    int width_remainder,
    int height_remainder
) {
    StackeeGeometry sg = {0};
    set_stackee_geometry(
        &sg, simple_direction, n_clients,
        width_remainder, height_remainder
    );

    int raw_width = usable_width / n_clients;
    int raw_height = usable_height / n_clients;
    int gap = get_gap();
    while (start != NULL) {
        int border_width = get_border_width(start);

        int width, height;
        if (simple_direction == SIMPLE_HORIZONTAL) {
            width = raw_width - gap - 2*border_width;
            height = usable_height - gap - 2*border_width;
        }

        else {
            width = usable_width - gap - 2*border_width;
            height = raw_height - gap - 2*border_width;
        }

        stackee_geometry_push(&sg, start, width, height, border_width);
        start = start->next;
    }

    return sg;
}

void destroy_stackee_geometry(StackeeGeometry* sg) {
    if (sg == NULL) return;
    Stackee* curr_stackee = sg->stackee_head;
    while (curr_stackee != NULL) {
        Stackee* tmp = curr_stackee;
        free(tmp);
        curr_stackee = curr_stackee->next;
    }
    sg->stackee_head = NULL;
}

int get_stackee_width(const StackeeGeometry* sg, int n) {
    if (sg == NULL || !stackee_valid_n(sg, n)) return 0;

    Stackee* stackee_n = get_stackee_n(sg, n);
    return stackee_n != NULL ? stackee_n->width_no_border : 0;
}

int get_adjusted_stackee_width(const StackeeGeometry* sg, int client_number) {
    if (sg == NULL || !stackee_valid_n(sg, client_number)) return 0;

    return get_stackee_width(sg, client_number) 
        + (client_number <= get_stackee_screen_width_remainder(sg) ? 1 : 0);
}

int get_stackee_height(const StackeeGeometry* sg, int n) {
    if (sg == NULL) return 0;

    Stackee* stackee_n = get_stackee_n(sg, n);
    return stackee_n != NULL ? stackee_n->height_no_border : 0;
}

int get_adjusted_stackee_height(const StackeeGeometry* sg, int client_number) {
    if (sg == NULL || !stackee_valid_n(sg, client_number)) return 0;

    int number = get_stackee_height(sg, client_number) 
        + (client_number <= get_stackee_screen_height_remainder(sg) ? 1 : 0);

    return number;
}

int get_stackee_screen_width_remainder(const StackeeGeometry* sg) {
    if (sg == NULL) return 0;
    return sg->screen_width_remainder;
}

int get_stackee_screen_height_remainder(const StackeeGeometry* sg) {
    if (sg == NULL) return 0;
    return sg->screen_height_remainder;
}

ResizeSteps get_stackee_max_geometry_after_horizontal_resize(Client* start, _Bool direction) {
    int max_left = 0, max_right = 0;
    while (start != NULL) {
        max_left = max(max_left, get_client_resize_step_left(start));
        max_right = max(max_right, get_client_resize_step_right(start));

        start = start->next;
    }

    return create_resize_steps(0, 0, max_left, max_right);
}

void set_position(Position* position, int x, int y) {
    if (position == NULL) return;
    position->x = x, position->y = y;
}

Position create_position(int x, int y) {
    Position pos;
    set_position(&pos, x, y);
    return pos;
}

int pos_x(const Position* position) {
    if (position == NULL) return 0;
    return position->x;
}

int pos_y(const Position* position) {
    if (position == NULL) return 0;
    return position->y;
}

void set_pos_x(Position* position, int x) {
    if (position == NULL) return;
    position->x = x;
}

void set_pos_y(Position* position, int y) {
    if (position == NULL) return;
    position->y = y;
}

void add_to_pos_x(Position* position, int x) {
    if (position == NULL) return;
    set_pos_x(position, pos_x(position) + x);
}

void sub_from_pos_x(Position* position, int x) {
    if (position == NULL) return;
    add_to_pos_y(position, -x);
}

void add_to_pos_y(Position* position, int y) {
    if (position == NULL) return;
    set_pos_y(position, pos_y(position) + y);
}

void sub_from_pos_y(Position* position, int y) {
    if (position == NULL) return;
    add_to_pos_y(position, -y);
}

int get_max_width(Monitor* monitor) {
    return 0.9 * monitor_width(monitor);
}

int get_max_height(Monitor* monitor) {
    return 0.9 * monitor_height(monitor);
}

int get_min_width() {
    return 20;
}

int get_min_height() {
    return 20;
}

int get_min_floater_width(Monitor* monitor, const Client* client) {
    int raw_min = get_min_width();
    int min_hint = get_min_height_hint(get_client_win(client));

    return width_hint_in_bounds(monitor, client, min_hint) ? min_hint : raw_min;
}

int get_min_floater_height(Monitor* monitor, const Client* client) {
    int raw_min = get_min_height();
    int min_hint = get_min_height_hint(get_client_win(client));

    return height_hint_in_bounds(monitor, client, min_hint) ? min_hint : raw_min;
}

int get_max_floater_width(Monitor* monitor, const Client* client) {
    int raw_max = monitor_width(monitor) 
        - (get_gap()<<1) 
        - (get_border_width(client)<<1);

    int max_hint = get_max_height_hint(get_client_win(client));

    return width_hint_in_bounds(monitor, client, max_hint) ? max_hint : raw_max;
}

int get_max_floater_height(Monitor* monitor, const Client* client) {
    int raw_max = monitor_height(monitor) 
        - (get_gap()<<1) 
        - (get_border_width(client)<<1);

    int max_hint = get_max_height_hint(get_client_win(client));

    return height_hint_in_bounds(monitor, client, max_hint) ? max_hint : raw_max;
}

Color get_border_color(const Client* client) {
    if (client_is_fullscreen(client)) {
        return get_fs_border_color();
    }

    if (get_client_in_resize_mode(client) 
        && !client_is_minimized(client)
    ) {
        return get_resize_border_color();
    }

    if (client != get_current_focus()) {
        return get_unfocused_border_color();
    }

    if (client_is_float(client)) {
        return get_float_border_color();
    }

    return get_focused_border_color();
}

unsigned int get_border_width(const Client* client) {
    if (client_is_fullscreen(client))
        return get_fs_border_width();

    if (client_is_minimized(client))
        return get_minimized_border_width();

    if (get_client_in_resize_mode(client))
        return get_resize_border_width();

    if (in_monocle_stack(
            get_monitor_from_list_membership(
                get_workspace_monitors(get_current_workspace()),
                client
            ),
            client
        )
    ) {
        return get_monocle_border_width();
    }

    if (client_is_float(client))
        return get_float_border_width();

    if (client != get_current_focus())
        return get_unfocused_border_width();

    return get_focused_border_width();
}

unsigned int get_unfocused_border_width() {
    return geometry.unfocused_border_width;
}

unsigned int get_focused_border_width() {
    return geometry.focused_border_width;
}

unsigned int get_resize_border_width() {
    return geometry.resize_border_width;
}

unsigned int get_float_border_width() {
    return geometry.float_border_width;
}

unsigned int get_minimized_border_width() {
    return geometry.minimized_border_width;
}

unsigned int get_monocle_border_width() {
    return geometry.monocle_border_width;
}

void set_unfocused_border_width(int border_width) {
    validate_border_width(&border_width);
    geometry.unfocused_border_width = (unsigned int) border_width; 
}

void set_focused_border_width(int border_width) {
    validate_border_width(&border_width);
    geometry.focused_border_width = (unsigned int) border_width; 
}

void set_resize_border_width(int border_width) {
    validate_border_width(&border_width);
    geometry.resize_border_width = (unsigned int) border_width; 
}

void set_float_border_width(int border_width) {
    validate_border_width(&border_width);
    geometry.float_border_width = (unsigned int) border_width; 
}

void set_minimized_border_width(int border_width) {
    validate_border_width(&border_width);
    geometry.minimized_border_width = (unsigned int) border_width; 
}

void set_monocle_border_width(int border_width) {
    validate_border_width(&border_width);
    geometry.monocle_border_width = (unsigned int) border_width; 
}

_Bool validate_border_width(int* bw) {
    if (*bw >= 0) return true;

    *bw = 0;
    lerr("Bad border width");
    return false;
}

void set_unfocused_border_color(const char* spec) {
    Color color = get_color(spec);
    if (color.bad) {
        lerr("Bad unfocused window border color. Adjusting to orange");
        color = get_color("orange");
    }
    geometry.unfocused_border_color = color;
}

Color get_unfocused_border_color() {
    return geometry.unfocused_border_color;
}

void set_focused_border_color(const char* spec) {
    Color color = get_color(spec);
    if (color.bad) {
        lerr("Bad focused window border color. Adjusting to green");
        color = get_color("green");
    }
    geometry.focused_border_color = color;
}

Color get_focused_border_color() {
    return geometry.focused_border_color;
}

void set_resize_border_color(const char* spec) {
    Color color = get_color(spec);
    if (color.bad) {
        lerr("Bad resize window border color. Adjusting to red");
        color = get_color("red");
    }
    geometry.resize_border_color = color;
}

Color get_resize_border_color() {
    return geometry.resize_border_color;
}

void set_float_border_color(const char* spec) {
    Color color = get_color(spec);
    if (color.bad) {
        lerr("Bad floating window border color. Adjusting to blue");
        color = get_color("blue");
    }
    geometry.float_border_color = color;
}

Color get_float_border_color() {
    return geometry.float_border_color;
}

void set_gap(int gap) {
    if (gap < 0) {
        lerr("Bad gap size");
        return;
    }

    int modifier = gap >= get_gap() ? 1 : -1;

    if ((gap & 1) != 0) gap = (gap + 1 * modifier);
    geometry.gap = (unsigned int) gap;
}

void set_gap_inc(int inc) {
    if (inc < 0) {
        lerr("Bad gap increment.");
        return;
    }

    geometry.gap_inc = (unsigned int) inc;
}

unsigned int get_gap_inc() {
    return geometry.gap_inc;
}

unsigned int get_gap() {
    return geometry.gap;
}

void set_window_resize_inc(int inc) {
    if (inc < 0) {
        lerr("Bad resize increment");
        return;
    }
    geometry.window_resize_inc = (unsigned int) inc;
}

unsigned int get_window_resize_inc() {
    return geometry.window_resize_inc;
}

unsigned int get_window_width_resize_inc(Window win) {
    int hint = get_width_inc_hint(win);
    return hint > 0 ? hint : geometry.window_resize_inc; 
}

void set_minimized_height_inc(int inc) {
    if (inc < 0) {
        lerr("Bad resize increment");
        return;
    }
    geometry.minimized_height_inc = (unsigned int) inc;
}

unsigned int get_minimized_height_inc() {
    return geometry.minimized_height_inc;
}

unsigned int get_window_height_resize_inc(Window win) {
    int hint = get_height_inc_hint(win);
    return hint > 0 ? hint : geometry.window_resize_inc; 
}

void set_floating_window_keyboard_movement_step(int inc) {
    if (inc < 0) {
        lerr("Bad resize increment");
        return;
    }
    geometry.floating_window_movement_step = inc;
}

int get_floating_window_keyboard_movement_step() {
    return geometry.floating_window_movement_step;
}

void set_minimized_height(int h) {
    geometry.minimized_height = h;
}

int get_minimized_height() {
    return geometry.minimized_height;
}

void configure_client(Monitor* monitor, Client* client, int x, int y, int w, int h) {
    if (client == NULL || !is_client_mapped(client)) return;

    XWindowChanges changes;
    if (client_is_fullscreen(client)) {
        changes.x = get_fs_x(monitor);
        changes.y = get_fs_y(monitor);
        changes.width = get_fs_width(monitor);
        changes.height = get_fs_height(monitor);
        changes.border_width = get_fs_border_width();
        set_client_border(client, changes.border_width, get_fs_border_color());
        raise_window(get_client_win(client));
    }

    else {
        changes.x = x;
        changes.y = y;
        changes.width = w;
        changes.height = h;
        changes.border_width = get_border_width(client);

        set_client_border(
            client,
            changes.border_width,
            get_border_color(client)
        );
    }

    unsigned int mask = CWX | CWY | CWWidth | CWHeight | CWBorderWidth;

    XConfigureWindow(dp, get_client_win(client), mask, &changes);

    set_client_position(client, changes.x, changes.y);
    set_client_geometry(
        client,
        changes.width,
        changes.height,
        changes.border_width
    );

    client_set_was_configured(client, true);
}

void move_client(Client* client, int x, int y) {
    if (client == NULL) return;
    XMoveWindow(dp, get_client_win(client), x, y);
    set_client_position(client, x, y);
}

void resize_client(Client* client, int w, int h) {
    if (client == NULL) return;
    XResizeWindow(dp, get_client_win(client), w, h);
    set_client_geometry(client, w, h, get_border_width(client));
}

void arrange_monitors(Time time, _Bool should_justify_focus) {
    Monitors* monitors = get_workspace_monitors(get_current_workspace());
    Monitor* monitor = monitors_head(monitors);

    if (monitor == NULL) 
        return;

    while (monitor->next != NULL) {
        arrange_monitor(monitor, time, NO_JUSTIFY_FOCUS);
        monitor = monitor->next;
    }

    arrange_monitor(monitor, time, should_justify_focus);
}

void clear_layout_enter_events() {
    XEvent ev;
    XSync(dp, false);
    while (XCheckMaskEvent(dp, EnterNotify, &ev));

    Position pp = get_pointer_pos();
    pp_set_xy(pos_x(&pp), pos_y(&pp));
}

void arrange_monitor(Monitor* monitor, Time time, _Bool should_justify_focus) {
    if (monitor == NULL) return;

    update_monitor_usable_height(monitor, monitor_height(monitor));
    update_monitor_usable_width(monitor, monitor_width(monitor));
    update_monitor_start_x(monitor, monitor_x(monitor));
    update_monitor_start_y(monitor, monitor_y(monitor));

    reconcile_struts(monitor);
    reconcile_minimized(monitor);

    get_layout_fn(
        get_monitor_layout(monitor, get_current_workspace())
    )(monitor);

    reconcile_floaters(monitor);

    clear_layout_enter_events();

    if (should_justify_focus == JUSTIFY_FOCUS)
        justify_focus(monitor, NULL, time);
}

_Bool width_hint_in_bounds(Monitor* monitor, const Client* client, int width) {
    if (monitor == NULL || client == NULL) return false;

    return width > 0 && width <= monitor_width(monitor) 
        - 2*get_gap() - 2*get_border_width(client);
}

_Bool height_hint_in_bounds(Monitor* monitor, const Client* client, int height) {
    if (monitor == NULL || client == NULL) return false;

    return height > 0 && height <= monitor_height(monitor) 
        - 2*get_gap() - 2*get_border_width(client);
}

int reconcile_floaters_width(Monitor* monitor, const Client* client, int width) {
    if (client == NULL) return width;

    int min_width = get_min_width_hint(get_client_win(client));
    int max_width = get_max_width_hint(get_client_win(client));

    if (width < min_width && width_hint_in_bounds(monitor, client, min_width)) 
        return min_width;

    if (width > max_width && width_hint_in_bounds(monitor, client, max_width)) 
        return max_width;
    
    return width;
}

int reconcile_floaters_height(Monitor* monitor, const Client* client, int height) {
    if (client == NULL) return height;

    int min_height = get_min_height_hint(get_client_win(client));
    int max_height = get_max_height_hint(get_client_win(client));

    if (height < min_height && height_hint_in_bounds(monitor, client, min_height)) 
        return min_height;

    if (height > max_height && height_hint_in_bounds(monitor, client, max_height)) 
        return max_height;

    return height;
}

int get_floater_base_width(const Client* client) {
    if (client == NULL) return 0;

    return get_base_width_hint(get_client_win(client));
}

int get_floater_base_height(const Client* client) {
    if (client == NULL) return 0;

    return get_base_height_hint(get_client_win(client));
}

int get_floater_preferred_width(Monitor* monitor, const Client* client) {
    if (client == NULL) return monitor_width(monitor) / 2;

    int hint = get_width_hint(get_client_win(client));
    return width_hint_in_bounds(monitor, client, hint) 
        ? hint : monitor_width(monitor) / 2;
}

int get_floater_preferred_height(Monitor* monitor, const Client* client) {
    if (client == NULL) return monitor_height(monitor) / 2;

    int hint = get_height_hint(get_client_win(client));
    return height_hint_in_bounds(monitor, client, hint) 
        ? hint : monitor_height(monitor) / 2;
}

void reconcile_floaters(Monitor* monitor) {
    DetachedClient* curr = fl_head(get_monitor_fl(monitor));
    while (curr != NULL) {
        Client* client = get_client_from_detached(curr);
        int width = get_floater_preferred_width(monitor, client);
        int height = get_floater_preferred_height(monitor, client);

        int x, y, clamped_width, clamped_height;
        if (get_detached_configured(curr)) {
            x = get_detached_x(curr);
            y = get_detached_y(curr);
            clamped_width = get_detached_width(curr);
            clamped_height = get_detached_height(curr);
        }

        else {
            x = monitor_x(monitor) + monitor_width(monitor) / 4;
            y = monitor_y(monitor) + monitor_height(monitor) / 4;
            clamped_width = reconcile_floaters_width(monitor, client, width);
            clamped_height = reconcile_floaters_height(monitor, client, height);
        }


        if (!floater_in_bounds(monitor, curr, x, y, clamped_width, clamped_height)) {
            x = get_gap();
            y = get_gap();
        }

        configure_client(
            monitor,
            client,
            x, y,
            clamped_width, clamped_height
        );

        set_detached_x(curr, x);
        set_detached_y(curr, y);
        set_detached_width(curr, clamped_width);
        set_detached_height(curr, clamped_height);

        set_detached_configured(curr, true);

        curr=curr->next;
    }

    raise_all_floaters(monitor);
}

void raise_all_floaters(Monitor* monitor) {
    DetachedClient* start = fl_head(get_monitor_fl(monitor));
    while (start != NULL) {
        raise_window(get_client_win(get_client_from_detached(start)));
        start=start->next;
    }

    if (monitor_fullscreen_exists(monitor)) {
        Client* fullscreen_client = get_monitor_fullscreen(monitor);
        if (is_client_mapped(fullscreen_client))
            raise_window(get_client_win(fullscreen_client));
    }
}

void move_floater(Monitor* monitor, DetachedClient* detached, int dx, int dy) {
    if (detached == NULL) return;

    Client* client = get_client_from_detached(detached);
    int xp = get_client_x(client) + dx;
    int yp = get_client_y(client) + dy;

    if (!floater_in_bounds(
        monitor,
        detached,
        xp, yp,
        get_client_width(client),
        get_client_height(client)
    )) return;

    move_client(
        client,
        xp, yp
    );

    set_client_position(client, xp, yp);

    set_detached_x(detached, xp);
    set_detached_y(detached, yp);

    clear_layout_enter_events();
}

void resize_floater(Monitor* monitor, DetachedClient* detached, int dw, int dh) {
    Client* client = get_client_from_detached(detached);

    int width = get_client_width(client);
    int height = get_client_height(client);
    int new_width = width + dw;
    int new_height = height + dh;

    int base_width = get_base_width_hint(get_client_win(client));
    int base_height = get_base_height_hint(get_client_win(client));

    int width_resize_inc = get_width_inc_hint(
        get_client_win(client)
    );

    int height_resize_inc = get_height_inc_hint(
        get_client_win(client)
    );

    if (base_width > 0 && width_resize_inc > 0) {
        new_width = base_width 
            + (new_width-base_width)/width_resize_inc 
            * width_resize_inc;
    }

    if (base_height > 0 && height_resize_inc > 0) {
        new_height = base_height 
            + (new_height-base_height) /height_resize_inc 
            * height_resize_inc;
    }

    if (new_width < get_min_floater_width(monitor, client) 
        || new_width > get_max_floater_width(monitor, client) 
        || new_height < get_min_floater_height(monitor, client) 
        || new_height > get_max_floater_height(monitor, client)
    ) return;

    if (!floater_in_bounds(
        monitor,
        detached,
        get_client_x(client),
        get_client_y(client),
        new_width,
        new_height
    )) return;

    resize_client(
        client,
        new_width,
        new_height
    );

    set_client_geometry(client, new_width, new_height, get_border_width(client));
    set_detached_width(detached, new_width);
    set_detached_height(detached, new_height);

    clear_layout_enter_events();
}

int get_true_width_for_minimized(int usable_width, int index, int n) {
    int q = usable_width / n;
    int r = usable_width % n;

    if (r > 0 && index <= r) return q+1;
    return q;
}

void reconcile_minimized_top_or_bottom(Monitor* monitor, int y, int space_between) {
    int cx = monitor_start_x(monitor) + get_gap();

    int each_height = get_minimized_height() 
        - 2*get_minimized_border_width() - space_between;
    int usable_width = monitor_usable_width(monitor) - 2*get_gap();

    MinimizedList* ml = get_monitor_ml(monitor);
    MinimizedClient* curr = ml_head(ml);
    while (curr != NULL) {
        Client* client = get_client_from_minimized(curr);

        int width = get_true_width_for_minimized(
            usable_width,
            ml_get_client_position(ml, curr),
            ml_size(ml)
        ) - 2*get_border_width(client);

        configure_client(monitor, client, cx, y, width, each_height);

        cx += width + (get_border_width(client)<<1);
        curr = curr->next;
    }

    update_monitor_usable_height(
        monitor,
        monitor_usable_height(monitor) 
        - get_minimized_height()
        - space_between
    );
}

void reconcile_minimized_bottom(Monitor* monitor, int space_between) {
    int y = monitor_start_y(monitor) + monitor_usable_height(monitor) 
        - get_gap() + (get_gap() != 0 ? space_between : 0)
        - get_minimized_height();

    reconcile_minimized_top_or_bottom(
        monitor, y, space_between
    );
}

void reconcile_minimized_top(Monitor* monitor, int space_between) {
    int y = monitor_start_y(monitor) + get_gap(); 

    update_monitor_start_y(
        monitor, 
        monitor_start_y(monitor)
        + get_minimized_height()
        + space_between
    );

    reconcile_minimized_top_or_bottom(
        monitor, y, space_between
    );
}

void reconcile_minimized_left_or_right(Monitor* monitor, int x, int space_between) { 
    int cy = monitor_start_y(monitor) + get_gap();

    int each_width = get_minimized_height() 
        - 2*get_minimized_border_width() - space_between;

    int usable_height = monitor_usable_height(monitor) - 2*get_gap(); 

    MinimizedList* ml = get_monitor_ml(monitor);
    MinimizedClient* curr = ml_head(ml);
    while (curr != NULL) {
        Client* client = get_client_from_minimized(curr);

        int height = get_true_width_for_minimized(
            usable_height,
            ml_get_client_position(ml, curr),
            ml_size(ml)
        ) - 2*get_border_width(client);

        configure_client(monitor, client, x, cy, each_width, height);

        cy += height + (get_border_width(client)<<1);
        curr = curr->next;
    }

    update_monitor_usable_width(
        monitor,
        monitor_usable_width(monitor) 
        - (each_width
        + 2*get_minimized_border_width())
    );
}

void reconcile_minimized_left(Monitor* monitor, int space_between) {
    int x = monitor_start_x(monitor) + get_gap(); 

    update_monitor_start_x(
        monitor,
        monitor_start_x(monitor)
        + get_minimized_height()
        + space_between
    );

    reconcile_minimized_left_or_right(
        monitor, x, space_between
    );
}

void reconcile_minimized_right(Monitor* monitor, int space_between) {
    int x = monitor_start_x(monitor) + monitor_usable_width(monitor) 
        - get_gap() + (get_gap() != 0 ? space_between : 0) 
        - get_minimized_height(); 

    reconcile_minimized_left_or_right(
        monitor, x, space_between
    );
}

ReconcileMinimizedFn get_reconcile_minimized_fn(MinimizedPosition mpos) {
    switch (mpos) {
        case MINIMIZED_BOTTOM:
            return reconcile_minimized_bottom;
        case MINIMIZED_TOP:
            return reconcile_minimized_top;
        case MINIMIZED_LEFT:
            return reconcile_minimized_left;
        case MINIMIZED_RIGHT:
            return reconcile_minimized_right;
        default:
            return NULL;
    }
}

void clamp_minimized_height_for_left_right(Monitor* monitor, int sb) {
    set_minimized_height(max(get_minimized_height(), 2*get_minimized_border_width() + sb + 5));
    set_minimized_height(
        min(
            get_minimized_height(),
            0.90 * monitor_usable_width(monitor)
        )
    );
}

void clamp_minimized_height_for_top_bottom(Monitor* monitor, int sb) {
    set_minimized_height(max(get_minimized_height(), 2*get_minimized_border_width() + sb + 5));
    set_minimized_height(
        min(
            get_minimized_height(),
            0.90 * monitor_usable_height(monitor)
        )
    );
}

void clamp_minimized_height(Monitor* monitor, int sb) {
    switch (get_minimized_position(monitor)) {
        case MINIMIZED_TOP:
        case MINIMIZED_BOTTOM:
            clamp_minimized_height_for_top_bottom(monitor, sb);
            return;
        case MINIMIZED_LEFT:
        case MINIMIZED_RIGHT:
            clamp_minimized_height_for_left_right(monitor, sb);
            return;
        default:
            return;
    }
}

void reconcile_minimized(Monitor* monitor) {
    if (monitor == NULL) return;

    int space_between = 2;
    clamp_minimized_height(monitor, space_between);

    if (ml_empty(get_monitor_ml(monitor))) {
        return;
    }

    get_reconcile_minimized_fn(
        get_minimized_position(monitor)
    )(monitor, space_between);
}

void reserve_space_for_nonparital_struts(const Strut* strut, int root_w, int root_h) {
    if (strut == NULL) return;

    int x = 0, y = 0, w = 0, h = 0;
    if (strut->top != 0) {
        x = 0, y =0;
        w = root_w;
        h = strut->top;

        adjust_monitors_for_strut(
            get_workspace_monitors(get_current_workspace()),
            STRUT_SIDE_TOP,
            x,y,
            w,h
        );
    }

    if (strut->bottom != 0) {
        x = 0, 
        y = root_h - strut->bottom;
        w = root_w;
        h = strut->bottom;

        adjust_monitors_for_strut(
            get_workspace_monitors(get_current_workspace()),
            STRUT_SIDE_BOTTOM,
            x,y,
            w,h
        );
    }

    if (strut->left != 0) {
        x = 0, y = 0;
        w = strut->left;
        h = root_h;

        adjust_monitors_for_strut(
            get_workspace_monitors(get_current_workspace()),
            STRUT_SIDE_LEFT,
            x,y,
            w,h
        );
    }

    if (strut->right != 0) {
        x = root_w - strut->right;
        y = 0;
        w = strut->right;
        h = root_h;

        adjust_monitors_for_strut(
            get_workspace_monitors(get_current_workspace()),
            STRUT_SIDE_RIGHT,
            x,y,
            w,h
        );
    }
}

void reserve_space_for_partial_struts(const Strut* strut, int root_w, int root_h) {
    if (strut == NULL) return;

    int x = 0, y = 0, w = 0, h = 0;
    if (strut->top != 0) {
        x = (int)strut->top_start_x;
        y = 0;
        w = (int)strut->top_end_x - x + 1;
        h = strut->top;

        adjust_monitors_for_strut(
            get_workspace_monitors(get_current_workspace()),
            STRUT_SIDE_TOP,
            x,y,
            w,h
        );
    }

    if (strut->bottom != 0) {
        x = (int)strut->bottom_start_x;
        y = root_h - (int)strut->bottom;
        w = (int)strut->bottom_end_x - x + 1;
        h = strut->bottom;

        adjust_monitors_for_strut(
            get_workspace_monitors(get_current_workspace()),
            STRUT_SIDE_BOTTOM,
            x,y,
            w,h
        );
    }

    if (strut->left != 0) {
        x = 0;
        y = (int)strut->left_start_y;
        w = (int)strut->left;
        h = (int)strut->left_end_y - y + 1;

        adjust_monitors_for_strut(
            get_workspace_monitors(get_current_workspace()),
            STRUT_SIDE_LEFT,
            x,y,
            w,h
        );
    }

    if (strut->right != 0) {
        x = root_w - (int)strut->right;
        y = (int)strut->right_start_y;
        w = (int)strut->right;
        h = (int)strut->right_end_y - y + 1;

        adjust_monitors_for_strut(
            get_workspace_monitors(get_current_workspace()),
            STRUT_SIDE_RIGHT,
            x,y,
            w,h
        );
    }
}

void reconcile_struts(Monitor* monitor) {
    if (monitor == NULL) return;

    XWindowAttributes* root_attrs = get_win_attrs(root);
    if (root_attrs == NULL)
        return;

    int root_w = root_attrs->width;
    int root_h = root_attrs->height;

    XFree(root_attrs);
    root_attrs = NULL;

    Docks* docks = get_monitor_dl(monitor);
    if (docks_empty(docks))
        return;

    Dock* curr = docks->head;
    while (curr != NULL) {
        if (!dock_strut_valid(curr)) {
            curr = curr->next;
            continue;
        }

        const Strut* strut = get_dock_strut(curr);

        if (!dock_has_strut_partial(curr))
            reserve_space_for_nonparital_struts(strut, root_w, root_h);
        else
            reserve_space_for_partial_struts(strut, root_w, root_h);

        curr = curr->next;
    }
}

void set_resize_steps(ResizeSteps* rs, int d, int u, int l, int r) {
    rs->step_down = d, rs->step_up = u;
    rs->step_left = l, rs->step_right = r;
}

ResizeSteps create_resize_steps(int d, int u, int l, int r) {
    ResizeSteps steps = {0};
    set_resize_steps(&steps, d, u, l, r);
    return steps;
}

_Bool geometry_out_of_bounds(Monitor* monitor, int width, int height) {
    int gap = get_gap();
    return width > (monitor_usable_width(monitor) - (2*gap)) 
        || width < get_min_width()
        || height > (monitor_usable_height(monitor) - (2*gap)) 
        || height < get_min_height();
}

_Bool police_gap(Monitor* monitor) {
    Client* curr = cl_head(get_monitor_cl(monitor));

    int gap_change = 0;
    while (curr != NULL && get_gap() >= get_min_gap_while_policing()) {
        int width = get_client_width(curr) + 2*gap_change;
        int height = get_client_height(curr) + 2*gap_change;
        while (!client_is_fullscreen(curr) 
                && get_gap() >= get_min_gap_while_policing()
                && geometry_out_of_bounds(monitor, width, height)
        ) {
            set_gap(max(get_gap()-2, 0));

            gap_change+=2;
            width += 2;
            height += 2;
        }

        curr = curr->next;
    }

    if (gap_change == 0) return false;

    curr = cl_head(get_monitor_cl(monitor));
    while (curr != NULL) {
        set_client_geometry(
            curr,
            get_client_width(curr) + 2*gap_change,
            get_client_height(curr) + 2*gap_change,
            get_border_width(curr)
        );
        curr = curr->next;
    }

    return true;
}

_Bool fast_tile(
    Monitor* monitor,
    Client* start,
    int start_x,
    int start_y,
    int width_available,
    int height_available
) {
    if (start == NULL) return true;

    if (start->next == NULL) {
        Client* alone = start;

        unsigned int border_width = get_border_width(alone);
        Color unfocused_border_color = get_unfocused_border_color();
        Color focused_border_color = get_focused_border_color();
        unsigned int gap = get_gap();

        int initial_x = start_x + gap;
        int initial_y = start_y + gap;

        int screen_width = width_available - gap;
        int screen_height = height_available;

        int client_width = screen_width - (gap + 2*border_width);
        int client_height = screen_height - (2*gap + 2*border_width);

        set_client_position(alone, initial_x, initial_y);
        set_client_geometry(alone, client_width, client_height, border_width);

        configure_client(
            monitor,
            alone,
            initial_x, initial_y,
            client_width, client_height
        ); 

        MTNode* node = mtnode_create(alone);
        mt_insert_below_of(
            get_monitor_mt(monitor),
            mt_root(get_monitor_mt(monitor)),
            node
        );

        return true;
    }

    return false;
}

StackeeGeometry get_simple_tile_geometry(_Bool simple_direction, Client* start, int width_available, int height_available) {
    int usable_screen_width = width_available - get_gap();
    int usable_screen_height = height_available - get_gap();
    int client_count = count_clients_to_right(start) + 1;

    int width_remainder = 0, height_remainder = 0;
    if (simple_direction == SIMPLE_HORIZONTAL) {
        width_remainder = usable_screen_width % client_count;
    }

    else {
        height_remainder = 
            (usable_screen_height % client_count);
    }

    return create_stackee_geometry(
        start,
        simple_direction,
        client_count,
        usable_screen_width,
        usable_screen_height,
        width_remainder,
        height_remainder
    );
}

void simple_change_position(Position* curr_pos, const Client* c, _Bool simple_direction) {
    if (curr_pos == NULL) return;

    if (simple_direction == SIMPLE_HORIZONTAL) {
        add_to_pos_x(
            curr_pos,
            get_client_width(c) 
            + 2*get_border_width(c) 
            + get_gap()
        ); 
    }

    else {
        add_to_pos_y(
            curr_pos,
            get_client_height(c) 
            + 2*get_border_width(c) 
            + get_gap()
        ); 
    }
}

void simple_initial_default_configure(
    Monitor* monitor,
    _Bool simple_direction,
    Client* start,
    int start_x,
    int start_y,
    int width_available,
    int height_available
) {
    StackeeGeometry each_geometry = get_simple_tile_geometry(
        simple_direction,
        start,
        width_available, 
        height_available
    );

    Position curr_pos = create_position(
        start_x + get_gap(), 
        start_y + get_gap()
    );

    MTNode* curr_parent = mt_root(get_monitor_mt(monitor));

    Client* c = start;
    while (c != NULL) {
        set_client_position(c, pos_x(&curr_pos), pos_y(&curr_pos));
        set_client_geometry(
            c,
            get_adjusted_stackee_width(
                &each_geometry,
                cl_client_distance(
                    get_monitor_cl(monitor), start, c
                ) + 1
            ),
            get_adjusted_stackee_height(
                &each_geometry,
                cl_client_distance(
                    get_monitor_cl(monitor), start, c
                ) + 1
            ),
            get_border_width(c)
        );

        simple_change_position(&curr_pos, c, simple_direction);

        MTNode* curr_mt_node = mtnode_create(c);
        mt_insert_below_of(get_monitor_mt(monitor), curr_parent, curr_mt_node);

        if (simple_direction == SIMPLE_VERTICAL) {
            curr_parent = curr_mt_node;
        }

        c=c->next;
    }

    destroy_stackee_geometry(&each_geometry);
}

int simple_get_min_geometry_of_stackees(const Client* start, GeometryInfoFn get_geometry) {
    if (start == NULL) return 0;

    int minimum = get_geometry(start);

    Client* next = start->next;
    while (next != NULL) {
        minimum = min(minimum, get_geometry(next));
        next=next->next;
    }

    return minimum;
}

_Bool simple_adjust_on_resize_breaks_other_stackees(Monitor* monitor, Client* c, _Bool simple_direction, int inc) {
    if (c == NULL || inc == 0) return false;

    int n = count_clients_to_right(c);
    Client* start = c;
    while (start != NULL) {
        int share = simple_get_resize_share_size(
            inc, 
            cl_client_distance(
                get_monitor_cl(monitor),
                c,
                start
            ), 
            count_clients_to_right(c)
        );

        int current_geometry = simple_direction == SIMPLE_HORIZONTAL 
            ? get_client_width(start) 
            : get_client_height(start);

        int updated = current_geometry - share;

        int min_geometry = simple_direction == SIMPLE_HORIZONTAL 
            ? get_min_width() 
            : get_min_height();

        int max_geometry = simple_direction == SIMPLE_HORIZONTAL 
            ? get_max_width(monitor) 
            : get_max_height(monitor);

        if (updated < min_geometry) {
            if (simple_direction == SIMPLE_HORIZONTAL) {
                set_client_resize_step_right(c, get_client_resize_step_right(c)-1);
            }

            else {
                set_client_resize_step_down(c, get_client_resize_step_down(c)-1);
            }

            return true;
        }
        
        else if (updated > max_geometry) {
            if (simple_direction == SIMPLE_HORIZONTAL) {
                set_client_resize_step_left(c, get_client_resize_step_left(c)-1);
            }

            else  {
                set_client_resize_step_up(c, get_client_resize_step_up(c)-1);
            }

            return true;
        }

        start = start->next;
    } 

    return false;
}

int simple_get_vertical_resize(Monitor* monitor, Client* c, _Bool simple_direction) {
    if (c == NULL || simple_direction == SIMPLE_HORIZONTAL) return 0;

    int height_inc = get_window_resize_inc() * 
        (get_client_resize_step_down(c) - get_client_resize_step_up(c)); 

    if (get_client_height(c) + height_inc > get_max_height(monitor) && get_client_resize_step_down(c) > 0) {
        set_client_resize_step_down(c, get_client_resize_step_down(c)-1);
        return simple_get_vertical_resize(monitor, c, simple_direction);
    }

    if (get_client_height(c) + height_inc < get_min_height() && get_client_resize_step_up(c) > 0) {
        set_client_resize_step_up(c, get_client_resize_step_up(c)-1);
        return simple_get_vertical_resize(monitor, c, simple_direction);
    }

    return height_inc;
}

int simple_get_horizontal_resize(Monitor* monitor, Client* c, _Bool simple_direction) {
    if (c == NULL || simple_direction == SIMPLE_VERTICAL) return 0;

    int width_inc = get_window_resize_inc() 
        * (get_client_resize_step_right(c) - get_client_resize_step_left(c)); 

    if (get_client_width(c) + width_inc > get_max_width(monitor) && get_client_resize_step_right(c) > 0) {
        set_client_resize_step_right(c, get_client_resize_step_right(c)-1);

        return simple_get_horizontal_resize(monitor, c, simple_direction);
    }

    if (get_client_width(c) + width_inc < get_min_width() && get_client_resize_step_left(c) > 0) {
        set_client_resize_step_left(c, get_client_resize_step_left(c)-1);

        return simple_get_horizontal_resize(monitor, c, simple_direction);
    }

    return width_inc;
}

int simple_get_resize_share_size(int inc, int index, int n) {
    if (n == 0) return 0;

    int q = inc / n;
    int r = inc % n;

    if (r > 0 && index <= r) return q + 1;
    if (r < 0 && index <= -r) return q - 1;

    return q;
}

int stackee_hor_resize_breaks_master(Monitor* monitor, Client* client, const ResizeSteps* rs, int master_width, _Bool master_side) {
    if (rs == NULL) return 0;

    int resize_inc = get_window_resize_inc();
    if (master_side == MASTER_LEFT) {
        if (master_width - resize_inc 
            * (rs->step_left - rs->step_right) > get_max_width(monitor)
        ) {
            return TOO_BIG; 
        }

        if (master_width - resize_inc 
            * (rs->step_left - rs->step_right) < get_min_width()
        ) {
            return TOO_SMALL; 
        }
    }

    else {
        if (master_width - resize_inc 
            * (rs->step_right - rs->step_left) > get_max_width(monitor)
        ) {
            return TOO_BIG; 
        }

        if (master_width - resize_inc 
            * (rs->step_right - rs->step_left) < get_min_width()
        ) {
            return TOO_SMALL; 
        }
    }

    return 0;
}

void simple_reconfigure_stackees(Monitor* monitor, Client* start, _Bool simple_direction, int inc, int n) {
    if (start == NULL) return; 

    int first_share = simple_get_resize_share_size(
        inc, cl_client_distance(
            get_monitor_cl(monitor),
            start,
            start
        )+1, n
    );

    Client* curr = start;
    int prev_delta = inc + first_share;
    int prev_share = first_share;
    while (curr != NULL) {
        int share = simple_get_resize_share_size(
            inc, cl_client_distance(
                get_monitor_cl(monitor),
                start,
                curr
            )+1, n
        );

        int delta = prev_delta - prev_share;
        prev_delta = delta;
        prev_share = share;

        int new_x = get_client_x(curr), new_y = get_client_y(curr);
        int new_width = get_client_width(curr), new_height = get_client_height(curr);

        if (simple_direction == SIMPLE_HORIZONTAL) {
            new_x = get_client_x(curr) + delta;
            new_width = get_client_width(curr) - share;
        }

        else {
            new_y = get_client_y(curr) + delta;
            new_height = get_client_height(curr) - share;
        }

        set_client_position(curr, new_x, new_y);
        set_client_geometry(curr, new_width, new_height, get_border_width(curr));

        curr=curr->next;
    }
}

void simple_adjust_stackees(Monitor* monitor, _Bool simple_direction, Client* start) {
    if (start == NULL) return;

    Client* c = start;
    while (c->next != NULL) {
        int width_inc = simple_get_horizontal_resize(monitor, c, simple_direction);
        int height_inc = simple_get_vertical_resize(monitor, c, simple_direction);

        int inc = simple_direction == SIMPLE_HORIZONTAL ? width_inc : height_inc;

        while (simple_adjust_on_resize_breaks_other_stackees(monitor, c, simple_direction, inc)) {
            if (simple_direction == SIMPLE_HORIZONTAL) {
                width_inc = simple_get_horizontal_resize(monitor, c, simple_direction);
            }

            else {
                height_inc = simple_get_vertical_resize(monitor, c, simple_direction);
            }

            inc = simple_direction == SIMPLE_HORIZONTAL ? width_inc : height_inc;
        }

        int adjusted_width = get_client_width(c) + width_inc;
        int adjusted_height = get_client_height(c) + height_inc;

        simple_reconfigure_stackees(monitor, c->next, simple_direction, inc, count_clients_to_right(c));

        set_client_position(c, get_client_x(c), get_client_y(c));
        set_client_geometry(c, adjusted_width, adjusted_height, get_border_width(c));

        c = c->next;
    }
}

void simple_tile(
    Monitor* monitor,
    _Bool simple_direction, 
    Client* start,
    int start_x, int start_y,
    int width_available, 
    int height_available
) {
    if (fast_tile(
        monitor,
        start,
        start_x,
        start_y,
        width_available,
        height_available
    )) return;

    // Configures windows but doesn't tell X to configure them
    simple_initial_default_configure(
        monitor,
        simple_direction,
        start,
        start_x,
        start_y,
        width_available,
        height_available
    );

    simple_adjust_stackees(monitor, simple_direction, start);

    Client* c = start;
    while (c != NULL) {
        // Geometry for each client is settled by the two calls above
        configure_client(
            monitor,
            c,
            get_client_x(c),
            get_client_y(c),
            get_client_width(c),
            get_client_height(c)
        );
        c = c->next;
    }
}

void simple_tile_horizontal(Monitor* monitor) {
    mt_destroy(get_monitor_mt(monitor));
    mt_init(get_monitor_mt(monitor));

    simple_tile(
        monitor,
        SIMPLE_HORIZONTAL,
        cl_head(get_monitor_cl(monitor)),
        monitor_start_x(monitor),
        monitor_start_y(monitor),
        monitor_usable_width(monitor),
        monitor_usable_height(monitor)
    );

    if (police_gap(monitor)) {
        arrange_monitor(monitor, CurrentTime, JUSTIFY_FOCUS);
        return;
    }
}

void simple_tile_vertical(Monitor* monitor) {
    mt_destroy(get_monitor_mt(monitor));
    mt_init(get_monitor_mt(monitor));

    simple_tile(
        monitor,
        SIMPLE_VERTICAL,
        cl_head(get_monitor_cl(monitor)),
        monitor_start_x(monitor),
        monitor_start_y(monitor),
        monitor_usable_width(monitor),
        monitor_usable_height(monitor)
    );

    if (police_gap(monitor)) {
        arrange_monitor(monitor, CurrentTime, JUSTIFY_FOCUS);
        return;
    }
}

void simple_tile_horizontal_custom(
    Monitor* monitor,
    Client* start,
    SubTileFn unused_fn,
    _Bool unused_bool,
    int start_x,
    int start_y,
    int width_available,
    int height_available
) {
    (void)unused_bool;
    (void)unused_fn;
    simple_tile(
        monitor,
        SIMPLE_HORIZONTAL,
        start,
        start_x,
        start_y,
        width_available,
        height_available
    );
}

void simple_tile_vertical_custom(
    Monitor* monitor,
    Client* start,
    SubTileFn unused_fn,
    _Bool unused_bool,
    int start_x,
    int start_y,
    int width_available,
    int height_available
) {
    (void)unused_bool;
    (void)unused_fn;
    simple_tile(
        monitor,
        SIMPLE_VERTICAL,
        start,
        start_x,
        start_y,
        width_available,
        height_available
    );
}

void update_resize_steps_on_master_break(ResizeSteps* rs, int break_status, _Bool master_side) {
    if (rs == NULL) return;

    if (master_side == MASTER_LEFT) {
        if (break_status == TOO_SMALL) {
            --rs->step_left;
        }

        else {
            ++rs->step_left;
        }
    } 

    if (master_side == MASTER_RIGHT) {
        if (break_status == TOO_SMALL) {
            --rs->step_right;
        }
        
        else {
            ++rs->step_right;
        }
    }
}

int get_stackee_resize_for_master(ResizeSteps* rs, _Bool master_side) {
    if (rs == NULL) return 0;

    int resize_inc = get_window_resize_inc();
    return master_side == MASTER_LEFT 
        ? resize_inc * (rs->step_left - rs->step_right)
        : resize_inc * (rs->step_right - rs->step_left);
}

MasterGeometry get_master_geometry(
    Monitor* monitor,
    Client* master,
    _Bool master_side,
    int width_available,
    int height_available,
    ResizeSetStepFn step_change_on_too_big,
    ResizeStepInfoFn too_big_step_info,
    ResizeSetStepFn step_change_on_too_small,
    ResizeStepInfoFn too_small_step_info
) {
    int master_increase_right = get_client_resize_step_right(master) 
        * get_window_resize_inc();

    int master_increase_left = get_client_resize_step_left(master) 
        * get_window_resize_inc();

    int master_width_no_border = width_available/2 
        - (get_gap() 
        + 2*get_border_width(master));

    int resize_inc = get_window_resize_inc();

    if (master_side == MASTER_LEFT) 
        master_width_no_border = master_width_no_border 
            - master_increase_left 
            + master_increase_right;

    else 
        master_width_no_border = master_width_no_border 
            - master_increase_right
            + master_increase_left; 

    if (master_width_no_border > get_max_width(monitor)) {
        master_width_no_border -= resize_inc;

        if (master_side == MASTER_LEFT)
            master_increase_right -= resize_inc;
        else
            master_increase_left -= resize_inc;

        step_change_on_too_big(master, too_big_step_info(master)-1);
    }

    if (master_width_no_border < get_min_width()) {
        master_width_no_border += resize_inc;

        if (master_side == MASTER_LEFT)
            master_increase_left -= resize_inc;
        else
            master_increase_right -= resize_inc;

        step_change_on_too_small(master, too_small_step_info(master)-1);
    }

    Client* next_client = master->next;
    ResizeSteps max_resize_steps = 
        get_stackee_max_geometry_after_horizontal_resize(
            next_client,
            master_side
        );

    int break_status = 0;
    while (
        (break_status = 
            stackee_hor_resize_breaks_master(
                monitor,
                master,
                &max_resize_steps,
                master_width_no_border,
                master_side
            )
        ) != 0
    ) {
        update_resize_steps_on_master_break(
            &max_resize_steps,
            break_status,
            master_side
        );
    }

    cl_set_clients_resize_step(
        next_client,
        max_resize_steps.step_left,
        set_client_resize_step_left
    );

    cl_set_clients_resize_step(
        next_client,
        max_resize_steps.step_right,
        set_client_resize_step_right
    );

    int stackee_resize = 
        get_stackee_resize_for_master(
            &max_resize_steps,
            master_side
        );

    master_width_no_border -= stackee_resize;

    int master_height_no_border 
        = height_available 
        - (2*get_gap() 
        + 2*get_border_width(master));

    int width_remain = width_available / 2;
    if (master_side == MASTER_LEFT)
        width_remain = width_remain 
            - master_increase_right 
            + master_increase_left 
            + stackee_resize;
    else 
        width_remain = width_remain 
            + master_increase_right 
            - master_increase_left 
            + stackee_resize;

    int height_remain = 
        height_available 
        - get_gap(); 
    
    MasterGeometry mg = {0};
    set_master_geometry(
        &mg,
        master,
        master_side,
        master_width_no_border,
        master_height_no_border,
        width_remain,
        height_remain
    );

    return mg;
}

void configure_master_and_insert(
    Monitor* monitor,
    const MasterGeometry* mg,
    int start_x, int start_y,
    int width_available
) {
    if (mg == NULL) return;

    int master_x = start_x;
    int master_y = start_y + get_gap();

    if (mg->master_side == MASTER_LEFT)
        master_x = start_x + get_gap();

    else
        master_x = start_x + width_available 
            - mg->master_width_no_border
            - get_gap() 
            - 2*get_border_width(mg->master);

    configure_client(
        monitor,
        mg->master,
        master_x, master_y,
        mg->master_width_no_border,
        mg->master_height_no_border
    );

    MotionTree* mt = get_monitor_mt(monitor);
    mt_insert_below_of(
        mt,
        mt_root(mt),
        mtnode_create(mg->master)
    );
}

void tile_master_custom(
    Monitor* monitor,
    Client* start,
    SubTileFn tile_remaining,
    _Bool master_left,
    int start_x,
    int start_y,
    int width_available,
    int height_available
) {
    if (start == NULL) return;

    if (fast_tile(
        monitor,
        start,
        start_x,
        start_y,
        width_available,
        height_available
    )) return;

    Client* master = start;

    if (master_left == MASTER_LEFT) {
        // This call will make all stackee step requests uniform.
        MasterGeometry master_geometry = get_master_geometry(
            monitor,
            master, master_left,
            width_available,
            height_available,
            set_client_resize_step_right, get_client_resize_step_right,
            set_client_resize_step_left, get_client_resize_step_left
        );

        configure_master_and_insert(
            monitor,
            &master_geometry,
            start_x, start_y,
            width_available
        );

        int stack_start_x = 
            get_client_x(master) 
            + 2*get_border_width(master) 
            + master_geometry.master_width_no_border
            + get_gap();
        int stack_start_y = get_client_y(master);

        tile_remaining.fn(
            monitor,
            master->next, 
            (SubTileFn) {.fn = simple_tile_vertical_custom},
            tile_remaining.master_direction,
            stack_start_x - get_gap(),
            stack_start_y - get_gap(),
            master_geometry.width_remain, 
            master_geometry.height_remain + get_gap()
        );
    }

    else {
        MasterGeometry master_geometry = get_master_geometry(
            monitor,
            master, master_left,
            width_available, height_available,
            set_client_resize_step_left, get_client_resize_step_left,
            set_client_resize_step_right, get_client_resize_step_right
        );

        int stack_start_x = start_x + get_gap();
        int stack_start_y = start_y + get_gap();

        tile_remaining.fn(
            monitor,
            master->next, 
            (SubTileFn) {.fn = simple_tile_vertical_custom},
            tile_remaining.master_direction,
            stack_start_x - get_gap(),
            stack_start_y - get_gap(),
            master_geometry.width_remain,
            master_geometry.height_remain + get_gap()
        );

        configure_master_and_insert(
            monitor,
            &master_geometry,
            start_x, start_y,
            width_available
        );
    }
}

void tile_master(Monitor* monitor, _Bool master_left) {
    tile_master_custom(
        monitor,
        cl_head(get_monitor_cl(monitor)),
        (SubTileFn) {.fn = simple_tile_vertical_custom},
        master_left, 
        monitor_start_x(monitor),
        monitor_start_y(monitor),
        monitor_usable_width(monitor),
        monitor_usable_height(monitor)
    );
}

void tile_master_left(Monitor* monitor) {
    mt_destroy(get_monitor_mt(monitor));
    mt_init(get_monitor_mt(monitor));

    tile_master(monitor, MASTER_LEFT);

    if (police_gap(monitor)) {
        arrange_monitor(monitor, CurrentTime, JUSTIFY_FOCUS);
        return;
    }
}

void tile_master_right(Monitor* monitor) {
    mt_destroy(get_monitor_mt(monitor));
    mt_init(get_monitor_mt(monitor));

    tile_master(monitor, MASTER_RIGHT);

    if (police_gap(monitor)) {
        arrange_monitor(monitor, CurrentTime, JUSTIFY_FOCUS);
        return;
    }
}

void tile_monocle_custom(
    Monitor* monitor,
    Client* start,
    SubTileFn unused_fn,
    _Bool unused_bool,
    int start_x,
    int start_y,
    int width_available,
    int height_available
) {
    (void)unused_bool;
    (void)unused_fn;
    if (fast_tile(
        monitor,
        start,
        start_x,
        start_y,
        width_available,
        height_available
    )) return;

    MotionTree* mt = get_monitor_mt(monitor);

    Client* curr = start;
    MTNode* curr_parent = mt_root(mt);

    int cx = start_x + get_gap(), cy = start_y + get_gap();
    int cw = width_available - 2*get_gap() - 2*get_border_width(curr);
    int ch = height_available - 2*get_gap() - 2*get_border_width(curr);
    while (curr != NULL) {
        set_client_position(curr, cx, cy);
        set_client_geometry(curr, cw, ch, get_monocle_border_width());

        cy += get_monocle_space_between();
        ch -= get_monocle_space_between();

        MTNode* mtnode = mtnode_create(curr);
        mt_insert_below_of(mt, curr_parent, mtnode);
        curr_parent = mtnode;

        curr=curr->next;
    }

    curr = start;
    while (curr != NULL) {
        configure_client(
            monitor,
            curr,
            get_client_x(curr),
            get_client_y(curr),
            get_client_width(curr),
            get_client_height(curr)
        );

        curr = curr->next;
    }
}

void tile_monocle(Monitor* monitor) {
    mt_destroy(get_monitor_mt(monitor));
    mt_init(get_monitor_mt(monitor));

    tile_monocle_custom(
        monitor,
        cl_head(get_monitor_cl(monitor)),
        (SubTileFn) {.fn = tile_noop_custom},
        MEANINGLESS_BOOL,
        monitor_start_x(monitor),
        monitor_start_y(monitor),
        monitor_usable_width(monitor),
        monitor_usable_height(monitor)
    );

    if (police_gap(monitor)) {
        arrange_monitor(monitor, CurrentTime, JUSTIFY_FOCUS);
        return;
    }
}

void tile_master_left_monocle(Monitor* monitor) {
    mt_destroy(get_monitor_mt(monitor));
    mt_init(get_monitor_mt(monitor));

    tile_master_custom(
        monitor,
        cl_head(get_monitor_cl(monitor)),
        (SubTileFn) {.fn = tile_monocle_custom},
        MASTER_LEFT,
        monitor_start_x(monitor),
        monitor_start_y(monitor),
        monitor_usable_width(monitor),
        monitor_usable_height(monitor)
    );

    if (police_gap(monitor)) {
        arrange_monitor(monitor, CurrentTime, JUSTIFY_FOCUS);
        return;
    }
}

void tile_master_right_monocle(Monitor* monitor) {
    mt_destroy(get_monitor_mt(monitor));
    mt_init(get_monitor_mt(monitor));

    tile_master_custom(
        monitor,
        cl_head(get_monitor_cl(monitor)),
        (SubTileFn) {.fn = tile_monocle_custom},
        MASTER_RIGHT,
        monitor_start_x(monitor),
        monitor_start_y(monitor),
        monitor_usable_width(monitor),
        monitor_usable_height(monitor)
    );

    if (police_gap(monitor)) {
        arrange_monitor(monitor, CurrentTime, JUSTIFY_FOCUS);
        return;
    }
}

void tile_master_master_left(Monitor* monitor) {
    mt_destroy(get_monitor_mt(monitor));
    mt_init(get_monitor_mt(monitor));

    tile_master_custom(
        monitor,
        cl_head(get_monitor_cl(monitor)),
        (SubTileFn) {.fn = tile_master_custom, .master=true, .master_direction=MASTER_LEFT},
        MASTER_LEFT,
        monitor_start_x(monitor),
        monitor_start_y(monitor),
        monitor_usable_width(monitor),
        monitor_usable_height(monitor)
    );

    if (police_gap(monitor)) {
        arrange_monitor(monitor, CurrentTime, JUSTIFY_FOCUS);
        return;
    }
}

void tile_master_master_right(Monitor* monitor) {
    mt_destroy(get_monitor_mt(monitor));
    mt_init(get_monitor_mt(monitor));

    tile_master_custom(
        monitor,
        cl_head(get_monitor_cl(monitor)),
        (SubTileFn) {.fn = tile_master_custom, .master=true, .master_direction=MASTER_RIGHT},
        MASTER_RIGHT,
        monitor_start_x(monitor),
        monitor_start_y(monitor),
        monitor_usable_width(monitor),
        monitor_usable_height(monitor)
    );

    if (police_gap(monitor)) {
        arrange_monitor(monitor, CurrentTime, JUSTIFY_FOCUS);
        return;
    }
}

void tile_noop_custom(
    Monitor* monitor,
    Client* start,
    SubTileFn tile_remaining,
    _Bool unused,
    int start_x,
    int start_y,
    int width_available,
    int height_available
) { 
    (void)monitor;
    (void)start;
    (void)tile_remaining;
    (void)unused;
    (void)start_x;
    (void)start_y;
    (void)width_available;
    (void)height_available;
}

void tile_noop(Monitor* monitor) { (void)monitor; }

int count_clients_to_right(const Client* start) {
    if (start == NULL) return 0;

    Client* first = start->next;
    int n = 0; while (first != NULL) {
        ++n;
        first=first->next; 
    }

    return n;
}

_Bool floater_in_bounds(Monitor* monitor, DetachedClient* detached, int new_x, int new_y, int new_width, int new_height) {
    int workspace = get_client_on_workspace(
        get_client_from_detached(detached)
    );

    Monitors* monitors = get_workspace_monitors(workspace);

    int border_width = get_border_width(get_client_from_detached(detached));
    int max_width = monitor_width(monitor);
    int max_height = monitor_height(monitor);

    int x_lb = monitors_get_smallest_x(monitors);
    int y_lb = monitors_get_smallest_y(monitors);

    int x_rb = monitors_get_largest_x(monitors);
    int y_rb = monitors_get_largest_y(monitors);

    int x_tl = new_x;
    int x_tr = new_x + new_width + (border_width<<1);
    int x_bl = new_x;
    int x_br = new_x + new_width + (border_width<<1);
        
    int y_tl = new_y;
    int y_tr = new_y;
    int y_bl = new_y + new_height + (border_width<<1);
    int y_br = new_y + new_height + (border_width<<1);

    int max_x_tl = x_lb;
    int max_x_tr = x_rb;
    int max_x_bl = x_lb;
    int max_x_br = x_rb;
        
    int max_y_tl = y_lb;
    int max_y_tr = y_lb;
    int max_y_bl = y_rb;
    int max_y_br = y_rb;

    _Bool a = x_tl >= max_x_tl && y_tl >= max_y_tl; 
    _Bool b = x_tr <= max_x_tr && y_tr >= max_y_tr;
    _Bool c = x_bl >= max_x_bl && y_bl <= max_y_bl;
    _Bool d = x_br <= max_x_br && y_br <= max_y_br;

    return a && b && c && d;
}

_Bool in_monocle_stack(Monitor* monitor, const Client* client) {
    if (client == NULL) return false;

    switch (get_monitor_layout(monitor, get_client_on_workspace(client))) {
        case ANGEL_MONOCLE:
            return true;
        case ANGEL_MASTER_LEFT_MONOCLE:
        case ANGEL_MASTER_RIGHT_MONOCLE:
            return cl_get_client_position(
                get_monitor_cl(monitor), client
            ) > 1;
        default:
            return false;
    }
}

Monitor* get_primary_monitor_or_fallback(Monitors* monitors) {
    Monitor* primary = get_primary_monitor(monitors);
    if (primary != NULL)
        return primary;

    int pp_x = pp_get_x();
    int pp_y = pp_get_y();

    return get_monitor_from_position(monitors, pp_x, pp_y);
}

MinimizedPosition get_minimized_position(Monitor* monitor) {
    if (monitor == NULL) return MINIMIZED_POSITION_UNKNOWN;

    MinimizedPosition mpos = monitor_minimized_position(monitor);
    return mpos == MINIMIZED_POSITION_UNKNOWN 
        ? get_fallback_minimized_position() 
        : mpos;
}
