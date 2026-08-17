/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "token.h"
#include "angel_strings.h"
#include "utils.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct TokenTypeStringPair {
    TokenType type;
    const char* str;
} TokenTypeStringPair;

typedef struct IdentifierStringPair {
    const char* ident_str;
    IdentifierType type;
} IdentifierStringPair;

static const IdentifierStringPair ident_str_to_ident_type[] = {
    {"exec", IDENT_EXEC},
    {"let", IDENT_LET},
    {"set-background", IDENT_SET_BACKGROUND},
    {"define", IDENT_DEFINE},
    {"set", IDENT_SET},
    {"color", IDENT_COLOR},
    {"layouts", IDENT_LAYOUTS},
    {"bind-command", IDENT_BIND_COMMAND},
    {"declare", IDENT_DECLARE},
    {"bind", IDENT_BIND},
    {"terminal", IDENT_TERMINAL},
    {"root-cursor", IDENT_ROOT_CURSOR},
    {"max-width", IDENT_MAX_WIDTH},
    {"max-height", IDENT_MAX_HEIGHT},
    {"min-width", IDENT_MIN_WIDTH},
    {"min-height", IDENT_MIN_HEIGHT},
    {"gap", IDENT_GAP},
    {"fullscreen-border-width", IDENT_FULLSCREEN_BORDER_WIDTH},
    {"window-resize-inc", IDENT_WINDOW_RESIZE_INC},
    {"float-move-step", IDENT_FLOAT_MOVE_STEP},
    {"default-workspace-layout", IDENT_DEFAULT_WORKSPACE_LAYOUT},
    {"minimized-height", IDENT_MINIMIZED_HEIGHT},
    {"resize-border", IDENT_RESIZE_BORDER},
    {"focused-border", IDENT_FOCUSED_BORDER},
    {"unfocused-border", IDENT_UNFOCUSED_BORDER},
    {"float-border", IDENT_FLOAT_BORDER},
    {"fullscreen-border", IDENT_FULLSCREEN_BORDER},
    {"angel_master_left", IDENT_ANGEL_MASTER_LEFT},
    {"angel_master_right", IDENT_ANGEL_MASTER_RIGHT},
    {"angel_simple_vertical", IDENT_ANGEL_SIMPLE_VERTICAL},
    {"angel_simple_horizontal", IDENT_ANGEL_SIMPLE_HORIZONTAL},
    {"angel_monocle", IDENT_ANGEL_MONOCLE},
    {"angel_master_left_monocle", IDENT_ANGEL_MASTER_LEFT_MONOCLE},
    {"angel_master_right_monocle", IDENT_ANGEL_MASTER_RIGHT_MONOCLE},
    {"angel_master_master_left", IDENT_ANGEL_MASTER_MASTER_LEFT},
    {"angel_master_master_right", IDENT_ANGEL_MASTER_MASTER_RIGHT},
    {"close-win", IDENT_CLOSE_WIN},
    {"open-term", IDENT_OPEN_TERM},
    {"win-down", IDENT_WIN_DOWN},
    {"win-up", IDENT_WIN_UP},
    {"win-left", IDENT_WIN_LEFT},
    {"win-right",  IDENT_WIN_RIGHT},
    {"move-win-down", IDENT_MOVE_WIN_DOWN},
    {"move-win-up", IDENT_MOVE_WIN_UP},
    {"move-win-left", IDENT_MOVE_WIN_LEFT},
    {"move-win-right", IDENT_MOVE_WIN_RIGHT},
    {"enter-resize", IDENT_ENTER_RESIZE},
    {"exit-resize", IDENT_EXIT_RESIZE},
    {"resize-down", IDENT_RESIZE_DOWN},
    {"resize-up", IDENT_RESIZE_UP},
    {"resize-left", IDENT_RESIZE_LEFT},
    {"resize-right", IDENT_RESIZE_RIGHT},
    {"toggle-fullscreen", IDENT_TOGGLE_FULLSCREEN},
    {"toggle-float", IDENT_TOGGLE_FLOAT},
    {"toggle-float-focus", IDENT_TOGGLE_FLOAT_FOCUS},
    {"switch-float-focus", IDENT_SWITCH_FLOAT_FOCUS},
    {"minimize-win", IDENT_MINIMIZE_WIN},
    {"toggle-minimize-focus", IDENT_TOGGLE_MINIMIZE_FOCUS},
    {"tile_master_left", IDENT_TILE_MASTER_LEFT},
    {"tile_master_right", IDENT_TILE_MASTER_RIGHT},
    {"tile_simple_vertical", IDENT_TILE_SIMPLE_VERTICAL},
    {"tile_simple_horizontal", IDENT_TILE_SIMPLE_HORIZONTAL},
    {"tile_monocle", IDENT_TILE_MONOCLE},
    {"tile_master_left_monocle", IDENT_TILE_MASTER_LEFT_MONOCLE},
    {"tile_master_right_monocle", IDENT_TILE_MASTER_RIGHT_MONOCLE},
    {"tile_master_master_left", IDENT_TILE_MASTER_MASTER_LEFT},
    {"tile_master_master_right", IDENT_TILE_MASTER_MASTER_RIGHT},
    {"switch-to-w1", IDENT_SWITCH_TO_W1},
    {"switch-to-w2", IDENT_SWITCH_TO_W2},
    {"switch-to-w3", IDENT_SWITCH_TO_W3},
    {"switch-to-w4", IDENT_SWITCH_TO_W4},
    {"switch-to-w5", IDENT_SWITCH_TO_W5},
    {"switch-to-w6", IDENT_SWITCH_TO_W6},
    {"switch-to-w7", IDENT_SWITCH_TO_W7},
    {"switch-to-w8", IDENT_SWITCH_TO_W8},
    {"switch-to-w9", IDENT_SWITCH_TO_W9},
    {"switch-to-w10", IDENT_SWITCH_TO_W10},
    {"move-to-w1", IDENT_MOVE_TO_W1},
    {"move-to-w2", IDENT_MOVE_TO_W2},
    {"move-to-w3", IDENT_MOVE_TO_W3},
    {"move-to-w4", IDENT_MOVE_TO_W4},
    {"move-to-w5", IDENT_MOVE_TO_W5},
    {"move-to-w6", IDENT_MOVE_TO_W6},
    {"move-to-w7", IDENT_MOVE_TO_W7},
    {"move-to-w8", IDENT_MOVE_TO_W8},
    {"move-to-w9", IDENT_MOVE_TO_W9},
    {"move-to-w10", IDENT_MOVE_TO_W10},
    {"tiled", IDENT_TILED},
    {"float", IDENT_FLOAT},
    {"standard_pointer", IDENT_CURSOR_STANDARD_POINTER},
    {"text_insertion", IDENT_CURSOR_TEXT_INSERTION},
    {"busy", IDENT_CURSOR_BUSY},
    {"hand_shape", IDENT_CURSOR_HAND_SHAPE},
    {"crosshair", IDENT_CURSOR_CROSSHAIR},
    {"four_df_resize", IDENT_CURSOR_FOUR_DF_RESIZE},
    {"horizontal_resize", IDENT_CURSOR_HORIZONTAL_RESIZE},
    {"vertical_resize", IDENT_CURSOR_VERTICAL_RESIZE},
    {"top_resize", IDENT_CURSOR_TOP_RESIZE},
    {"bottom_resize", IDENT_CURSOR_BOTTOM_RESIZE},
    {"left_resize", IDENT_CURSOR_LEFT_RESIZE},
    {"right_resize", IDENT_CURSOR_RIGHT_RESIZE},
    {"top_left_resize", IDENT_CURSOR_TOP_LEFT_RESIZE},
    {"top_right_resize", IDENT_CURSOR_TOP_RIGHT_RESIZE},
    {"bottom_left_resize", IDENT_CURSOR_BOTTOM_LEFT_RESIZE},
    {"restart-manager", IDENT_RESTART_MANAGER},
    {"quit-manager", IDENT_QUIT_MANAGER}, 
    {"unmap-workspace", IDENT_UNMAP_WORKSPACE}, 
    {"map-workspace", IDENT_MAP_WORKSPACE}, 
    {"map-latest-unmap", IDENT_MAP_LATEST_UNMAP}, 
    {"unmap-window", IDENT_UNMAP_WINDOW}, 
    {"gap-inc", IDENT_GAP_INC},
    {"gap-dec", IDENT_GAP_DEC}, 
    {"focused-border-width", IDENT_FOCUSED_BORDER_WIDTH},
    {"unfocused-border-width", IDENT_UNFOCUSED_BORDER_WIDTH},
    {"resize-border-width", IDENT_RESIZE_BORDER_WIDTH},
    {"float-border-width", IDENT_FLOAT_BORDER_WIDTH},
    {"minimized-border-width", IDENT_MINIMIZED_BORDER_WIDTH},
    {"space-between-monocle", IDENT_SPACE_BETWEEN_MONOCLE}, 
    {"monocle-border-width", IDENT_MONOCLE_BORDER_WIDTH},
    {"float-all-tiled", IDENT_FLOAT_ALL_TILED},
    {"tile-all-float", IDENT_TILE_ALL_FLOAT},
    {"minimize-all-windows", IDENT_MINIMIZE_ALL_WINDOWS},
    {"unminimize-all-windows", IDENT_UNMINIMIZE_ALL_WINDOWS},
    {"minimized-position", IDENT_MINIMIZED_POSITION},
    {"top", IDENT_TOP},
    {"bottom", IDENT_BOTTOM},
    {"left", IDENT_LEFT},
    {"right", IDENT_RIGHT},
    {"minimize-top", IDENT_MINIMIZE_TOP},
    {"minimize-bottom", IDENT_MINIMIZE_BOTTOM},
    {"minimize-left", IDENT_MINIMIZE_LEFT},
    {"minimize-right", IDENT_MINIMIZE_RIGHT},
    {"minimize-inc", IDENT_MINIMIZE_INC},
    {"minimize-dec", IDENT_MINIMIZE_DEC},
    {"minimized-height-inc", IDENT_MINIMIZED_HEIGHT_INC},
    {"gap-inc-size", IDENT_GAP_INC_SIZE},
    {"move-win-monitor-down", IDENT_MOVE_WIN_MONITOR_DOWN},
    {"move-win-monitor-up", IDENT_MOVE_WIN_MONITOR_UP},
    {"move-win-monitor-left", IDENT_MOVE_WIN_MONITOR_LEFT},
    {"move-win-monitor-right", IDENT_MOVE_WIN_MONITOR_RIGHT},
    {"focus-mode-focus", IDENT_FOCUS_MODE_FOCUS},
    {"focus-mode-pointer", IDENT_FOCUS_MODE_POINTER},
    {"cycle-tiled-forward", IDENT_CYCLE_TILED_FORWARD},
    {"cycle-tiled-backward", IDENT_CYCLE_TILED_BACKWARD},
    {"new-focus-start", IDENT_NEW_FOCUS_START},
    {"adjacent", IDENT_ADJACENT},
    {"end", IDENT_END},
    {"new-focus-start-adjacent", IDENT_NEW_FOCUS_START_ADJACENT},
    {"new-focus-start-end", IDENT_NEW_FOCUS_START_END},
    {"next-focus-on-close",IDENT_NEXT_FOCUS_ON_CLOSE},
    {"next-focus-on-close-focus-stack",IDENT_NEXT_FOCUS_ON_CLOSE_FOCUS_STACK},
    {"next-focus-on-close-next",IDENT_NEXT_FOCUS_ON_CLOSE_NEXT},
    {"use-stack",IDENT_USE_STACK},
    {"next",IDENT_NEXT}

};

