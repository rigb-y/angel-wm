/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "defaults.h"
#include "logging.h"
#include "layouts.h"
#include "colors.h"
#include "geometry.h"
#include "cursors.h"
#include "terminal.h"
#include "windows.h"
#include "workspaces.h"
#include "types.h"
#include "monitors.h"
#include "manage.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

DefaultConfig defaults = {0};

static _Bool validate_border_width(int*);
static void scatter_defaults();

void set_override_parameters(
    OverrideParameters* op,
    int n,
    Layout layout,
    Cursor cursor,
    const char* term_name,
    const char* color_spec
) {
    if (op == NULL) return;
    op->n = n;
    op->layout = layout;
    op->cursor = cursor;
    op->term_name = term_name;
    op->color_spec = color_spec;

    op->manage_term_name = false;
    op->manage_color_spec = false;

    op->definition_type = DT_UNKNOWN;
}

OverrideParameters create_override_parameters() {
    OverrideParameters op = {0};
    set_override_parameters(
        &op,
        -1, 
        ANGEL_LAYOUT_UNKNOWN,
        (Cursor)-1,
        NULL, ""
    );

    return op;
}

void destroy_override_parameters(OverrideParameters* op) {
    if (op == NULL) return; 

    if (op->manage_term_name) free((char*)op->term_name);
    if (op->manage_term_name) free((char*)op->color_spec);

    op->n = -1;
    op->layout = ANGEL_LAYOUT_UNKNOWN;
    op->cursor = (Cursor)-1;
    op->term_name = NULL;
    op->color_spec = "";

    op->manage_term_name = false;
    op->manage_color_spec = false;

    op->definition_type = DT_UNKNOWN;
}

void set_override_definition_type(OverrideParameters* op, DefinitionType dt) {
    if (op == NULL) return;
    op->definition_type = dt;
}

DefinitionType get_override_definition_type(OverrideParameters* op) {
    if (op == NULL) return DT_UNKNOWN;
    return op->definition_type;
}

void set_override_n(OverrideParameters* op, int n) {
    if (op == NULL) return;
    op->n = n;
}

void set_override_layout(OverrideParameters* op, Layout layout) {
    if (op == NULL) return;
    op->layout = layout;
}

void set_override_cursor(OverrideParameters* op, Cursor cursor) {
    if (op == NULL) return;
    op->cursor = cursor;
}

void set_override_term_name(OverrideParameters* op, const char* term_name) {
    if (op == NULL) return;
    op->term_name = term_name;
}

void set_override_color_spec(OverrideParameters* op, const char* spec) {
    if (op == NULL) return;
    op->color_spec = spec;
}

int get_override_n(const OverrideParameters* op) {
    if (op == NULL) return 1;
    return op->n;
}

Layout get_override_layout(const OverrideParameters* op) {
    if (op == NULL) return ANGEL_MASTER_LEFT;
    return op->layout;
}

Cursor get_override_cursor(const OverrideParameters* op) {
    if (op == NULL) return cursors.standard_pointer;
    return op->cursor;
}

const char* get_override_term_name(const OverrideParameters* op) {
    if (op == NULL) return NULL;
    return op->term_name;
}

const char* get_override_color_spec(const OverrideParameters* op) {
    if (op == NULL) return NULL;
    return op->color_spec;
}

_Bool set_default_unfocused_border_width(int bw) {
    _Bool good = validate_border_width(&bw);
    defaults.default_unfocused_border_width = bw;
    return good == true ? true : false;
}

_Bool set_default_focused_border_width(int bw) {
    _Bool good = validate_border_width(&bw);
    defaults.default_focused_border_width = bw;
    return good == true ? true : false;
}

_Bool set_default_resize_border_width(int bw) {
    _Bool good = validate_border_width(&bw);
    defaults.default_resize_border_width = bw;
    return good == true ? true : false;
}

