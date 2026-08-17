/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_POINTER_H
#define ANGEL_POINTER_H

typedef struct PointerPosition PointerPosition;
extern PointerPosition pointer_pos;

typedef struct Position Position;

typedef struct PointerPosition {
    int x, y;
} PointerPosition;

void pp_set_x(int);
void pp_set_y(int);
void pp_set_xy(int, int);
int pp_get_x();
int pp_get_y();

_Bool pp_has_moved(const Position*);
Position get_pointer_pos();

#endif