static const TokenTypeStringPair token_type_to_string_map[] = {
    {TOKEN_NULL, "TOKEN_NULL"},
    {TOKEN_COMMENT, "TOKEN_COMMENT"},
    {TOKEN_SEMICOLON, "TOKEN_SEMICOLON"},
    {TOKEN_NEWLINE, "TOKEN_NEWLINE"},
    {TOKEN_COMMA, "TOKEN_COMMA"},
    {TOKEN_IDENT, "TOKEN_IDENT"},
    {TOKEN_STRING, "TOKEN_STRING"},
    {TOKEN_INTEGER, "TOKEN_INTEGER"},
    {TOKEN_RBRACE, "TOKEN_RBRACE"},
    {TOKEN_LBRACE, "TOKEN_LBRACE"},
    {TOKEN_EQUAL, "TOKEN_EQUAL"},
    {TOKEN_DOLLAR_SIGN, "TOKEN_DOLLAR_SIGN"},
    {TOKEN_ARROW, "TOKEN_ARROW"},
    {TOKEN_LBRACKET, "TOKEN_LBRACKET"},
    {TOKEN_RBRACKET, "TOKEN_RBRACKET"},
    {TOKEN_EOF, "TOKEN_EOF"}
};

const char* token_type_to_string(TokenType type) {
    for (int i = 0; i < ARRAY_SIZE(token_type_to_string_map); ++i) {
        if (token_type_to_string_map[i].type == type)
            return token_type_to_string_map[i].str;
    }
    return "TOKEN_NULL";
}

