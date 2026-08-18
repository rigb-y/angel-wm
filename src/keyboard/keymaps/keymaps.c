/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "keymaps.h"
#include "keyboard.h"
#include "events.h"
#include "utils.h"

#define XK_TECHNICAL

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdlib.h>

KeyMaps key_maps = {0};

static void action_bind_list_insert(ActionBind**, ActionBind*);
static void destroy_action_bind_list(ActionBind*);
static void destroy_command_bind_resources(CommandBind*);
static void destroy_command_bind_list(CommandBind*);

const DefaultBind default_binds[] = {
    {XK_q, DEFAULT_MOD, action_close_win},
    {XK_Return, DEFAULT_MOD, action_open_term},
    {XK_h, DEFAULT_MOD, action_win_down},
    {XK_j, DEFAULT_MOD, action_win_left},
    {XK_k, DEFAULT_MOD, action_win_up},
    {XK_l, DEFAULT_MOD, action_win_right},
    {XK_h, DEFAULT_MOD | ALT, action_move_win_monitor_left},
    {XK_j, DEFAULT_MOD | ALT, action_move_win_monitor_down},
    {XK_k, DEFAULT_MOD | ALT, action_move_win_monitor_up},
    {XK_l, DEFAULT_MOD | ALT, action_move_win_monitor_right},
    {XK_h, DEFAULT_MOD | SHIFT, action_win_move_left},
    {XK_j, DEFAULT_MOD | SHIFT, action_win_move_down},
    {XK_k, DEFAULT_MOD | SHIFT, action_win_move_up},
    {XK_l, DEFAULT_MOD | SHIFT, action_win_move_right},
    {XK_m, DEFAULT_MOD, action_enter_resize},
    {XK_m, DEFAULT_MOD, action_exit_resize},
    {XK_h, DEFAULT_MOD | CONTROL, action_resize_left},
    {XK_j, DEFAULT_MOD | CONTROL, action_resize_down},
    {XK_k, DEFAULT_MOD | CONTROL, action_resize_up},
    {XK_l, DEFAULT_MOD | CONTROL, action_resize_right},
    {XK_f, DEFAULT_MOD, action_toggle_fullscreen},
    {XK_space, DEFAULT_MOD, action_toggle_float},
    {XK_space, DEFAULT_MOD | ALT, action_toggle_floaters_focus},
    {XK_b, DEFAULT_MOD, action_switch_float_focus},
    {XK_c, DEFAULT_MOD, action_minimize_window},
    {XK_Tab, DEFAULT_MOD, action_toggle_minimize_focus},
    {XK_1, DEFAULT_MOD | ALT, action_tile_master_left},
    {XK_2, DEFAULT_MOD | ALT, action_tile_master_right},
    {XK_3, DEFAULT_MOD | ALT, action_tile_simple_vertical},
    {XK_4, DEFAULT_MOD | ALT, action_tile_simple_horizontal},
    {XK_5, DEFAULT_MOD | ALT, action_tile_monocle},
    {XK_6, DEFAULT_MOD | ALT, action_tile_master_left_monocle},
    {XK_7, DEFAULT_MOD | ALT, action_tile_master_right_monocle},
    {XK_8, DEFAULT_MOD | ALT, action_tile_master_master_left},
    {XK_9, DEFAULT_MOD | ALT, action_tile_master_master_right},
    {XK_1, DEFAULT_MOD, action_switch_to_workspace_one},
    {XK_2, DEFAULT_MOD, action_switch_to_workspace_two},
    {XK_3, DEFAULT_MOD, action_switch_to_workspace_three},
    {XK_4, DEFAULT_MOD, action_switch_to_workspace_four},
    {XK_5, DEFAULT_MOD, action_switch_to_workspace_five},
    {XK_6, DEFAULT_MOD, action_switch_to_workspace_six},
    {XK_7, DEFAULT_MOD, action_switch_to_workspace_seven},
    {XK_8, DEFAULT_MOD, action_switch_to_workspace_eight},
    {XK_9, DEFAULT_MOD, action_switch_to_workspace_nine},
    {XK_0, DEFAULT_MOD, action_switch_to_workspace_ten},
    {XK_1, DEFAULT_MOD | SHIFT, action_move_to_workspace_one},
    {XK_2, DEFAULT_MOD | SHIFT, action_move_to_workspace_two},
    {XK_3, DEFAULT_MOD | SHIFT, action_move_to_workspace_three},
    {XK_4, DEFAULT_MOD | SHIFT, action_move_to_workspace_four},
    {XK_5, DEFAULT_MOD | SHIFT, action_move_to_workspace_five},
    {XK_6, DEFAULT_MOD | SHIFT, action_move_to_workspace_six},
    {XK_7, DEFAULT_MOD | SHIFT, action_move_to_workspace_seven},
    {XK_8, DEFAULT_MOD | SHIFT, action_move_to_workspace_eight},
    {XK_9, DEFAULT_MOD | SHIFT, action_move_to_workspace_nine},
    {XK_0, DEFAULT_MOD | SHIFT, action_move_to_workspace_ten},
    {XK_r, DEFAULT_MOD | SHIFT, action_restart_manager},
    {XK_x, DEFAULT_MOD | SHIFT, action_quit_manager},
    {XK_u, DEFAULT_MOD | CONTROL | SHIFT, action_unmap_all_workspace_windows},
    {XK_m, DEFAULT_MOD | CONTROL | SHIFT, action_map_all_workspace_windows},
    {XK_u, DEFAULT_MOD | SHIFT, action_unmap_window},
    {XK_m, DEFAULT_MOD | SHIFT, action_map_latest_unmap},
    {XK_u, DEFAULT_MOD, action_gap_dec},
    {XK_i, DEFAULT_MOD, action_gap_inc}, 
    {XK_space, DEFAULT_MOD | ALT | SHIFT, action_float_all_tiled},
    {XK_space, DEFAULT_MOD | SHIFT, action_tile_all_float},
    {XK_c, DEFAULT_MOD | ALT | SHIFT, action_minimize_all},
    {XK_c, DEFAULT_MOD | SHIFT, action_unminimize_all},
    {XK_1, DEFAULT_MOD | CONTROL, action_minimize_left},
    {XK_2, DEFAULT_MOD | CONTROL, action_minimize_bottom},
    {XK_3, DEFAULT_MOD | CONTROL, action_minimize_top},
    {XK_4, DEFAULT_MOD | CONTROL, action_minimize_right},
    {XK_u, DEFAULT_MOD | ALT, action_dec_minimized_size},
    {XK_i, DEFAULT_MOD | ALT, action_inc_minimized_size},
    {XK_f, DEFAULT_MOD | SHIFT, action_change_to_focus_mode_focus},
    {XK_p, DEFAULT_MOD | SHIFT, action_change_to_focus_mode_pointer},
    {XK_u, DEFAULT_MOD, action_cycle_tiled_forward},
    {XK_u, DEFAULT_MOD | SHIFT, action_cycle_tiled_forward},
    {XK_a, DEFAULT_MOD | SHIFT, action_new_focus_start_adjacent},
    {XK_e, DEFAULT_MOD | SHIFT, action_new_focus_start_end},
    {XK_a, DEFAULT_MOD | ALT, action_next_focus_on_close_use_stack},
    {XK_e, DEFAULT_MOD | ALT, action_next_focus_on_close_next}
};

