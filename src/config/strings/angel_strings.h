/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_ANGEL_STRINGS_H
#define ANGEL_ANGEL_STRINGS_H

#include <stddef.h>

typedef struct string {
    char* data;
    size_t size;
    size_t capacity;
    size_t curr_pos;
} string;

string create_string();
string create_string_with_data(const char*);
size_t string_length(const string*);
size_t string_capacity(const string*);
size_t string_size(const string*);
const char* string_data(const string*);
void string_append(string*, char);
void string_append_chars(string*, const char*, size_t);

void string_destroy(string*);
void string_refresh(string*);
string string_copy(string*);

const char* string_data_copy(const string*);

#endif