_Bool set_default_float_border_width(int bw) {
    _Bool good = validate_border_width(&bw);
    defaults.default_float_border_width = bw;
    return good == true ? true : false;
}

_Bool set_default_minimized_border_width(int bw) {
    _Bool good = validate_border_width(&bw);
    defaults.default_minimized_border_width = bw;
    return good == true ? true : false;
}

_Bool set_default_monocle_border_width(int bw) {
    _Bool good = validate_border_width(&bw);
    defaults.default_monocle_border_width = bw;
    return good == true ? true : false;
}

_Bool set_default_fs_border_width(int bw) {
    _Bool good = validate_border_width(&bw);
    defaults.default_fs_border_width = bw;
    return good == true ? true : false;
}

_Bool validate_border_width(int* bw) {
    if (*bw >= 0) return true;

    *bw = 0;
    lerr("Bad border width. Adjusting to 0");
    return false;
}

_Bool set_default_gap(int gap) {
    if (gap < 0) {
        lerr("Bad gap. Adjusting to 0");
        defaults.default_gap = 0;
        return false;
	}
	defaults.default_gap = gap;
	return true;
}

_Bool set_default_gap_inc(int inc) {
    if (inc < 0) {
        lerr("Bad inc increment. Adjusting to 2");
        defaults.default_gap_inc = 2;
        return false;
	}
	defaults.default_gap_inc = inc;
	return true;
}

_Bool set_default_resize_inc(int inc) {
    if (inc < 0) {
        lerr("Bad resize increment. Adjusting to 0");
        defaults.default_resize_inc = 0;
        return false;
	}
	defaults.default_resize_inc = inc;
	return true;
}

_Bool set_default_minimized_height_inc(int inc) {
    if (inc < 0) {
        lerr("Bad resize increment. Adjusting to 0");
        defaults.default_minimized_height_inc = 0;
        return false;
	}
	defaults.default_minimized_height_inc = inc;
	return true;
}

_Bool set_default_float_move_step(int step) {
    if (step < 0) {
        lerr("Bad floating window movement step. Adjusting to 0");
        defaults.default_float_move_step = 0;
        return false;
	}
	defaults.default_float_move_step = step;
	return true;
}

_Bool set_default_minimized_height(int h) {
    int max = 0;

    MinimizedPosition pos = get_fallback_minimized_position();
    if (pos == MINIMIZED_BOTTOM || pos == MINIMIZED_TOP)
        max = 0.9 * get_min_height_of_monitors();
    else
        max = 0.9 * get_min_width_of_monitors();

    if (h < 5 || h > max) {
        lerr("Bad mininimized height. Adjusting to 5 pixels");
        defaults.default_minimized_height = 5;
        return false;
    }

	defaults.default_minimized_height = h;
	return true;
}

_Bool set_default_space_between_monocle(int space) {
    if (space < 0 || space > 0.5 * get_min_height_of_monitors()) {
        lerr(
            "Space between monocled windows out of bounds."
            "Adjusting to 1% screen height"
        );
        defaults.default_space_between_monocle = 0.1 * get_min_height_of_monitors();
        return false;
	}
	defaults.default_space_between_monocle = space;
	return true;
}

_Bool set_default_layout(Layout layout) {
    if (!is_layout_accepted(layout)) {
        lerr("Layout unknown. Adjusting to master-left");
        defaults.default_layout = ANGEL_MASTER_LEFT;
        return false;
	}
	defaults.default_layout = layout;
	return true;
}

_Bool set_default_root_cursor(Cursor cursor) {
    if (cursor == (Cursor)-1) {
        lerr("Bad cursor. Adjusting to standard pointer");
        defaults.default_root_cursor = cursors.standard_pointer;
        return false;
    }
	defaults.default_root_cursor = cursor;
    return true;
}

_Bool set_default_term(const char* term_name) {
    if (term_name == NULL || strcmp(term_name, "") == 0) {
        lerr("Bad term name. Adjusting to kitty");
        defaults.default_term = "kitty";
        return false;
	}
	defaults.default_term = term_name;
	return true;
}

