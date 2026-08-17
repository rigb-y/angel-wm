/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "parser_utils.h"
#include "token.h"
#include "config_error.h"
#include "lexer.h"
#include "types.h"
#include "utils.h"
#include "defaults.h"
#include "layouts.h"
#include "cursors.h"
#include "angel_strings.h"
#include "events.h"
#include "workspaces.h"
#include "monitors.h"

#include <string.h>
#include <stdbool.h>

static _Bool is_valid(const Token*, const IdentifierType[], int);
static _Bool try_skip_block(Token*);
static int reconcile_integer_value(const Token*);
static MinimizedPosition get_minimized_position_from_ident_type(const Token*);
static FocusStart get_focus_start_position_from_ident_type(const Token*);
static FocusEnd get_focus_end_position_from_ident_type(const Token*);

static void reconcile_ident_value(
    OverrideParameters*,
    const Token*,
    int*,
    Layout*,
    Cursor*,
    const char**
);

static void reconcile_string(
    OverrideParameters*,
    const Token*,
    const char**,
    const char**
);

static const IdentifierType valid_defines[] = {
    IDENT_TERMINAL, IDENT_ROOT_CURSOR, IDENT_MINIMIZED_HEIGHT,
    IDENT_NEW_FOCUS_START, IDENT_NEXT_FOCUS_ON_CLOSE
};

static const IdentifierType valid_sets[] = {
    IDENT_GAP, IDENT_FULLSCREEN_BORDER_WIDTH,
    IDENT_WINDOW_RESIZE_INC, IDENT_FLOAT_MOVE_STEP,
    IDENT_DEFAULT_WORKSPACE_LAYOUT, 
    IDENT_FOCUSED_BORDER_WIDTH,
    IDENT_UNFOCUSED_BORDER_WIDTH,
    IDENT_RESIZE_BORDER_WIDTH,
    IDENT_FLOAT_BORDER_WIDTH,
    IDENT_MINIMIZED_BORDER_WIDTH,
    IDENT_MONOCLE_BORDER_WIDTH,
    IDENT_SPACE_BETWEEN_MONOCLE,
    IDENT_MINIMIZED_POSITION,
    IDENT_MINIMIZED_HEIGHT_INC,
    IDENT_GAP_INC_SIZE
};

static const IdentifierType valid_colors[] = {
    IDENT_RESIZE_BORDER, IDENT_FOCUSED_BORDER, IDENT_UNFOCUSED_BORDER,
    IDENT_FLOAT_BORDER, IDENT_FULLSCREEN_BORDER
};

static const IdentifierType valid_layouts[] = {
    IDENT_ANGEL_MASTER_LEFT, IDENT_ANGEL_MASTER_RIGHT, 
    IDENT_ANGEL_SIMPLE_VERTICAL, IDENT_ANGEL_SIMPLE_HORIZONTAL,
    IDENT_ANGEL_MONOCLE, IDENT_ANGEL_MASTER_LEFT_MONOCLE,
    IDENT_ANGEL_MASTER_RIGHT_MONOCLE, IDENT_ANGEL_MASTER_MASTER_LEFT,
    IDENT_ANGEL_MASTER_MASTER_RIGHT
};

static const IdentifierType valid_declares[] = {
    IDENT_TILED, IDENT_FLOAT
};