IdentifierType get_identifier_type(const Token* token) {
    if (token == NULL) return IDENT_UNKNOWN;

    for (int i = 0; i < ARRAY_SIZE(ident_str_to_ident_type); ++i) {
        int n = strlen(ident_str_to_ident_type[i].ident_str);
        _Bool same_size = n == string_size(get_token_identifier(token));

        if (same_size && strncmp(
            string_data(get_token_identifier(token)), 
            ident_str_to_ident_type[i].ident_str, n+1
        ) == 0)
            return ident_str_to_ident_type[i].type;
    }
    return IDENT_UNKNOWN;
}

void set_token(Token* token, TokenType type, int line_no, int col_no) {
    if (token == NULL) return;
    token->type = type;
    token->line_no = line_no;
    token->col_no = col_no;
    token->ident_type = IDENT_UNKNOWN;
    token->lexeme = create_string();
    token->integer = 0;
    token->percentage = false;
    token->identifier = create_string();
    token->percentage_mode = PERCENTAGE_MODE_UNKNOWN;
}

Token create_token(TokenType type, int line_no, int col_no) {
    Token token = {0};
    set_token(&token, type, line_no, col_no);
    return token;
}

Token create_empty_token() {
    return create_token(TOKEN_NULL, -1, -1);
}

void set_token_type(Token* token, TokenType type) {
    if (token == NULL) return;
    token->type = type;
}