_Bool set_default_unfocused_border(const char* spec) {
    Color color = get_color(spec);
    if (spec == NULL || strcmp(spec, "") == 0 || color.bad) {
        lerr("Bad unfocused window border color. Adjusting to blue");
        defaults.default_unfocused_border = "#27DAF5";
        return false;
	}
	defaults.default_unfocused_border = spec;
	return true;
}

_Bool set_default_focused_border(const char* spec) {
    Color color = get_color(spec);
    if (spec == NULL || strcmp(spec, "") == 0 || color.bad) {
        lerr("Bad focused window border color. Adjusting to pink");
        defaults.default_focused_border = "pink";
        return false;
	}
	defaults.default_focused_border = spec;
	return true;
}

_Bool set_default_resize_border(const char* spec) {
    Color color = get_color(spec);
    if (spec == NULL || strcmp(spec, "") == 0 || color.bad) {
        lerr("Bad resize window border color. Adjusting to red");
        defaults.default_resize_border = "red";
        return false;
	}
	defaults.default_resize_border = spec;
	return true;
}

_Bool set_default_float_border(const char* spec) {
    Color color = get_color(spec);
    if (spec == NULL || strcmp(spec, "") == 0 || color.bad) {
        lerr("Bad floating window border color. Adjusting to green");
        defaults.default_float_border = "green";
        return false;
	}
	defaults.default_float_border = spec;
	return true;
}

_Bool set_default_fs_border(const char* spec) {
    Color color = get_color(spec);
    if (spec == NULL || strcmp(spec, "") == 0 || color.bad) {
        lerr("Bad fullscreen window border color. Adjusting to green");
        defaults.default_fs_border = "green";
        return false;
	}
	defaults.default_fs_border = spec;
	return true;
}

_Bool set_default_minimized_position(MinimizedPosition minimized_position) {
    if (minimized_position != MINIMIZED_LEFT 
        && minimized_position != MINIMIZED_RIGHT
        && minimized_position != MINIMIZED_BOTTOM
        && minimized_position != MINIMIZED_TOP
    ) {
        lerr("Bad minimized position. Adjusting to bottom");
        defaults.default_minimized_position = MINIMIZED_BOTTOM;
        return false;
    }

    defaults.default_minimized_position = minimized_position;
    return true;
}

_Bool override_noop(const OverrideParameters* op) { (void)op; return true;}

_Bool override_default_unfocused_border_width(const OverrideParameters* op) {
	_Bool st = set_default_unfocused_border_width(get_override_n(op));
	set_unfocused_border_width(get_default_unfocused_border_width());
	return st;
}

_Bool override_default_focused_border_width(const OverrideParameters* op) {
	_Bool st = set_default_focused_border_width(get_override_n(op));
	set_focused_border_width(get_default_focused_border_width());
	return st;
}

_Bool override_default_resize_border_width(const OverrideParameters* op) {
	_Bool st = set_default_resize_border_width(get_override_n(op));
	set_resize_border_width(get_default_resize_border_width());
	return st;
}

_Bool override_default_float_border_width(const OverrideParameters* op) {
	_Bool st = set_default_float_border_width(get_override_n(op));
	set_float_border_width(get_default_float_border_width());
	return st;
}

_Bool override_default_minimized_border_width(const OverrideParameters* op) {
	_Bool st = set_default_minimized_border_width(get_override_n(op));
	set_minimized_border_width(get_default_minimized_border_width());
	return st;
}

_Bool override_default_monocle_border_width(const OverrideParameters* op) {
	_Bool st = set_default_monocle_border_width(get_override_n(op));
	set_monocle_border_width(get_default_monocle_border_width());
	return st;
}

_Bool override_default_fs_border_width(const OverrideParameters* op) {
	_Bool st = set_default_fs_border_width(get_override_n(op));
	set_fs_geometry(get_default_fs_border_width(), get_default_fs_border());
	return st;
}

