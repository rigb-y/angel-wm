/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel_strings.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

string create_string() {
    string str = {0};
    str.size = 0;
    str.capacity = 1;
    str.curr_pos = 0;
    if ((str.data = malloc((sizeof(char) * str.capacity) + 1)) == NULL)
        return str;
    
    str.data[0] = '\0';
    return str;
}

string create_string_with_data(const char* data) {
    if (data == NULL) return create_string();

    string ret = create_string();
    string_append_chars(&ret, data, get_raw_str_size(data));

    return ret;
}

size_t string_length(const string* str) {
    if (str == NULL) return 0;
    return str->size;
}

size_t string_capacity(const string* str) {
    if (str == NULL) return 0;
    return str->capacity;
}

size_t string_size(const string* str) {
    if (str == NULL) return 0;
    return str->size;
}

const char* string_data(const string* str) {
    if (str == NULL || str->data == NULL) return "";
    return str->data;
}

void string_append(string* str, char c) {
    if (str == NULL) return;

    if (str->size == str->capacity) {
        char* re = malloc((sizeof(char) * (str->capacity<<1)) + 1);
        if (re == NULL)
            return;
        memcpy(re, str->data, str->size+1);
        free(str->data);
        str->data = re;
        str->capacity = str->capacity<<1;
    }

    *(str->data + str->curr_pos++) = c;
    *(str->data + str->curr_pos) = '\0';

    ++str->size;
}

void string_append_chars(string* str, const char* from, size_t n) {
    if (str == NULL || from == NULL) return;
    for (size_t i = 0; i < n; ++i)
        string_append(str, from[i]);
}

void string_destroy(string* str) {
    if (str == NULL) return;
    str->capacity = 1;
    str->size = 0;
    str->curr_pos = 0;
    free(str->data);
    str->data = NULL;
}

void string_refresh(string* str) {
    if (str == NULL) return; 
    string_destroy(str);
    *str = create_string();
}

string string_copy(string* from) {
    string into = create_string();
    if (from == NULL) return into;
    free(into.data);

    into.data = malloc((sizeof(char) * from->capacity) + 1);
    if (into.data == NULL) 
        return into;

    into.size = from->size;
    into.capacity = from->capacity;
    into.curr_pos = from->curr_pos;

    memcpy(into.data, from->data, from->size);
    *(into.data + from->size) = '\0';
    return into;
}

const char* string_data_copy(const string* from) {
    if (from == NULL) return NULL;
    char* ret;
    if ((ret = malloc((sizeof(char) * string_capacity(from)) + 1)) == NULL)
        return NULL;

    memcpy(ret, string_data(from), string_size(from));
    ret[string_size(from)] = '\0';

    return ret;
}