TokenType get_token_type(const Token* token) {
    if (token == NULL) return TOKEN_NULL;
    return token->type;
}

void set_token_line_no(Token* token, int line_no) {
    if (token == NULL) return;
    token->line_no = line_no;
}

void set_token_col_no(Token* token, int col_no) {
    if (token == NULL) return;
    token->col_no = col_no;
}

void set_token_lexeme(Token* token, string lexeme) {
    if (token == NULL) return;
    free(token->lexeme.data);
    token->lexeme = lexeme;
}

const string* get_token_lexeme(const Token* token) {
    if (token == NULL) return NULL;
    return &token->lexeme;
}

void set_token_identifier(Token* token, string identifier) {
    if (token == NULL) return;
    free(token->identifier.data);
    token->identifier = identifier;
    token->ident_type = get_identifier_type(token);
}

void set_token_integer(Token* token, int n) {
    if (token == NULL) return;
    token->integer = n;
}

const string* get_token_identifier(const Token* token) {
    if (token == NULL) return NULL;
    return &token->identifier;
}

IdentifierType get_token_identifier_type(const Token* token) {
    if (token == NULL) return IDENT_UNKNOWN;
    return token->ident_type;
}

int get_token_integer(const Token* token) {
    if (token == NULL) return 0;
    return token->integer;
}

