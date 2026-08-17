/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_SCREEN_H
#define ANGEL_SCREEN_H

#include <X11/Xlib.h>

int get_screen_count();
int get_default_screen_number();
Screen* get_default_screen();

int get_screen_width(int);
int get_screen_height(int);
int get_default_screen_width();
int get_default_screen_height();

#endif
