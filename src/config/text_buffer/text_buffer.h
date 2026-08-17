/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_TEXT_BUFFER_H
#define ANGEL_TEXT_BUFFER_H

extern char* config_path;
extern int curr_pos;
extern int line_no;
extern int col_no; 

char* get_line(int);
_Bool config_exists();
_Bool init_buffer();

const char* buffer_get_next_char();
const char* buffer_get_curr_char();
const char* buffer_get_prev_char();
_Bool buffer_advance();
_Bool buffer_retreat();

_Bool buffer_eof();
void buffer_destroy();

void __debug_dump_all_lines(); 

#endif
