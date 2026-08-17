/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "screen.h"

int get_screen_count() {
    return XScreenCount(dp);
}
int get_default_screen_number() {
    return XDefaultScreen(dp);
}

Screen* get_default_screen() {
    return XDefaultScreenOfDisplay(dp);
}

int get_screen_width(int screen_number) {
    return XDisplayWidth(dp, screen_number);
}

int get_screen_height(int screen_number) {
    return XDisplayHeight(dp, screen_number);
}

int get_default_screen_width() {
    return XDisplayWidth(dp, get_default_screen_number());
}

int get_default_screen_height() {
    return XDisplayHeight(dp, get_default_screen_number());
}
