/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_KEYBOARD_H
#define ANGEL_KEYBOARD_H

#include <X11/Xlib.h>

#define UNKNOWN_MODIFIER 0
#define SUPER Mod4Mask 
#define ALT Mod1Mask
#define CONTROL ControlMask
#define SHIFT ShiftMask

#define DEFAULT_MOD SUPER

extern const KeySym key_syms[];
extern KeyCode key_codes[];

int get_modifier();
void set_modifier(int);
KeySym get_keysym_from_str(const char*);
int get_modifier_from_str(const char*);
void init_keyboard();
void establish_root_passive_key_grab(KeySym, unsigned int);
void establish_root_passive_key_grabs();
void get_keycodes();
KeyCode get_keycode(KeySym);
KeySym get_keysym(KeyCode);
void ungrab_all_root_passive_grabs();
void refresh_keyboard(const XMappingEvent*);

#endif
