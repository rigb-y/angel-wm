/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_TERMINAL_H
#define ANGEL_TERMINAL_H

typedef struct Terminal Terminal;

extern Terminal terminal;

typedef struct Terminal {
    const char* term_name;
} Terminal;

void set_term_name(const char*);
const char* get_term_name();

void spawn_terminal();

#endif
