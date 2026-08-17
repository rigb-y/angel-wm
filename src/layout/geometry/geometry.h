/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_GEOMETRY_H
#define ANGEL_GEOMETRY_H

#include "colors.h"
#include "types.h"

#include <X11/Xlib.h>

#define ZERO_BORDER_WIDTH 0
#define NO_X_INC 0
#define NO_Y_INC 0
#define ORIGIN_X 0
#define ORIGIN_Y 0

#define JUSTIFY_FOCUS true
#define NO_JUSTIFY_FOCUS false

typedef struct Monitor Monitor;
typedef struct Monitors Monitors;

typedef struct FullscreenGeometry FullscreenGeometry;
extern FullscreenGeometry fs_geometry;

typedef struct Geometry Geometry;
extern Geometry geometry;

typedef struct MonocleGeometry MonocleGeometry;
extern MonocleGeometry monocle_geometry;

typedef struct MTNode MTNode;
typedef struct MotionTree MotionTree;
typedef struct Client Client;
typedef struct DetachedClient DetachedClient;

typedef struct Position {
    int x, y;
} Position;

/* Position statics */
void set_position(Position*, int, int);
Position create_position(int, int);
int pos_x(const Position*);
int pos_y(const Position*);
void set_pos_x(Position*, int);
void set_pos_y(Position*, int);
void add_to_pos_x(Position*, int);
void sub_from_pos_x(Position*, int);
void add_to_pos_y(Position*, int);
void sub_from_pos_y(Position*, int);

typedef struct FullscreenGeometry {
    unsigned int border_width;
    Color border_color;
} FullscreenGeometry;

void set_fs_geometry(int, const char*); 

unsigned int get_fs_border_width();
Color get_fs_border_color();
int get_fs_x(Monitor*);
int get_fs_y(Monitor*);
int get_fs_width(Monitor*);
int get_fs_height(Monitor*);

typedef struct MonocleGeometry {
    int space_between;
} MonocleGeometry;

void set_monocle_space_between(int);
int get_monocle_space_between();
_Bool in_monocle_stack(Monitor*, const Client*);

typedef struct Geometry {
    Color unfocused_border_color;
    Color focused_border_color;
    Color resize_border_color;
    Color float_border_color;
    unsigned int unfocused_border_width;
    unsigned int focused_border_width;
    unsigned int resize_border_width;
    unsigned int float_border_width;
    unsigned int minimized_border_width;
    unsigned int monocle_border_width;
    unsigned int gap;
    unsigned int gap_inc;
    unsigned int window_resize_inc;
    int floating_window_movement_step;
    int minimized_height;
    MinimizedPosition fallback_minimized_position;
    unsigned int minimized_height_inc;
    unsigned int min_gap_while_policing;
} Geometry;

void set_fallback_minimized_position(MinimizedPosition);
MinimizedPosition get_fallback_minimized_position();

void set_min_gap_while_policing(int);
unsigned int get_min_gap_while_policing();

int get_max_width(Monitor*);
int get_max_height(Monitor*);
int get_min_width();
int get_min_height();

int get_min_floater_width(Monitor*, const Client*);
int get_min_floater_height(Monitor*, const Client*);

int get_max_floater_width(Monitor*, const Client*);
int get_max_floater_height(Monitor*, const Client*);

Color get_border_color(const Client*);
unsigned int get_border_width(const Client*);

unsigned int get_unfocused_border_width();
unsigned int get_focused_border_width();
unsigned int get_resize_border_width();
unsigned int get_float_border_width();
unsigned int get_minimized_border_width();
unsigned int get_monocle_border_width();

void set_unfocused_border_width(int);
void set_focused_border_width(int);
void set_resize_border_width(int);
void set_float_border_width(int);
void set_minimized_border_width(int);
void set_monocle_border_width(int);

void set_unfocused_border_color(const char*);
Color get_unfocused_border_color();

void set_focused_border_color(const char*);
Color get_focused_border_color();

void set_resize_border_color(const char*);
Color get_resize_border_color();

void set_float_border_color(const char*);
Color get_float_border_color();

void set_gap(int);
void set_gap_inc(int);
unsigned int get_gap();
unsigned int get_gap_inc();

void set_window_resize_inc(int);
unsigned int get_window_resize_inc();
void set_minimized_height_inc(int);
unsigned int get_minimized_height_inc();
unsigned int get_window_width_resize_inc(Window);
unsigned int get_window_height_resize_inc(Window);

void set_floating_window_keyboard_movement_step(int);
int get_floating_window_keyboard_movement_step();

void set_minimized_height(int);
int get_minimized_height();

void configure_client(Monitor*, Client*, int, int, int, int);
void move_client(Client*, int, int);
void resize_client(Client*, int, int);
void position_client(Client*);
void arrange_monitors(Time, _Bool);
void arrange_monitor(Monitor*, Time, _Bool);
void reconcile_floaters(Monitor*);
void raise_all_floaters(Monitor*);
void move_floater(Monitor*, DetachedClient*, int, int);
void resize_floater(Monitor*, DetachedClient*, int, int);
void reconcile_minimized(Monitor*);
void reconcile_struts(Monitor*);

void tile_noop(Monitor*);

void simple_tile_horizontal(Monitor*);
void simple_tile_vertical(Monitor*);
void tile_master_left(Monitor*);
void tile_master_right(Monitor*);
void tile_monocle(Monitor*);

void tile_master_left_monocle(Monitor*);
void tile_master_right_monocle(Monitor*);
void tile_master_master_left(Monitor*);
void tile_master_master_right(Monitor*);

_Bool floater_in_bounds(Monitor*, DetachedClient*, int, int, int, int);
Monitor* get_primary_monitor_or_fallback(Monitors*);

MinimizedPosition get_minimized_position(Monitor*);

#endif
