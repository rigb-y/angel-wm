/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "terminal.h"
#include "shell.h"

Terminal terminal = {0};

void set_term_name(const char* name) {
    terminal.term_name = name;
}

const char* get_term_name() {
    return terminal.term_name;
}

void spawn_terminal() {
    exec_program(get_term_name());
}