static const IdentifierType valid_binds[] = {
    IDENT_OPEN_TERM, IDENT_WIN_DOWN, IDENT_WIN_UP,
    IDENT_WIN_LEFT, IDENT_WIN_RIGHT, IDENT_MOVE_WIN_DOWN,
    IDENT_MOVE_WIN_UP, IDENT_MOVE_WIN_LEFT, IDENT_MOVE_WIN_RIGHT,
    IDENT_ENTER_RESIZE, IDENT_EXIT_RESIZE, IDENT_RESIZE_DOWN,
    IDENT_RESIZE_UP, IDENT_RESIZE_LEFT, IDENT_RESIZE_RIGHT,
    IDENT_TOGGLE_FULLSCREEN, IDENT_TOGGLE_FLOAT, IDENT_TOGGLE_FLOAT_FOCUS,
    IDENT_SWITCH_FLOAT_FOCUS, IDENT_MINIMIZE_WIN, IDENT_TOGGLE_MINIMIZE_FOCUS,
    IDENT_TILE_MASTER_LEFT, IDENT_TILE_MASTER_RIGHT, IDENT_TILE_SIMPLE_VERTICAL,
    IDENT_TILE_SIMPLE_HORIZONTAL, IDENT_TILE_MONOCLE, IDENT_SWITCH_TO_W1,
    IDENT_SWITCH_TO_W2, IDENT_SWITCH_TO_W3, IDENT_SWITCH_TO_W4,
    IDENT_SWITCH_TO_W5, IDENT_SWITCH_TO_W6, IDENT_SWITCH_TO_W7,
    IDENT_SWITCH_TO_W8, IDENT_SWITCH_TO_W9, IDENT_SWITCH_TO_W10,
    IDENT_MOVE_TO_W1, IDENT_MOVE_TO_W2, IDENT_MOVE_TO_W3,
    IDENT_MOVE_TO_W4, IDENT_MOVE_TO_W5, IDENT_MOVE_TO_W6,
    IDENT_MOVE_TO_W7, IDENT_MOVE_TO_W8, IDENT_MOVE_TO_W9,
    IDENT_MOVE_TO_W10, IDENT_RESTART_MANAGER, IDENT_QUIT_MANAGER,
    IDENT_UNMAP_WORKSPACE, IDENT_MAP_WORKSPACE, IDENT_UNMAP_WINDOW, 
    IDENT_MAP_LATEST_UNMAP, IDENT_GAP_INC, IDENT_GAP_DEC,
    IDENT_FLOAT_ALL_TILED, IDENT_TILE_ALL_FLOAT,
    IDENT_MINIMIZE_ALL_WINDOWS, IDENT_UNMINIMIZE_ALL_WINDOWS,
    IDENT_MINIMIZE_LEFT, IDENT_MINIMIZE_RIGHT, IDENT_MINIMIZE_TOP,
    IDENT_MINIMIZE_BOTTOM, IDENT_MINIMIZE_INC, IDENT_MINIMIZE_DEC,
    IDENT_TILE_MASTER_LEFT_MONOCLE, IDENT_TILE_MASTER_RIGHT_MONOCLE, 
    IDENT_TILE_MASTER_MASTER_LEFT, IDENT_TILE_MASTER_MASTER_RIGHT,
    IDENT_MOVE_WIN_MONITOR_DOWN, IDENT_MOVE_WIN_MONITOR_UP,
    IDENT_MOVE_WIN_MONITOR_LEFT, IDENT_MOVE_WIN_MONITOR_RIGHT,
    IDENT_FOCUS_MODE_FOCUS, IDENT_FOCUS_MODE_POINTER,
    IDENT_CYCLE_TILED_FORWARD, IDENT_CYCLE_TILED_BACKWARD,
    IDENT_NEW_FOCUS_START_ADJACENT, IDENT_NEW_FOCUS_START_END,
    IDENT_NEXT_FOCUS_ON_CLOSE_FOCUS_STACK,
    IDENT_NEXT_FOCUS_ON_CLOSE_NEXT
};

static const IdentifierType valid_cursors[] = {
    IDENT_CURSOR_STANDARD_POINTER,
    IDENT_CURSOR_TEXT_INSERTION, IDENT_CURSOR_BUSY,
    IDENT_CURSOR_HAND_SHAPE, IDENT_CURSOR_CROSSHAIR,
    IDENT_CURSOR_FOUR_DF_RESIZE, IDENT_CURSOR_HORIZONTAL_RESIZE,
    IDENT_CURSOR_VERTICAL_RESIZE, IDENT_CURSOR_TOP_RESIZE,
    IDENT_CURSOR_BOTTOM_RESIZE, IDENT_CURSOR_LEFT_RESIZE,
    IDENT_CURSOR_RIGHT_RESIZE, IDENT_CURSOR_TOP_LEFT_RESIZE,
    IDENT_CURSOR_TOP_RIGHT_RESIZE, IDENT_CURSOR_BOTTOM_LEFT_RESIZE,
    IDENT_CURSOR_BOTTOM_RIGHT_RESIZE,
};

