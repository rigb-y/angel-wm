/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_DEFAULTS_H
#define ANGEL_DEFAULTS_H

#include "layouts.h"
#include "types.h"

#define DESTROY_TERM_NAME true
#define NO_DESTROY_TERM_NAME false

#define DESTROY_COLOR_SPEC true
#define NO_DESTROY_COLOR_SPEC false

typedef struct DefaultConfig DefaultConfig;
extern DefaultConfig defaults;

typedef enum DefinitionType {
    DT_UNKNOWN, DT_VAR, DT_DEFINE, 
    DT_SET, DT_COLOR, DT_DECLARE,
    DT_SET_BACKGROUND, DT_LAYOUT,
    DT_BIND_COMMAND, DT_BIND
} DefinitionType;

typedef struct DefaultConfig {
    unsigned int default_unfocused_border_width;
    unsigned int default_focused_border_width;
    unsigned int default_resize_border_width;
    unsigned int default_float_border_width;
    unsigned int default_minimized_border_width;
    unsigned int default_monocle_border_width;
    unsigned int default_fs_border_width;

    unsigned int default_gap;
    unsigned int default_gap_inc;
    unsigned int default_resize_inc;

    unsigned int default_minimized_height_inc;
    MinimizedPosition default_minimized_position;

    int default_float_move_step;

    int default_minimized_height;

    int default_space_between_monocle;

    Layout default_layout;

    Cursor default_root_cursor;

    const char* default_term;

    const char* default_unfocused_border;
    const char* default_focused_border;
    const char* default_resize_border;
    const char* default_float_border;
    const char* default_fs_border;
} DefaultConfig;

typedef struct OverrideParameters {
    int n;
    Layout layout;
    Cursor cursor;
    const char* term_name;
    const char* color_spec;

    _Bool manage_term_name;
    _Bool manage_color_spec;

    DefinitionType definition_type;
} OverrideParameters;

void set_override_parameters(
    OverrideParameters*,
    int,
    Layout,
    Cursor,
    const char*,
    const char*
);
OverrideParameters create_override_parameters();
void destroy_override_parameters(OverrideParameters*);

void set_override_definition_type(OverrideParameters*, DefinitionType);
DefinitionType get_override_definition_type(OverrideParameters*);

void set_override_n(OverrideParameters*, int);
void set_override_layout(OverrideParameters*, Layout);
void set_override_cursor(OverrideParameters*, Cursor);
void set_override_term_name(OverrideParameters*, const char*);
void set_override_color_spec(OverrideParameters*, const char*);

int get_override_n(const OverrideParameters*);
Layout get_override_layout(const OverrideParameters*);
Cursor get_override_cursor(const OverrideParameters*);
const char* get_override_term_name(const OverrideParameters*);
const char* get_override_color_spec(const OverrideParameters*);

_Bool set_default_unfocused_border_width(int);
_Bool set_default_focused_border_width(int);
_Bool set_default_resize_border_width(int);
_Bool set_default_float_border_width(int);
_Bool set_default_minimized_border_width(int);
_Bool set_default_monocle_border_width(int);
_Bool set_default_fs_border_width(int);
_Bool set_default_gap(int);
_Bool set_default_gap_inc(int);
_Bool set_default_resize_inc(int);
_Bool set_default_minimized_height_inc(int);
_Bool set_default_float_move_step(int);
_Bool set_default_minimized_height(int);
_Bool set_default_space_between_monocle(int);
_Bool set_default_layout(Layout);
_Bool set_default_root_cursor(Cursor);
_Bool set_default_term(const char*);
_Bool set_default_unfocused_border(const char*);
_Bool set_default_focused_border(const char*);
_Bool set_default_resize_border(const char*);
_Bool set_default_float_border(const char*);
_Bool set_default_fs_border(const char*);
_Bool set_default_minimized_position(MinimizedPosition);

_Bool override_noop(const OverrideParameters*);
_Bool override_default_unfocused_border_width(const OverrideParameters*);
_Bool override_default_focused_border_width(const OverrideParameters*);
_Bool override_default_resize_border_width(const OverrideParameters*);
_Bool override_default_float_border_width(const OverrideParameters*);
_Bool override_default_minimized_border_width(const OverrideParameters*);
_Bool override_default_monocle_border_width(const OverrideParameters*);
_Bool override_default_fs_border_width(const OverrideParameters*);
_Bool override_default_gap(const OverrideParameters*);
_Bool override_default_gap_inc(const OverrideParameters*);
_Bool override_default_resize_inc(const OverrideParameters*);
_Bool override_default_minimized_height_inc(const OverrideParameters*);
_Bool override_default_float_move_step(const OverrideParameters*);
_Bool override_default_minimized_height(const OverrideParameters*);
_Bool override_default_space_between_monocle(const OverrideParameters*);
_Bool override_default_layout(const OverrideParameters*);
_Bool override_default_root_cursor(const OverrideParameters*);
_Bool override_default_term(const OverrideParameters*);
_Bool override_default_unfocused_border(const OverrideParameters*);
_Bool override_default_focused_border(const OverrideParameters*);
_Bool override_default_resize_border(const OverrideParameters*);
_Bool override_default_float_border(const OverrideParameters*);
_Bool override_default_fs_border(const OverrideParameters*);
_Bool override_default_minimized_position(const OverrideParameters*);
_Bool override_default_focus_start(const OverrideParameters*);
_Bool override_default_focus_end(const OverrideParameters* op);

unsigned int get_default_unfocused_border_width();
unsigned int get_default_focused_border_width();
unsigned int get_default_resize_border_width();
unsigned int get_default_float_border_width();
unsigned int get_default_minimized_border_width();
unsigned int get_default_monocle_border_width();
unsigned int get_default_fs_border_width();
unsigned int get_default_resize_inc();
unsigned int get_default_minimized_height_inc();
unsigned int get_default_gap();
unsigned int get_default_gap_inc();
int get_default_float_move_step();
int get_default_minimized_height();
int get_default_space_between_monocle();
Layout get_default_layout();
Cursor get_default_root_cursor();
const char* get_default_term();
const char* get_default_unfocused_border();
const char* get_default_focused_border();
const char* get_default_resize_border();
const char* get_default_float_border();
const char* get_default_fs_border();
MinimizedPosition get_default_minimized_position();

void set_defaults();

#endif
