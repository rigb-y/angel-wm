/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_CONFIG_ERROR_H
#define ANGEL_CONFIG_ERROR_H

typedef enum ConfigErrorType {
    ANGEL_GOOD, ANGEL_EOF, ANGEL_UNKNOWN_TOKEN,
    ANGEL_SYNTAX_ERROR, ANGEL_EXPECTED_COMMA,
    ANGEL_UNEXPECTED_EOF, ANGEL_EXPECTED_RBRACE,
    ANGEL_UNKNOWN_DEFINE, ANGEL_UNKNOWN_SET,
    ANGEL_EXPECTED_DELIMITER, ANGEL_UNKNOWN_LAYOUT,
    ANGEL_EXPECTED_LBRACE, ANGEL_BAD_LEX, ANGEL_COMMENT,
    ANGEL_EXPECTED_IDENTIFIER, ANGEL_BAD_TOKEN,
    ANGEL_UNEXPECTED_TOKEN, ANGEL_BAD_VALUE, 
    ANGEL_BAD_PATH, ANGEL_SYMBOL_EXISTS,
    ANGEL_SYMBOL_NOT_FOUND, ANGEL_INTERNAL_ERROR,
    ANGEL_EXPECTED_LBRACKET, ANGEL_EXPECTED_RBRACKET,
    ANGEL_EXPECTED_WORKSPACE_NUMBER, ANGEL_BAD_WORKSPACE,
    ANGEL_BAD_STATE, ANGEL_BAD_MOD
} ConfigErrorType;

typedef struct Token Token;

typedef struct ConfigError {
    ConfigErrorType type;
    int line_no;
    int col_no;
} ConfigError;

typedef struct ConfigErrorPair {
    ConfigErrorType type;
    const char* message;
} ConfigErrorPair;

const char* get_error_message(ConfigErrorType);
void print_error_message(const ConfigError*);

void set_config_error(ConfigError*, ConfigErrorType, int, int);
ConfigError create_config_error(ConfigErrorType, int, int);

ConfigError create_error_skip_statement(Token*, ConfigErrorType); 

void set_config_error_type(ConfigError*, ConfigErrorType);
ConfigErrorType get_config_error_type(const ConfigError*);

void set_config_error_line_no(ConfigError*, int);
void set_config_error_col_no(ConfigError*, int);

int get_config_error_line_no(const ConfigError*);
int get_config_error_col_no(const ConfigError*);

_Bool config_error_type_is(const ConfigError*, ConfigErrorType);

_Bool config_error_is_good(const ConfigError*);
_Bool config_error_is_eof(const ConfigError*);
_Bool config_error_is_syntax_error(const ConfigError*);
_Bool config_error_is_unknown_token(const ConfigError*);
_Bool config_error_is_expected_comma(const ConfigError*);
_Bool config_error_is_unexpected_eof(const ConfigError*);
_Bool config_error_is_expected_rbrace(const ConfigError*);
_Bool config_error_is_expected_lbrace(const ConfigError*);
_Bool config_error_is_unknown_define(const ConfigError*);
_Bool config_error_is_unknown_set(const ConfigError*);
_Bool config_error_is_expected_delimiter(const ConfigError*);
_Bool config_error_is_unknown_layout(const ConfigError*);
_Bool config_error_is_comment(const ConfigError*);

#endif