typedef struct IdentTypeLayoutPair {
    IdentifierType type;
    Layout layout;
} IdentTypeLayoutPair;

static const IdentTypeLayoutPair ident_type_to_layout[] = {
    {IDENT_ANGEL_MASTER_LEFT, ANGEL_MASTER_LEFT},
    {IDENT_ANGEL_MASTER_RIGHT, ANGEL_MASTER_RIGHT},
    {IDENT_ANGEL_SIMPLE_VERTICAL, ANGEL_SIMPLE_VERTICAL},
    {IDENT_ANGEL_SIMPLE_HORIZONTAL, ANGEL_SIMPLE_HORIZONTAL},
    {IDENT_ANGEL_MONOCLE, ANGEL_MONOCLE},
    {IDENT_ANGEL_MASTER_LEFT_MONOCLE, ANGEL_MASTER_LEFT_MONOCLE},
    {IDENT_ANGEL_MASTER_RIGHT_MONOCLE, ANGEL_MASTER_RIGHT_MONOCLE },
    {IDENT_ANGEL_MASTER_MASTER_LEFT, ANGEL_MASTER_MASTER_LEFT},
    {IDENT_ANGEL_MASTER_MASTER_RIGHT, ANGEL_MASTER_MASTER_RIGHT}
};

typedef struct IdentTypeCursorPair {
    IdentifierType type;
    Cursor cursor;
} IdentTypeCursorPair;

Layout get_layout_from_ident_type(const Token* token) {
    if (token == NULL) return ANGEL_LAYOUT_UNKNOWN;
    for (int i = 0; i < ARRAY_SIZE(ident_type_to_layout); ++i) 
        if (ident_type_to_layout[i].type == get_token_identifier_type(token))
            return ident_type_to_layout[i].layout;
    return ANGEL_LAYOUT_UNKNOWN;
}

typedef struct IdentTypeDefinitionTypePair {
    IdentifierType ident_type;
    DefinitionType def_type;
} IdentTypeDefinitiontypePair;

static const IdentTypeDefinitiontypePair ident_type_to_def_type[] = {
    {IDENT_DEFINE, DT_DEFINE},
    {IDENT_SET, DT_SET}, 
    {IDENT_COLOR, DT_COLOR}, 
    {IDENT_DECLARE, DT_DECLARE}, 
    {IDENT_SET_BACKGROUND, DT_SET_BACKGROUND},
    {IDENT_LAYOUTS, DT_LAYOUT}, 
    {IDENT_BIND, DT_BIND}, 
    {IDENT_BIND_COMMAND, DT_BIND_COMMAND}, 
    {IDENT_LET, DT_VAR}
};

DefinitionType get_def_type(const Token* token) {
    if (token == NULL) return DT_UNKNOWN;
    for (int i = 0; i < ARRAY_SIZE(ident_type_to_def_type); ++i)
        if (ident_type_to_def_type[i].ident_type == get_token_identifier_type(token))
            return ident_type_to_def_type[i].def_type;
    return DT_UNKNOWN;
}

typedef struct IdentTypeInitialStatePair {
    IdentifierType type;
    InitialState state;
} IdentTypeInitialStatePair;

static const IdentTypeInitialStatePair ident_type_to_initial_state[] = {
    {IDENT_TILED, TILED},
    {IDENT_FLOAT, FLOAT}
};

InitialState get_initial_state_from_ident_type(const Token* token) {
    if (token == NULL) return NULL_STATE;
    for (int I = 0; I < ARRAY_SIZE(ident_type_to_initial_state); ++I)
        if (ident_type_to_initial_state[I].type == get_token_identifier_type(token))
            return ident_type_to_initial_state[I].state;
    return NULL_STATE;
}

typedef struct IdentifierOverrideFnPair {
    IdentifierType type;
    OverrideDefaultFn fn;
} IdentifierOverrideFnPair;

