/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "config_error.h"
#include "text_buffer.h"
#include "utils.h"
#include "token.h"
#include "parser_utils.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

static const ConfigErrorPair error_messages[] = {
    {ANGEL_GOOD, ""}, {ANGEL_EOF, ""},
    {ANGEL_UNKNOWN_TOKEN, "Unknown token"},
    {ANGEL_SYNTAX_ERROR, "Syntax error"},
    {ANGEL_EXPECTED_COMMA, "Expecetd \033[32m,\033[0m"},
    {ANGEL_UNEXPECTED_EOF, "Unexpected eof"},
    {ANGEL_EXPECTED_RBRACE, "Expected \033[32m}\033[0m"},
    {ANGEL_UNKNOWN_DEFINE, "Unknown define"},
    {ANGEL_UNKNOWN_SET, "Unknown set"},
    {ANGEL_EXPECTED_DELIMITER, "Expected delimiter"},
    {ANGEL_UNKNOWN_LAYOUT, "Unknown layout"},
    {ANGEL_EXPECTED_LBRACE, "Expected \033[32m{\033[0m"},
    {ANGEL_BAD_LEX, "Bad lex"},
    {ANGEL_EXPECTED_IDENTIFIER, "Expected identifier"},
    {ANGEL_BAD_TOKEN, "Bad token"},
    {ANGEL_UNEXPECTED_TOKEN, "Unexpected token"}, 
    {ANGEL_BAD_VALUE, "Bad value"},
    {ANGEL_BAD_PATH, "Bad path"},
    {ANGEL_SYMBOL_EXISTS, "Symbol already exists"},
    {ANGEL_SYMBOL_NOT_FOUND, "Symbol does not exist"},
    {ANGEL_INTERNAL_ERROR, "Internal error, contact maintainer"},
    {ANGEL_EXPECTED_LBRACKET, "Expected \033[32m[\033[0m"},
    {ANGEL_EXPECTED_RBRACKET, "Expected \033[32m]\033[0m"},
    {ANGEL_BAD_WORKSPACE, "Bad workspace number"}, 
    {ANGEL_BAD_STATE, "Unknown state"},
    {ANGEL_BAD_MOD, "Bad mod"}
};

const char* get_error_message(ConfigErrorType type) {
    for (int i = 0; i < ARRAY_SIZE(error_messages); ++i) {
        if (error_messages[i].type == type) 
            return error_messages[i].message;
    }
    return "";
}

void print_error_message(const ConfigError* config_error) {
    if (config_error == NULL 
        || config_error_is_eof(config_error) 
        || config_error_is_good(config_error)
    ) return;

    char* line = get_line(config_error->line_no);
    if (line == NULL) return;

    fprintf(
        stdout,
        "\033[31m[Config Error]\033[0m: %s at line %d, col %d\n%s", 
        get_error_message(get_config_error_type(config_error)),
        config_error->line_no,
        config_error->col_no,
        line
    );

    for (int i = 1; i < config_error->col_no; ++i) {
        fprintf(stdout, " ");
    } 
    fprintf(stdout, "\033[31m^\033[0m\n\n");

    free(line);
}

void set_config_error(ConfigError* config_error, ConfigErrorType error_type, int line_no, int col_no) {
    if (config_error == NULL) return;
    config_error->type = error_type;
    config_error->line_no = line_no;
    config_error->col_no = col_no;
}

ConfigError create_config_error(ConfigErrorType error_type, int line_no, int col_no) {
    ConfigError error = {0};
    set_config_error(&error, error_type, line_no, col_no);
    return error;
}

ConfigError create_error_skip_statement(Token* token, ConfigErrorType error_type) {
    if (token == NULL) return create_config_error(ANGEL_BAD_TOKEN, -1, -1);

    ConfigError err = create_config_error(
        error_type,
        get_token_line_no(token),
        get_token_col_no(token)
    );

    skip_to_next_statement(token);
    return err;
}

void set_config_error_type(ConfigError* config_error, ConfigErrorType error_type) {
    if (config_error == NULL) return;
    config_error->type = error_type;
}

ConfigErrorType get_config_error_type(const ConfigError* config_error) {
    if (config_error == NULL) return ANGEL_GOOD;
    return config_error->type;
}

void set_config_error_line_no(ConfigError* config_error, int line_no) {
    if (config_error == NULL) return;
    config_error->line_no = line_no;
}

void set_config_error_col_no(ConfigError* config_error, int col_no) {
    if (config_error == NULL) return;
    config_error->col_no = col_no;
}

int get_config_error_line_no(const ConfigError* config_error) {
    if (config_error == NULL) return -1;
    return config_error->line_no;
}

int get_config_error_col_no(const ConfigError* config_error) {
    if (config_error == NULL) return -1;
    return config_error->col_no;
}

_Bool config_error_type_is(const ConfigError* config_error, ConfigErrorType error_type) {
    if (config_error == NULL) return false;
    return config_error->type == error_type;
}

_Bool config_error_is_good(const ConfigError* config_error) {
    if (config_error == NULL) return false;
    return config_error_type_is(config_error, ANGEL_GOOD);
}

_Bool config_error_is_eof(const ConfigError* config_error) {
    if (config_error == NULL) return false;
    return config_error_type_is(config_error, ANGEL_EOF);
}

_Bool config_error_is_syntax_error(const ConfigError* config_error) {
    if (config_error == NULL) return false;
    return config_error_type_is(config_error, ANGEL_SYNTAX_ERROR);
}

_Bool config_error_is_unknown_token(const ConfigError* config_error) {
    if (config_error == NULL) return false;
    return config_error_type_is(config_error, ANGEL_UNKNOWN_TOKEN);
}

_Bool config_error_is_expected_comma(const ConfigError* config_error) {
    if (config_error == NULL) return false;
    return config_error_type_is(config_error, ANGEL_EXPECTED_COMMA);
}

_Bool config_error_is_unexpected_eof(const ConfigError* config_error) {
    if (config_error == NULL) return false;
    return config_error_type_is(config_error, ANGEL_UNEXPECTED_EOF);
}

_Bool config_error_is_expected_rbrace(const ConfigError* config_error) {
    if (config_error == NULL) return false;
    return config_error_type_is(config_error, ANGEL_EXPECTED_RBRACE);
}

_Bool config_error_is_expected_lbrace(const ConfigError* config_error) {
    if (config_error == NULL) return false;
    return config_error_type_is(config_error, ANGEL_EXPECTED_LBRACE);
}

_Bool config_error_is_unknown_define(const ConfigError* config_error) {
    if (config_error == NULL) return false;
    return config_error_type_is(config_error, ANGEL_UNKNOWN_DEFINE);
}

_Bool config_error_is_unknown_set(const ConfigError* config_error) {
    if (config_error == NULL) return false;
    return config_error_type_is(config_error, ANGEL_UNKNOWN_SET);
}

_Bool config_error_is_expected_delimiter(const ConfigError* config_error) {
    if (config_error == NULL) return false;
    return config_error_type_is(config_error, ANGEL_EXPECTED_DELIMITER);
}

_Bool config_error_is_unknown_layout(const ConfigError* config_error) {
    if (config_error == NULL) return false;
    return config_error_type_is(config_error, ANGEL_UNKNOWN_LAYOUT);
}

_Bool config_error_is_comment(const ConfigError* config_error) {
    if (config_error == NULL) return false;
    return config_error_type_is(config_error, ANGEL_COMMENT);
}
