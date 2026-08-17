/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "text_buffer.h"
#include "logging.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH_SIZE 256

char* config_path = NULL;

int curr_pos = -1;
int line_no = 1;
int col_no = 0;

static size_t buffer_len = 0;
static char* buffer = NULL;

static int prev_line_ending_col_no = 0;

static long int get_buffer_len();
static _Bool build_config_path();
static _Bool create_buffer();
static _Bool populate_buffer();

typedef struct BufferPositionPair {
    int buff_start, buff_end;
} BufferPositionPair;

static void set_buffer_position_pair(BufferPositionPair*, int, int);
static BufferPositionPair create_buffer_position_pair(int, int);
static int buffer_position_start(const BufferPositionPair*);
static int buffer_position_end(const BufferPositionPair*);

typedef struct LinePair {
    int line_no;
    BufferPositionPair buffer_positions;
    struct LinePair* next;
} LinePair;

void set_line_pair(LinePair*, int, BufferPositionPair);
LinePair* create_line_pair(int, BufferPositionPair);
char* construct_line(LinePair*);

void set_buffer_position_pair(BufferPositionPair* bpp, int start, int end) {
    if (bpp == NULL) return;
    bpp->buff_start = start;
    bpp->buff_end = end;
}

BufferPositionPair create_buffer_position_pair(int start, int end) {
    BufferPositionPair bpp = {0};
    set_buffer_position_pair(&bpp, start, end);
    return bpp;
}

int buffer_position_start(const BufferPositionPair* bpp) {
    if (bpp == NULL) return -1;
    return bpp->buff_start;
}

int buffer_position_end(const BufferPositionPair* bpp){
    if (bpp == NULL) return -1;
    return bpp->buff_end;
}

void set_line_pair(LinePair* lp, int line_no, BufferPositionPair bpp) {
    if (lp == NULL) return;
    lp->line_no = line_no;
    lp->buffer_positions = bpp;
    lp->next = NULL;
}

LinePair* create_line_pair(int line_no, BufferPositionPair bpp) {
    LinePair* lp;
    if ((lp = calloc(1, sizeof(LinePair))) == NULL)
        return NULL;

    set_line_pair(lp, line_no, bpp);
    return lp;
}

char* construct_line(LinePair* lp) {
    if (lp == NULL) return NULL;

    int start = buffer_position_start(&lp->buffer_positions);
    int end = buffer_position_end(&lp->buffer_positions);
    if (start < 0 || end >= buffer_len) return NULL;

    long size = (end - start + 1) + 1;

    char* line;
    if ((line = malloc(sizeof(char) * size)) == NULL)
        return NULL;

    memcpy(line, buffer+start, size-1);
    line[size-1] = '\0';

    return line;
}

typedef struct Lines {
    LinePair* head;
} Lines;

static Lines lines = {0};
static void lines_append(LinePair*);
static LinePair* find_line_pair(int);
static void destroy_lines();

void lines_append(LinePair* lp) {
    if (lp == NULL) return;

    if (lines.head == NULL) {
        lines.head = lp;
        lines.head->next = NULL;
        return;
    }

    LinePair* curr = lines.head;
    while (curr->next != NULL) {
        curr = curr->next;
    }

    curr->next = lp;
    lp->next = NULL;
}

LinePair* find_line_pair(int line) {
    if (lines.head == NULL) return NULL;    
    LinePair* curr = lines.head;
    while (curr != NULL && curr->line_no != line) {
        curr = curr->next;
    }
    return curr;
}

void destroy_lines() {
    if (lines.head == NULL) return;

    LinePair* curr = lines.head;
    while (curr != NULL) {
        LinePair* tmp = curr;
        curr = curr->next;
        free(tmp);
    }

    lines.head = NULL;
}

char* get_line(int line) {
    LinePair* line_pair = find_line_pair(line);
    return line_pair != NULL 
        ? construct_line(line_pair) 
        : NULL;
}