static const IdentifierOverrideFnPair ident_type_to_override_fn[] = {
    {IDENT_UNKNOWN, override_noop},
    {IDENT_GAP, override_default_gap},
    {IDENT_TERMINAL, override_default_term},
    {IDENT_ROOT_CURSOR, override_default_root_cursor},
    {IDENT_FOCUSED_BORDER_WIDTH, override_default_focused_border_width},
    {IDENT_UNFOCUSED_BORDER_WIDTH, override_default_unfocused_border_width},
    {IDENT_RESIZE_BORDER_WIDTH, override_default_resize_border_width},
    {IDENT_FLOAT_BORDER_WIDTH, override_default_float_border_width},
    {IDENT_MINIMIZED_BORDER_WIDTH, override_default_minimized_border_width},
    {IDENT_FULLSCREEN_BORDER_WIDTH, override_default_fs_border_width},
    {IDENT_WINDOW_RESIZE_INC, override_default_resize_inc},
    {IDENT_FLOAT_MOVE_STEP, override_default_float_move_step},
    {IDENT_DEFAULT_WORKSPACE_LAYOUT, override_default_layout},
    {IDENT_MINIMIZED_HEIGHT, override_default_minimized_height},
    {IDENT_RESIZE_BORDER, override_default_resize_border},
    {IDENT_FOCUSED_BORDER, override_default_focused_border},
    {IDENT_UNFOCUSED_BORDER, override_default_unfocused_border},
    {IDENT_FLOAT_BORDER, override_default_float_border},
    {IDENT_FULLSCREEN_BORDER, override_default_fs_border},
    {IDENT_SPACE_BETWEEN_MONOCLE, override_default_space_between_monocle},
    {IDENT_MONOCLE_BORDER_WIDTH, override_default_monocle_border_width},
    {IDENT_MINIMIZED_POSITION, override_default_minimized_position},
    {IDENT_MINIMIZED_HEIGHT_INC, override_default_minimized_height_inc},
    {IDENT_GAP_INC_SIZE, override_default_gap_inc},
    {IDENT_NEW_FOCUS_START, override_default_focus_start}
};

typedef struct IdentifierActionPair {
    IdentifierType type;
    KeybindActionFn fn;
} IdentifierActionPair;