_Bool override_default_gap(const OverrideParameters* op) {
	_Bool st = set_default_gap(get_override_n(op));
	set_gap(get_default_gap());
    set_min_gap_while_policing(get_default_gap());
	return st;
}

_Bool override_default_gap_inc(const OverrideParameters* op) {
	_Bool st = set_default_gap_inc(get_override_n(op));
	set_gap_inc(get_default_gap_inc());
	return st;
}

_Bool override_default_resize_inc(const OverrideParameters* op) {
	_Bool st = set_default_resize_inc(get_override_n(op));
	set_window_resize_inc(get_default_resize_inc());
	return st;
}

_Bool override_default_minimized_height_inc(const OverrideParameters* op) {
	_Bool st = set_default_minimized_height_inc(get_override_n(op));
	set_minimized_height_inc(get_default_minimized_height_inc());
	return st;
}

_Bool override_default_float_move_step(const OverrideParameters* op) {
	_Bool st = set_default_float_move_step(get_override_n(op));
	set_floating_window_keyboard_movement_step(get_default_float_move_step());
	return st;
}

_Bool override_default_minimized_height(const OverrideParameters* op) {
	_Bool st = set_default_minimized_height(get_override_n(op));
	set_minimized_height(get_default_minimized_height());
	return st;
}

_Bool override_default_space_between_monocle(const OverrideParameters* op) {
	_Bool st = set_default_space_between_monocle(get_override_n(op));
	set_monocle_space_between(get_default_space_between_monocle());
	return st;
}

_Bool override_default_layout(const OverrideParameters* op) {
	_Bool st = set_default_layout(get_override_layout(op));
    set_default_workspace_layout(get_default_layout());
	return st;
}

_Bool override_default_root_cursor(const OverrideParameters* op) {
	_Bool st = set_default_root_cursor(get_override_cursor(op));
	set_root_cursor(get_default_root_cursor());
	return st;
}

_Bool override_default_term(const OverrideParameters* op) {
	_Bool st = set_default_term(get_override_term_name(op));
	set_term_name(get_default_term());
	return st;
}

_Bool override_default_unfocused_border(const OverrideParameters* op) {
	_Bool st = set_default_unfocused_border(get_override_color_spec(op));
	set_unfocused_border_color(get_default_unfocused_border());
	return st;
}

_Bool override_default_focused_border(const OverrideParameters* op) {
	_Bool st = set_default_focused_border(get_override_color_spec(op));
	set_focused_border_color(get_default_focused_border());
	return st;
}

_Bool override_default_resize_border(const OverrideParameters* op) {
	_Bool st = set_default_resize_border(get_override_color_spec(op));
	set_resize_border_color(get_default_resize_border());
	return st;
}

_Bool override_default_float_border(const OverrideParameters* op) {
	_Bool st = set_default_float_border(get_override_color_spec(op));
	set_float_border_color(get_default_float_border());
	return st;
}

_Bool override_default_fs_border(const OverrideParameters* op) {
	_Bool st = set_default_fs_border(get_override_color_spec(op));
	set_fs_geometry(get_default_fs_border_width(), get_default_fs_border());
	return st;
}

_Bool override_default_minimized_position(const OverrideParameters* op) {
    _Bool st = set_default_minimized_position(get_override_n(op));
    set_fallback_minimized_position(get_default_minimized_position());
    return st;
}

_Bool override_default_focus_start(const OverrideParameters* op) {
    _Bool st = set_focus_start(get_override_n(op));
    return st;
}

_Bool override_default_focus_end(const OverrideParameters* op) {
    _Bool st = set_focus_end(get_override_n(op));
    return st;
}

unsigned int get_default_unfocused_border_width() {
	return defaults.default_unfocused_border_width;
}

unsigned int get_default_focused_border_width() {
	return defaults.default_focused_border_width;
}

