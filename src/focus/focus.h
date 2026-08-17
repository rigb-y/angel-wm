/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_FOCUS_H
#define ANGEL_FOCUS_H

#define FML(X) \
    X(POINTER) \
    X(FOCUS)

#define MAKE_ENUM(Z) FOCUS_MODE_##Z,
typedef enum FocusMode {
    FML(MAKE_ENUM)
} FocusMode;
#undef MAKE_ENUM

FocusMode get_focus_mode();
_Bool focus_mode_pointer();
_Bool focus_mode_focus();
void set_focus_mode(FocusMode);
_Bool focus_mode_valid(FocusMode);

#endif