static const IdentifierActionPair ident_type_to_action_fn[] = {
    {IDENT_CLOSE_WIN, action_close_win},
    {IDENT_OPEN_TERM, action_open_term},
    {IDENT_WIN_DOWN, action_win_down},
    {IDENT_WIN_UP, action_win_up},
    {IDENT_WIN_LEFT, action_win_left},
    {IDENT_WIN_RIGHT, action_win_right},
    {IDENT_MOVE_WIN_DOWN, action_win_move_down},
    {IDENT_MOVE_WIN_UP, action_win_move_up},
    {IDENT_MOVE_WIN_LEFT, action_win_move_left},
    {IDENT_MOVE_WIN_RIGHT, action_win_move_right},
    {IDENT_ENTER_RESIZE, action_enter_resize},
    {IDENT_EXIT_RESIZE, action_exit_resize},
    {IDENT_RESIZE_DOWN, action_resize_down},
    {IDENT_RESIZE_UP, action_resize_up},
    {IDENT_RESIZE_LEFT, action_resize_left},
    {IDENT_RESIZE_RIGHT, action_resize_right},
    {IDENT_TOGGLE_FULLSCREEN, action_toggle_fullscreen},
    {IDENT_TOGGLE_FLOAT, action_toggle_float},
    {IDENT_TOGGLE_FLOAT_FOCUS, action_toggle_floaters_focus},
    {IDENT_SWITCH_FLOAT_FOCUS, action_switch_float_focus},
    {IDENT_MINIMIZE_WIN, action_minimize_window},
    {IDENT_TOGGLE_MINIMIZE_FOCUS, action_toggle_minimize_focus},
    {IDENT_TILE_MASTER_LEFT, action_tile_master_left},
    {IDENT_TILE_MASTER_RIGHT, action_tile_master_right},
    {IDENT_TILE_SIMPLE_VERTICAL, action_tile_simple_vertical},
    {IDENT_TILE_SIMPLE_HORIZONTAL, action_tile_simple_horizontal},
    {IDENT_TILE_MONOCLE, action_tile_monocle},
    {IDENT_TILE_MASTER_LEFT_MONOCLE, action_tile_master_left_monocle},
    {IDENT_TILE_MASTER_RIGHT_MONOCLE, action_tile_master_right_monocle},
    {IDENT_TILE_MASTER_MASTER_LEFT, action_tile_master_master_left},
    {IDENT_TILE_MASTER_MASTER_RIGHT, action_tile_master_master_right},
    {IDENT_SWITCH_TO_W1, action_switch_to_workspace_one},
    {IDENT_SWITCH_TO_W2, action_switch_to_workspace_two},
    {IDENT_SWITCH_TO_W3, action_switch_to_workspace_three},
    {IDENT_SWITCH_TO_W4, action_switch_to_workspace_four},
    {IDENT_SWITCH_TO_W5, action_switch_to_workspace_five},
    {IDENT_SWITCH_TO_W6, action_switch_to_workspace_six},
    {IDENT_SWITCH_TO_W7, action_switch_to_workspace_seven},
    {IDENT_SWITCH_TO_W8, action_switch_to_workspace_eight},
    {IDENT_SWITCH_TO_W9, action_switch_to_workspace_nine},
    {IDENT_SWITCH_TO_W10, action_switch_to_workspace_ten},
    {IDENT_MOVE_TO_W1, action_move_to_workspace_one},
    {IDENT_MOVE_TO_W2, action_move_to_workspace_two},
    {IDENT_MOVE_TO_W3, action_move_to_workspace_three},
    {IDENT_MOVE_TO_W4, action_move_to_workspace_four},
    {IDENT_MOVE_TO_W5, action_move_to_workspace_five},
    {IDENT_MOVE_TO_W6, action_move_to_workspace_six},
    {IDENT_MOVE_TO_W7, action_move_to_workspace_seven},
    {IDENT_MOVE_TO_W8, action_move_to_workspace_eight},
    {IDENT_MOVE_TO_W9, action_move_to_workspace_nine},
    {IDENT_MOVE_TO_W10, action_move_to_workspace_ten},
    {IDENT_RESTART_MANAGER, action_restart_manager},
    {IDENT_QUIT_MANAGER, action_quit_manager},
    {IDENT_UNMAP_WORKSPACE, action_unmap_all_workspace_windows},
    {IDENT_MAP_WORKSPACE, action_map_all_workspace_windows},
    {IDENT_MAP_LATEST_UNMAP, action_map_latest_unmap},
    {IDENT_UNMAP_WINDOW, action_unmap_window},
    {IDENT_GAP_INC, action_gap_inc},
    {IDENT_GAP_DEC, action_gap_dec},
    {IDENT_FLOAT_ALL_TILED, action_float_all_tiled},
    {IDENT_TILE_ALL_FLOAT, action_tile_all_float},
    {IDENT_MINIMIZE_ALL_WINDOWS, action_minimize_all},
    {IDENT_UNMINIMIZE_ALL_WINDOWS, action_unminimize_all},
    {IDENT_MINIMIZE_LEFT, action_minimize_left},
    {IDENT_MINIMIZE_RIGHT, action_minimize_right},
    {IDENT_MINIMIZE_TOP, action_minimize_top},
    {IDENT_MINIMIZE_BOTTOM, action_minimize_bottom},
    {IDENT_MINIMIZE_INC, action_inc_minimized_size},
    {IDENT_MINIMIZE_DEC, action_dec_minimized_size},
    {IDENT_MOVE_WIN_MONITOR_DOWN, action_move_win_monitor_down},
    {IDENT_MOVE_WIN_MONITOR_UP, action_move_win_monitor_up},
    {IDENT_MOVE_WIN_MONITOR_LEFT, action_move_win_monitor_left},
    {IDENT_MOVE_WIN_MONITOR_RIGHT, action_move_win_monitor_right},
    {IDENT_FOCUS_MODE_FOCUS, action_change_to_focus_mode_focus},
    {IDENT_FOCUS_MODE_POINTER, action_change_to_focus_mode_pointer},
    {IDENT_CYCLE_TILED_FORWARD, action_cycle_tiled_forward}, 
    {IDENT_CYCLE_TILED_BACKWARD, action_cycle_tiled_backward},
    {IDENT_NEW_FOCUS_START_ADJACENT, action_new_focus_start_adjacent},
    {IDENT_NEW_FOCUS_START_END, action_new_focus_start_end},
    {IDENT_NEW_FOCUS_START_ADJACENT, action_new_focus_start_adjacent},
    {IDENT_NEXT_FOCUS_ON_CLOSE_FOCUS_STACK, action_next_focus_on_close_use_stack},
    {IDENT_NEXT_FOCUS_ON_CLOSE_NEXT, action_next_focus_on_close_next}
};

