/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_KEYMAPS_H
#define ANGEL_KEYMAPS_H

#include "types.h"
#include <X11/Xlib.h>

typedef struct KeyMap KeyMap;
typedef struct KeyMaps KeyMaps;
typedef struct ActionBind ActionBind;

extern KeyMaps key_maps;

typedef enum BindSearchLocation {
    SEARCH_DEFAULTS,
    SEARCH_REMAPS
} BindSearchLocation;

typedef struct DefaultBind {
    KeySym key_sym;
    unsigned int state;
    KeybindActionFn action;
} DefaultBind;

extern const DefaultBind default_binds[];

typedef struct Remap {
    KeySym key_sym;
    unsigned int state;
    KeybindActionFn action;
} Remap;

typedef struct KeyMap {
    KeySym key_sym;
    unsigned int state;
} KeyMap;

KeyMap create_keymap(KeySym, unsigned int);
_Bool are_keymaps_same(const KeyMap*, const KeyMap*);

typedef struct ActionBind {
    KeyMap map;
    KeybindActionFn action;

    struct ActionBind* next;
} ActionBind;

void set_action_bind(ActionBind*, KeyMap, KeybindActionFn);
ActionBind* create_action_bind(KeyMap, KeybindActionFn);
KeybindActionFn get_keybind_action(ActionBind*);
KeybindActionFn action_find(const KeyMap* map, BindSearchLocation);

typedef struct CommandBind {
    KeyMap map;
    const char* command;
    struct CommandBind* next;
} CommandBind;

void set_command_bind(CommandBind*, KeyMap, const char*);
void command_bind_create(KeyMap, const char*);
void command_insert(CommandBind*);
const char* command_find(KeyMap);

typedef struct KeyMaps {
    ActionBind* default_head;
    ActionBind* remap_head;
    CommandBind* command_head;
} KeyMaps;

void default_bind_insert(ActionBind*);
void default_bind_remove(KeybindActionFn);
void remap_insert(ActionBind*);
_Bool is_drag_win(int, unsigned int);
_Bool is_mouse_resize(int, unsigned int);

void set_default_binds();
void install_remaps(const Remap[], int n);
void install_remap(const Remap*);

void destroy_keymaps();

#endif
