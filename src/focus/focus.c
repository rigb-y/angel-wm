/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "focus.h"
#include "utils.h"

#include <stdbool.h>

static FocusMode focus_mode = FOCUS_MODE_POINTER;

#define MAKE_VALID_FOCUS_MODES(Z) FOCUS_MODE_##Z,
static const FocusMode valid_focus_modes[] = {
    FML(MAKE_VALID_FOCUS_MODES) 
};
#undef MAKE_VALID_FOCUS_MODES

FocusMode get_focus_mode() {
    return focus_mode;
}

_Bool focus_mode_pointer() {
    return focus_mode == FOCUS_MODE_POINTER;
}

_Bool focus_mode_focus() {
    return focus_mode == FOCUS_MODE_FOCUS;
}

void set_focus_mode(FocusMode mode) {
    focus_mode = mode;
}

_Bool focus_mode_valid(FocusMode mode) {
    for (int I = 0; I < ARRAY_SIZE(valid_focus_modes); ++I)
        if (valid_focus_modes[I] == mode)
            return true;
    return false;
}