KeybindActionFn get_action_fn(IdentifierType type) {
    for (int I = 0; I < ARRAY_SIZE(ident_type_to_action_fn); ++I)
        if (ident_type_to_action_fn[I].type == type)
            return ident_type_to_action_fn[I].fn;
    return action_noop;
}

typedef struct IdentifierTypeMinimizedPositionPair {
    IdentifierType type;
    MinimizedPosition position;
} IdentifierTypeMinimizedPositionPair;

static const IdentifierTypeMinimizedPositionPair ident_type_to_minimized_position[] = {
    {IDENT_TOP, MINIMIZED_TOP},
    {IDENT_BOTTOM, MINIMIZED_BOTTOM},
    {IDENT_LEFT, MINIMIZED_LEFT},
    {IDENT_RIGHT, MINIMIZED_RIGHT}
};

MinimizedPosition get_minimized_position_from_ident_type(const Token* token) {
    for (int I = 0; I < ARRAY_SIZE(ident_type_to_minimized_position); ++I) 
        if (ident_type_to_minimized_position[I].type == get_token_identifier_type(token))
            return ident_type_to_minimized_position[I].position;
    return MINIMIZED_POSITION_UNKNOWN;
}

typedef struct IdentifierTypeFocusStartPositionPair {
    IdentifierType type;
    FocusStart position;
} IdentifierTypeFocusStartPositionPair;

static const IdentifierTypeFocusStartPositionPair ident_type_to_focus_start_position[] = {
    {IDENT_ADJACENT, FOCUS_START_ADJACENT},
    {IDENT_END, FOCUS_START_END}
};

FocusStart get_focus_start_position_from_ident_type(const Token* token) {
    for (int I = 0; I < ARRAY_SIZE(ident_type_to_focus_start_position); ++I) 
        if (ident_type_to_focus_start_position[I].type == get_token_identifier_type(token))
            return ident_type_to_focus_start_position[I].position;
    return FOCUS_START_UNKNOWN;
}

typedef struct IdentifierTypeFocusEndPositionPair {
    IdentifierType type;
    FocusEnd position;
} IdentifierTypeFocusEndPositionPair;

static const IdentifierTypeFocusEndPositionPair ident_type_to_focus_end_position[] = {
    {IDENT_USE_STACK, FOCUS_END_FOCUS_STACK},
    {IDENT_NEXT, FOCUS_END_NEXT}
};

FocusEnd get_focus_end_position_from_ident_type(const Token* token) {
    for (int I = 0; I < ARRAY_SIZE(ident_type_to_focus_end_position); ++I) 
        if (ident_type_to_focus_end_position[I].type == get_token_identifier_type(token))
            return ident_type_to_focus_end_position[I].position;
    return FOCUS_END_UNKNOWN;
}

_Bool is_valid(const Token* token, const IdentifierType lookup_array[], int n) {
    if (token == NULL) return false;
    for (int i = 0; i < n; ++i) {
        if (get_token_identifier_type(token) == lookup_array[i])
            return true;
    }
    return false;
}

_Bool is_define_valid(const Token* token) {
    if (token == NULL) return false;
    return is_valid(token, valid_defines, ARRAY_SIZE(valid_defines));
}

_Bool is_set_valid(const Token* token) {
    if (token == NULL) return false;
    return is_valid(token, valid_sets, ARRAY_SIZE(valid_sets));
}