unsigned int get_default_resize_border_width() {
	return defaults.default_resize_border_width;
}

unsigned int get_default_float_border_width() {
	return defaults.default_float_border_width;
}

unsigned int get_default_minimized_border_width() {
	return defaults.default_minimized_border_width;
}

unsigned int get_default_monocle_border_width() {
	return defaults.default_monocle_border_width;
}

unsigned int get_default_fs_border_width() {
	return defaults.default_fs_border_width;
}

unsigned int get_default_gap() {
	return defaults.default_gap;
}

unsigned int get_default_gap_inc() {
	return defaults.default_gap_inc;
}

unsigned int get_default_resize_inc() {
	return defaults.default_resize_inc;
}

unsigned int get_default_minimized_height_inc() {
	return defaults.default_minimized_height_inc;
}

int get_default_float_move_step() {
	return defaults.default_float_move_step;
}

int get_default_minimized_height() {
	return defaults.default_minimized_height;
}

int get_default_space_between_monocle() {
	return defaults.default_space_between_monocle;
}

Layout get_default_layout() {
	return defaults.default_layout;
}

Cursor get_default_root_cursor() {
	return defaults.default_root_cursor;
}

const char* get_default_term() {
	return defaults.default_term;
}

const char* get_default_unfocused_border() {
	return defaults.default_unfocused_border;
}

const char* get_default_focused_border() {
	return defaults.default_focused_border;
}

const char* get_default_resize_border() {
	return defaults.default_resize_border;
}

const char* get_default_float_border() {
	return defaults.default_float_border;
}

const char* get_default_fs_border() {
	return defaults.default_fs_border;
}

MinimizedPosition get_default_minimized_position() {
    return defaults.default_minimized_position;
}

void scatter_defaults() {
    set_root_cursor(get_default_root_cursor());
    set_term_name(get_default_term());
    set_focused_border_width(get_default_focused_border_width());
    set_unfocused_border_width(get_default_unfocused_border_width());
    set_float_border_width(get_default_float_border_width());
    set_minimized_border_width(get_default_minimized_border_width());
    set_resize_border_width(get_default_resize_border_width());
    set_unfocused_border_color(get_default_unfocused_border());
    set_focused_border_color(get_default_focused_border());
    set_gap(get_default_gap());
    set_min_gap_while_policing(get_default_gap());
    set_window_resize_inc(get_default_resize_inc());
    set_floating_window_keyboard_movement_step(get_default_float_move_step());
    set_resize_border_color(get_default_resize_border());
    set_float_border_color(get_default_float_border());
    set_default_workspace_layout(get_default_layout());
    set_fs_geometry(get_default_fs_border_width(), get_default_fs_border());
    set_minimized_height(get_default_minimized_height());
    set_monocle_space_between(get_default_space_between_monocle());
    set_fallback_minimized_position(get_default_minimized_position());
    set_minimized_height_inc(get_default_minimized_height_inc());
    set_gap_inc(get_default_gap_inc());
}

void set_defaults() {
    set_default_focused_border_width(5);
    set_default_unfocused_border_width(5);
    set_default_float_border_width(5);
    set_default_minimized_border_width(5);
    set_default_resize_border_width(5);
    set_default_monocle_border_width(5);
    set_default_fs_border_width(1);
    set_default_gap(10);
    set_default_float_move_step(15);
    set_default_resize_inc(15);
    set_default_space_between_monocle(20);
    set_default_layout(ANGEL_MASTER_LEFT);
    set_default_unfocused_border("#27daf5");
    set_default_focused_border("pink");
    set_default_resize_border("red");
    set_default_float_border("green");
    set_default_fs_border("green");
    set_default_minimized_height(10);
    set_default_term("kitty");
    set_default_root_cursor(cursors.standard_pointer);
    set_default_minimized_position(MINIMIZED_BOTTOM);
    set_default_minimized_height_inc(5);
    set_default_gap_inc(2);

    scatter_defaults();
}