void set_action_bind(ActionBind* action_bind, KeyMap map, KeybindActionFn action) {
    if (action_bind == NULL) return;
    action_bind->map = map;
    action_bind->action = action;
}

ActionBind* create_action_bind(KeyMap map, KeybindActionFn action) {
    ActionBind* action_bind;
    if ((action_bind = calloc(1, sizeof(ActionBind))) == NULL)
        return NULL;

    set_action_bind(action_bind, map, action);
    return action_bind;
}

KeybindActionFn get_keybind_action(ActionBind* action_bind) {
    if (action_bind == NULL) return action_noop;
    return action_bind->action;
}

KeyMap create_keymap(KeySym key_sym, unsigned int state) {
    KeyMap map = {0};

    map.key_sym = key_sym;
    map.state = state;

    return map;
}

void action_bind_list_insert(ActionBind** head, ActionBind* bind) {
    if (bind == NULL) return;

    if (*head == NULL) {
        *head = bind;
        (*head)->next = NULL;
        return;
    }

    ActionBind* next = *head;
    *head = bind;
    (*head)->next = next;
}

void default_bind_insert(ActionBind* map) {
    action_bind_list_insert(&key_maps.default_head, map);
}

void default_bind_remove(KeybindActionFn action) {
    if (action == NULL || key_maps.default_head == NULL) return;

    if (get_keybind_action(key_maps.default_head) == action) {
        ActionBind* next = key_maps.default_head->next;
        free(key_maps.default_head);
        key_maps.default_head = next;
        return;
    }

    ActionBind* curr = key_maps.default_head;
    ActionBind* prev = curr;
    while (curr != NULL) {
        if (get_keybind_action(curr) == action)
            break;
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) return;

    prev->next = curr->next;
    free(curr);
}