_Bool is_color_valid(const Token* token) {
    if (token == NULL) return false;
    return is_valid(token, valid_colors, ARRAY_SIZE(valid_colors));
}

_Bool is_layout_valid(const Token* token) {
    if (token == NULL) return false;
    return is_valid(token, valid_layouts, ARRAY_SIZE(valid_layouts));
}

_Bool is_declare_valid(const Token* token) {
    if (token == NULL) return false;
    return is_valid(token, valid_declares, ARRAY_SIZE(valid_declares));
}

_Bool is_bind_valid(const Token* token) {
    if (token == NULL) return false;
    return is_valid(token, valid_binds, ARRAY_SIZE(valid_binds));
}

_Bool is_minimized_position_identifier(const Token* token) {
    if (token == NULL) return false;
    IdentifierType type = get_token_identifier_type(token);
    return type == IDENT_TOP 
        || type == IDENT_BOTTOM
        || type == IDENT_LEFT
        || type == IDENT_RIGHT;
}

_Bool is_focus_start_position_identifier(const Token* token) {
    if (token == NULL) return false;
    IdentifierType type = get_token_identifier_type(token);
    return type == IDENT_ADJACENT 
        || type == IDENT_END;
}

_Bool is_focus_end_position_identifier(const Token* token) {
    if (token == NULL) return false;
    IdentifierType type = get_token_identifier_type(token);
    return type == IDENT_USE_STACK 
        || type == IDENT_NEXT;
}

_Bool is_cursor(const Token* token) {
    if (token == NULL) return false;
    for (int i = 0; i < ARRAY_SIZE(valid_cursors); ++i) 
        if (get_token_identifier_type(token) == valid_cursors[i])
            return true;
    return false;
}

Cursor get_cursor(const Token* token) {
    const IdentTypeCursorPair ident_type_to_cursor[] = {
        {IDENT_CURSOR_STANDARD_POINTER, cursors.standard_pointer},
        {IDENT_CURSOR_TEXT_INSERTION, cursors.text_insertion},
        {IDENT_CURSOR_BUSY, cursors.busy},
        {IDENT_CURSOR_HAND_SHAPE, cursors.hand_shape},
        {IDENT_CURSOR_CROSSHAIR, cursors.crosshair},
        {IDENT_CURSOR_FOUR_DF_RESIZE, cursors.four_df_resize},
        {IDENT_CURSOR_HORIZONTAL_RESIZE, cursors.horizontal_resize},
        {IDENT_CURSOR_VERTICAL_RESIZE, cursors.vertical_resize},
        {IDENT_CURSOR_TOP_RESIZE, cursors.top_resize},
        {IDENT_CURSOR_BOTTOM_RESIZE, cursors.bottom_resize},
        {IDENT_CURSOR_LEFT_RESIZE, cursors.left_resize},
        {IDENT_CURSOR_RIGHT_RESIZE, cursors.right_resize},
        {IDENT_CURSOR_TOP_LEFT_RESIZE, cursors.top_left_resize},
        {IDENT_CURSOR_TOP_RIGHT_RESIZE, cursors.top_right_resize},
        {IDENT_CURSOR_BOTTOM_LEFT_RESIZE, cursors.bottom_left_resize},
        {IDENT_CURSOR_BOTTOM_RIGHT_RESIZE, cursors.bottom_right_resize},
    };

    for (int i = 0; i < ARRAY_SIZE(ident_type_to_cursor); ++i) 
        if (ident_type_to_cursor[i].type == get_token_identifier_type(token))
            return ident_type_to_cursor[i].cursor;

    return cursors.standard_pointer;
}

OverrideDefaultFn get_override_fn(const Token* token) {
    if (token == NULL) return override_noop;

    for (int i = 0; i < ARRAY_SIZE(ident_type_to_override_fn); ++i)
        if (ident_type_to_override_fn[i].type == get_token_identifier_type(token))
            return ident_type_to_override_fn[i].fn;
    return override_noop;
}