void set_token_percentage_mode(Token* token, PercentageMode mode) {
    if (token == NULL) return;
    token->percentage_mode = mode;
}

PercentageMode get_token_percentage_mode(const Token* token) {
    if (token == NULL) return PERCENTAGE_MODE_UNKNOWN;
    return token->percentage_mode;
}

void set_token_line_col(Token* token, int line_no, int col_no) {
    if (token == NULL) return;
    set_token_line_no(token, line_no);
    set_token_col_no(token, col_no);
}

int get_token_line_no(const Token* token) {
    if (token == NULL) return -1;
    return token->line_no;
}

int get_token_col_no(const Token* token) {
    if (token == NULL) return -1;
    return token->col_no;
}

const char* copy_lexeme(const Token* token) {
    if (token == NULL) return NULL;

    return string_data_copy(get_token_lexeme(token));
}

_Bool token_type_is(const Token* token, TokenType type) {
    if (token == NULL) return false;
    return token->type == type;
}

_Bool token_is_null(const Token* token) {
    if (token == NULL) return true;
    return token_type_is(token, TOKEN_NULL);
}

_Bool token_is_delimeter(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_NEWLINE) 
        || token_type_is(token, TOKEN_COMMA) 
        || token_type_is(token, TOKEN_SEMICOLON);
}

_Bool token_is_eof(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_EOF);
}

_Bool token_is_comment(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_COMMENT);
}

_Bool token_is_semicolon(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_SEMICOLON);
}

_Bool token_is_newline(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_NEWLINE);
}

_Bool token_is_comma(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_COMMA);
}

_Bool token_is_ident(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_IDENT);
}

_Bool token_is_string(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_STRING);
}

_Bool token_is_integer(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_INTEGER);
}

_Bool token_is_rbrace(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_RBRACE);
}

_Bool token_is_lbrace(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_LBRACE);
}

_Bool token_is_equal(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_EQUAL);
}

_Bool token_is_dollar_sign(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_DOLLAR_SIGN);
}

_Bool token_is_arrow(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_ARROW);
}

_Bool token_is_lbracket(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_LBRACKET);
}

_Bool token_is_rbracket(const Token* token) {
    if (token == NULL) return false;
    return token_type_is(token, TOKEN_RBRACKET);
}

void token_destroy(Token* token) {
    string_destroy(&token->lexeme);
    string_destroy(&token->identifier);
    token->type = TOKEN_NULL;
    token->line_no = 0;
    token->col_no = 0;
    token->ident_type = IDENT_UNKNOWN;
    token->integer = 0;
    token->percentage = false;
}

void token_refresh(Token* token) {
    token_destroy(token);
    token->lexeme = create_string();
    token->identifier = create_string();
}

void dump_token(Token* token) {
    if (token == NULL) return;
    const char* lexeme = string_data(&token->lexeme);
    const char* identifier = string_data(&token->identifier);
    IdentifierType ident_type = get_token_identifier_type(token);
    int line_no = token->line_no;
    int col_no = token->col_no;
    const char* token_string = token_type_to_string(get_token_type(token));

    printf("%s:\nlexeme: %s\nidentifier: %s\nidentifier type: %d\nline_no: %d\n col_no: %d\n\n",
        token_string, 
        lexeme,
        identifier,
        ident_type,
        line_no,
        col_no
    );
}