void remap_insert(ActionBind* map) {
    action_bind_list_insert(&key_maps.remap_head, map);
}

void set_command_bind(CommandBind* bind, KeyMap map, const char* command) {
    if (bind == NULL) return;
    bind->map = map;
    bind->command = command;
}

void command_bind_create(KeyMap map, const char* command) {
    if (command == NULL) return;
    CommandBind* command_bind;
    if ((command_bind = calloc(1, sizeof(CommandBind))) == NULL)
        return;

    set_command_bind(command_bind, map, command);
    command_insert(command_bind);
}

void command_insert(CommandBind* bind) {
    if (bind == NULL) return;

    if (key_maps.command_head == NULL) {
        key_maps.command_head = bind;
        return;
    }

    CommandBind* next = key_maps.command_head;
    key_maps.command_head = bind;
    bind->next = next;
}

const char* command_find(KeyMap map) {
    CommandBind* curr = key_maps.command_head;
    while (curr != NULL && !are_keymaps_same(&curr->map, &map))
        curr = curr->next; 
    return curr != NULL ? curr->command : NULL;
}

KeybindActionFn action_find(const KeyMap* map, BindSearchLocation location) {
    ActionBind* curr = location == SEARCH_DEFAULTS 
        ? key_maps.default_head 
        : key_maps.remap_head;

    while (curr != NULL && !are_keymaps_same(&curr->map, map))
        curr = curr->next; 
    return curr != NULL ? curr->action : NULL;
}

_Bool are_keymaps_same(const KeyMap* first, const KeyMap* second) {
    if (first == NULL || second == NULL) return false;

    return first->key_sym == second->key_sym &&
        first->state == second->state;
}

void set_default_binds() {
    for (int i = 0; i < ARRAY_SIZE(default_binds); ++i) {
        default_bind_insert(
            create_action_bind( 
                create_keymap(
                    default_binds[i].key_sym,
                    default_binds[i].state
                ),
                default_binds[i].action
            )
        );
    }
}

void install_remap(const Remap* remap) {
    remap_insert(
        create_action_bind( 
            create_keymap(
                remap->key_sym,
                remap->state
            ),
            remap->action
        )
    );
    default_bind_remove(remap->action);
}

void install_remaps(const Remap remaps[], int n) {
    for (int i = 0; i < n; ++i) {
        install_remap(&remaps[i]);
    }
}

void destroy_action_bind_list(ActionBind* head) {
    if (head == NULL) return;

    ActionBind* map = head;
    while (map != NULL) {
        ActionBind* tmp = map;
        map = map->next;
        free(tmp);
    }
}

void destroy_command_bind_resources(CommandBind* command) {
    if (command == NULL) return;
    free((char*)command->command);
    command->command = NULL;
}

void destroy_command_bind_list(CommandBind* head) {
    if (head == NULL) return;

    CommandBind* command = key_maps.command_head;
    while (command != NULL) {
        CommandBind* tmp = command;
        command = command->next;
        destroy_command_bind_resources(tmp);
        free(tmp);
    }
}

void destroy_keymaps() {
    destroy_action_bind_list(key_maps.default_head);
    destroy_action_bind_list(key_maps.remap_head);
    destroy_command_bind_list(key_maps.command_head);

    key_maps.default_head = NULL;
    key_maps.remap_head = NULL;
    key_maps.command_head = NULL;
}

_Bool is_drag_win(int button, unsigned int state) {
    return button == Button1 && state == get_modifier();
}

_Bool is_mouse_resize(int button, unsigned int state) {
    return button == Button3 && state == get_modifier();
}