int reconcile_integer_value(const Token* token) {
    if (token == NULL) return -1;

    int n = get_token_integer(token);

    if (token->percentage == true 
        && get_token_percentage_mode(token) 
        == PERCENTAGE_MODE_WIDTH
    ) {
       return (float)n/100 * get_min_width_of_monitors();
    }

    else if (token->percentage == true 
        && get_token_percentage_mode(token) 
        == PERCENTAGE_MODE_HEIGHT
    ) {
        return (float)n/100 * get_min_height_of_monitors();
    }

    return n;
}

void reconcile_ident_value(
    OverrideParameters* op,
    const Token* token,
    int* n,
    Layout* layout,
    Cursor* cursor, 
    const char** term_name
) {
    if (op == NULL 
        || token == NULL 
        || layout == NULL 
        || cursor == NULL 
        || term_name == NULL
    ) return;

    if (is_minimized_position_identifier(token))
        *n = get_minimized_position_from_ident_type(token);

    else if (is_focus_start_position_identifier(token))
        *n = get_focus_start_position_from_ident_type(token);

    else if (is_focus_end_position_identifier(token))
        *n = get_focus_start_position_from_ident_type(token);

    else if (is_layout_valid(token))
        *layout = get_layout_from_ident_type(token);

    else if (is_cursor(token))
        *cursor = get_cursor(token);

    else {
        const char* data_copy = string_data_copy(get_token_identifier(token));
        if (data_copy != NULL) {
            *term_name = data_copy;
            op->manage_term_name = true;
        }
    }
}

void reconcile_string(
    OverrideParameters* op,
    const Token* token,
    const char** term_name,
    const char** color_spec
) {
    if (op == NULL
        || token == NULL 
        || term_name == NULL 
        || color_spec == NULL
    ) return;

    DefinitionType dt = get_override_definition_type(op);

    const char* data_copy = copy_lexeme(token);
    if (dt == DT_COLOR) {
        *color_spec = data_copy;
        op->manage_color_spec = true;
        return;
    }

    *term_name = data_copy;
    op->manage_term_name = true;
}

ConfigError fill_override_parameters(OverrideParameters* op, Token* token) {
    if (token == NULL) return create_config_error(ANGEL_BAD_TOKEN, -1, -1);

    int n = -1;
    Layout layout = ANGEL_LAYOUT_UNKNOWN;
    Cursor cursor = (Cursor)-1;
    const char* term_name = NULL;
    const char* color_spec = "";

    if (token_is_string(token))
        reconcile_string(
            op,
            token,
            &term_name,
            &color_spec
        );

    else if (token_is_integer(token))
        n = reconcile_integer_value(
            token
        );

    else if (token_is_ident(token))
        reconcile_ident_value(
            op,
            token,
            &n,
            &layout,
            &cursor,
            &term_name
        );

    else
        return create_error_skip_statement(
            token, 
            ANGEL_UNEXPECTED_TOKEN
        );

    set_override_parameters(
        op,
        n,
        layout,
        cursor,
        term_name,
        color_spec
    );

    return create_config_error(
        ANGEL_GOOD,
        get_token_line_no(token),
        get_token_col_no(token)
    );
}

_Bool skip_block(Token* token) {
    int lbc = 1, rbc = 0;
    get_next_token_print_error(token);
    while (!token_is_eof(token) && rbc != lbc) {
        get_next_token_print_error(token);

        if (token_is_lbrace(token)) ++lbc;
        else if (token_is_rbrace(token)) ++rbc;
    }

    if (token_is_rbrace(token))
        get_next_token_print_error(token);

    else if (token_is_eof(token)) {
        ConfigError err = create_config_error(
            ANGEL_UNEXPECTED_EOF,
            get_token_line_no(token),
            get_token_col_no(token)
        );

        print_error_message(&err);
    }

    return true;
}

_Bool try_skip_block(Token* token) {
    return token_is_lbrace(token) ? skip_block(token) : false;
}

void skip_to_next_statement(Token* token) {
    while (!token_is_delimeter(token) 
            && !token_is_lbrace(token) 
            && !token_is_eof(token)
    ) {
        get_next_token_print_error(token);
    }

    if (token_is_eof(token)) 
        return;

    if (!try_skip_block(token))
        get_next_token_print_error(token);
}