_Bool build_config_path() {
    free(config_path);

    const char* home = getenv("HOME");

    if (home == NULL) 
        return false;

    char* path; 
    if ((path = malloc(sizeof(char) * MAX_PATH_SIZE)) == NULL) 
        return false;

    snprintf(path, MAX_PATH_SIZE, "%s/.config/angel/angel.conf", home);

    free(config_path);
    config_path = path;

    return true;
}

_Bool config_exists() {
    if (!build_config_path()) {
        lerr("Failed to find config path");
        return false;
    }

    FILE* config = fopen(config_path, "rb");

    if (config != NULL) {
        fclose(config);
        return true;
    }

    linfo("Couldn't find config, reverting to defaults");
    return false;
}

long int get_buffer_len() {
    FILE* config = fopen(config_path, "rb");
    if (config == NULL) {
        lerr("Failed to read config file");
        return -1;
    }

    int fs_r = fseek(config, 0, SEEK_END);
    if (fs_r != 0) {
        lerr("Something went wrong while reading the config file");
        fclose(config);
        return -1;
    }

    long int size = ftell(config);
    if (size == -1L) {
        lerr("Something went wrong while reading the config file");
        fclose(config);
        return -1;
    }

    fclose(config);
    return (size_t)size;
}

_Bool create_buffer() {
    long int buffer_size = get_buffer_len();
    if (buffer_size == -1)
        return false;

    free(buffer);
    buffer = malloc(sizeof(char) * buffer_size);
    if (buffer == NULL)
        return false;

    buffer_len = (size_t) buffer_size;
    return true;
}

_Bool populate_buffer() {
    FILE* config = fopen(config_path, "rb");
    if (config == NULL) {
        lerr("Failed to open config file for reading");
        return false;
    }

    if (buffer == NULL) {
        fclose(config);
        return false;
    }

    char c; 
    int k = 0, line_start = 0, line_no = 1;
    while ((c = fgetc(config)) != EOF && k != buffer_len) {
        buffer[k] = c;
        if (c == '\n') {
            lines_append(
                create_line_pair(
                    line_no, (BufferPositionPair){line_start, k}
                )
            );
            ++line_no;
            line_start = k+1;
        }
        ++k;
    }

    fclose(config);
    return true;
}

_Bool init_buffer() {
    buffer_destroy();

    int status = config_exists() 
        && create_buffer() 
        && populate_buffer();

    return status;
}

const char* buffer_get_next_char() {
    const char* curr = buffer_get_curr_char();
    if (curr != NULL && *curr == '\n') {
        ++line_no;
        prev_line_ending_col_no = col_no;
        col_no = 0;
    }

    if (!buffer_advance())
        return NULL;

    return buffer_get_curr_char();
}

const char* buffer_get_curr_char() {
    if (buffer_eof() || curr_pos < 0) 
        return NULL;

    return &buffer[curr_pos];
}

const char* buffer_get_prev_char() {
    if (!buffer_retreat())
        return NULL;

    const char* ch =  buffer_get_curr_char();
    if (ch != NULL && *ch == '\n') {
        --line_no;
        col_no = prev_line_ending_col_no;
    }

    return ch;
}

_Bool buffer_advance() {
    if (buffer_eof()) return false;

    ++curr_pos;
    ++col_no;

    return !buffer_eof();
}

_Bool buffer_retreat() {
    if (curr_pos <= 0 || buffer_eof()) 
        return false;

    --col_no;
    --curr_pos;

    return true;
}

_Bool buffer_eof() {
    return curr_pos >= 0 
        && (size_t)curr_pos >= buffer_len;
}

void buffer_destroy() {
    free(buffer);
    free(config_path);
    destroy_lines();

    curr_pos = -1;
    line_no = 1;
    col_no = 1;
    buffer_len = 0;
    buffer = NULL;
    config_path = NULL;
    prev_line_ending_col_no = 0;
}

void __debug_dump_all_lines() {
    LinePair* curr = lines.head;
    while (curr != NULL) {
        char* line = construct_line(curr);
        if (line != NULL)
            printf("%s", line);
        curr = curr->next;
    }
}
