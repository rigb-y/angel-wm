/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_EVENTS_H
#define ANGEL_EVENTS_H

#include <X11/Xlib.h>

typedef struct KeyMap KeyMap;
typedef struct Client Client;
typedef struct Monitor Monitor;
typedef struct Monitors Monitors;

void evloop();
_Bool client_is_must_leave(Client*);
void set_must_leave(Client*);
void clear_must_leave();
_Bool wm_running();
_Bool wm_restart();
_Bool wm_quit();

void handle_nothing(const XEvent*);
void handle_enter_notify(const XEvent*);
void handle_leave_notify(const XEvent*);
void handle_key_press(const XEvent*);
void handle_map_request(const XEvent*);
void handle_unmap_notify(const XEvent*);
void handle_map_notify(const XEvent*);
void handle_destroy_notify(const XEvent*);
void handle_button_press(const XEvent*);
void handle_button_release(const XEvent*);
void handle_motion_notify(const XEvent*);
void handle_focus_change(const XEvent*);
void handle_mapping_notify(const XEvent*);
void handle_configure_request(const XEvent*);
void handle_screen_change_notify(const XEvent*);
void handle_rr_notify(const XEvent*);

void dispatch_keybind(const XKeyEvent*, const KeyMap*);
void action_noop(const XKeyEvent*);
void action_close_win(const XKeyEvent*);
void action_open_term(const XKeyEvent*);
void action_win_down(const XKeyEvent*);
void action_win_up(const XKeyEvent*);
void action_win_left(const XKeyEvent*);
void action_win_right(const XKeyEvent*);
void action_win_move_down(const XKeyEvent*);
void action_win_move_up(const XKeyEvent*);
void action_win_move_left(const XKeyEvent*);
void action_win_move_right(const XKeyEvent*);
void action_enter_resize(const XKeyEvent*);
void action_exit_resize(const XKeyEvent*);
void action_resize_down(const XKeyEvent*);
void action_resize_up(const XKeyEvent*);
void action_resize_left(const XKeyEvent*);
void action_resize_right(const XKeyEvent*);
void action_toggle_fullscreen(const XKeyEvent*);
void action_toggle_float(const XKeyEvent*);
void action_toggle_floaters_focus(const XKeyEvent*);
void action_switch_float_focus(const XKeyEvent*);
void action_minimize_window(const XKeyEvent*);
void action_toggle_minimize_focus(const XKeyEvent*);
void action_tile_master_left(const XKeyEvent*);
void action_tile_master_right(const XKeyEvent*);
void action_tile_simple_vertical(const XKeyEvent*);
void action_tile_simple_horizontal(const XKeyEvent*);
void action_tile_monocle(const XKeyEvent*);
void action_tile_master_left_monocle(const XKeyEvent*);
void action_tile_master_right_monocle(const XKeyEvent*);
void action_tile_master_master_left(const XKeyEvent*);
void action_tile_master_master_right(const XKeyEvent*);
void action_switch_to_workspace_one(const XKeyEvent*);
void action_switch_to_workspace_two(const XKeyEvent*);
void action_switch_to_workspace_three(const XKeyEvent*);
void action_switch_to_workspace_four(const XKeyEvent*);
void action_switch_to_workspace_five(const XKeyEvent*);
void action_switch_to_workspace_six(const XKeyEvent*);
void action_switch_to_workspace_seven(const XKeyEvent*);
void action_switch_to_workspace_eight(const XKeyEvent*);
void action_switch_to_workspace_nine(const XKeyEvent*);
void action_switch_to_workspace_ten(const XKeyEvent*);
void action_move_to_workspace_one(const XKeyEvent*);
void action_move_to_workspace_two(const XKeyEvent*);
void action_move_to_workspace_three(const XKeyEvent*);
void action_move_to_workspace_four(const XKeyEvent*);
void action_move_to_workspace_five(const XKeyEvent*);
void action_move_to_workspace_six(const XKeyEvent*);
void action_move_to_workspace_seven(const XKeyEvent*);
void action_move_to_workspace_eight(const XKeyEvent*);
void action_move_to_workspace_nine(const XKeyEvent*);
void action_move_to_workspace_ten(const XKeyEvent*);
void action_restart_manager(const XKeyEvent*);
void action_quit_manager(const XKeyEvent*);
void action_unmap_all_workspace_windows(const XKeyEvent*);
void action_map_all_workspace_windows(const XKeyEvent*);
void action_unmap_window(const XKeyEvent*);
void action_map_latest_unmap(const XKeyEvent*);
void action_gap_dec(const XKeyEvent*);
void action_gap_inc(const XKeyEvent*);
void action_float_all_tiled(const XKeyEvent*);
void action_tile_all_float(const XKeyEvent*);
void action_minimize_all(const XKeyEvent*);
void action_unminimize_all(const XKeyEvent*);

void action_minimize_left(const XKeyEvent*);
void action_minimize_right(const XKeyEvent*);
void action_minimize_top(const XKeyEvent*);
void action_minimize_bottom(const XKeyEvent*);

void action_inc_minimized_size(const XKeyEvent*);
void action_dec_minimized_size(const XKeyEvent*);

void action_move_win_monitor_down(const XKeyEvent*);
void action_move_win_monitor_up(const XKeyEvent*);
void action_move_win_monitor_left(const XKeyEvent*);
void action_move_win_monitor_right(const XKeyEvent*);
void action_monitor_motion(Monitors*, Monitor*, int, int, Time);

void action_change_to_focus_mode_focus(const XKeyEvent*);
void action_change_to_focus_mode_pointer(const XKeyEvent*);

void action_cycle_tiled_forward(const XKeyEvent*);
void action_cycle_tiled_backward(const XKeyEvent*);

void action_new_focus_start_adjacent(const XKeyEvent*);
void action_new_focus_start_end(const XKeyEvent*);

void action_next_focus_on_close_use_stack(const XKeyEvent*);
void action_next_focus_on_close_next(const XKeyEvent*);

void action_toggle_persistent(const XKeyEvent*);

#endif
