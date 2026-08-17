/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "lexer.h"
#include "text_buffer.h"
#include "token.h"
#include "angel_strings.h"
#include "config_error.h"
#include "types.h"
#include "utils.h"
#include "parser_utils.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

static inline _Bool valid_identifier_character(char);

static _Bool try_lex_identifier(char, Token* token, string*, ConfigError* err);
static _Bool try_lex_string(char, Token* token, string*, ConfigError* err);
static _Bool try_lex_integer(char, Token* token, string*, ConfigError* err);

static ConfigError lex_unknown(Token*, string*);
static ConfigError lex_comment(Token*, string*);
static ConfigError lex_single_character(Token*, TokenType);
static ConfigError lex_semicolon(Token*, string*);
static ConfigError lex_newline(Token*, string*);
static ConfigError lex_comma(Token*, string*);
static ConfigError lex_lbrace(Token*, string*);
static ConfigError lex_rbrace(Token*, string*);
static ConfigError lex_lbracket(Token*, string*);
static ConfigError lex_rbracket(Token*, string*);
static ConfigError lex_equal(Token*, string*);
static ConfigError lex_dollar_sign(Token*, string*);
static ConfigError lex_arrow(Token*, string*);

typedef struct LexPair {
    char c; 
    LexFn fn;
} LexPair;

static void set_lex_pair(LexPair*, char, LexFn);
static LexPair* create_lex_pair(char, LexFn);
static LexFn get_lex_fn(const LexPair*);

static const LexPair lex_map[] = {
    {'#', lex_comment},
    {';', lex_semicolon},
    {'\n', lex_newline},
    {',', lex_comma},
    {'{', lex_lbrace},
    {'}', lex_rbrace},
    {'[', lex_lbracket},
    {']', lex_rbracket},
    {'=', lex_equal},
    {'$', lex_dollar_sign},
    {'-', lex_arrow}
};

static LexFn find_lex_fn(char);

_Bool valid_identifier_character(char c) {
    return (isdigit(c) || isalpha(c) || c == '_' || c == '-'); 
}

LexFn find_lex_fn(char c) {
    for (int i = 0; i < ARRAY_SIZE(lex_map); ++i)
        if (lex_map[i].c == c) 
            return get_lex_fn(&lex_map[i]);
    return lex_unknown;
} 

void set_lex_pair(LexPair* lp, char c, LexFn fn) {
    if (lp == NULL) return;
    lp->c = c;
    lp->fn = fn;
}

LexPair* create_lex_pair(char c, LexFn fn) {
    LexPair* lp;
    if ((lp = calloc(1, sizeof(LexPair))) == NULL)
        return NULL;

    lp->c = c;
    lp->fn = fn;
    return lp;
}

LexFn get_lex_fn(const LexPair* lp) {
    return lp != NULL 
        ? lp->fn 
        : lex_unknown;
}

_Bool try_lex_identifier(char c, Token* token, string* lexeme, ConfigError* err) {
    if (token == NULL || (!isalpha(c))) return false;

    set_token_type(token, TOKEN_IDENT);

    const char* ch; 
    while ((ch = buffer_get_next_char()) != NULL) {
        if (!valid_identifier_character(*ch)) {
            buffer_retreat();
            return true;
        }

        string_append(lexeme, *ch);
    }

    *err = create_config_error(
        ANGEL_GOOD, 
        get_token_line_no(token),
        get_token_col_no(token)
    );

    return true;
}

_Bool try_lex_string(char c, Token* token, string* lexeme, ConfigError* err) {
    if (token == NULL || (c != '"')) return false;

    set_token_type(token, TOKEN_STRING);
    string_refresh(lexeme);

    const char* ch;
    while ((ch = buffer_get_next_char()) != NULL) {
        if (*ch == '"') break;

        if (buffer_eof()) {
            *err = create_config_error(
                ANGEL_UNEXPECTED_EOF,
                get_token_line_no(token),
                get_token_col_no(token)
            );
            return true;
        }
        
        string_append(lexeme, *ch);
    }

    *err = create_config_error(
        ANGEL_GOOD, 
        get_token_line_no(token),
        get_token_col_no(token)
    );

    return true;
}

_Bool try_lex_integer(char c, Token* token, string* lexeme, ConfigError* err) {
    if (token == NULL || (!isdigit(c))) return false;

    set_token_type(token, TOKEN_INTEGER);

    const char* ch;
    while ((ch = buffer_get_next_char()) != NULL) {
        if (*ch == '%') {
            token->percentage = true;

            ch = buffer_get_next_char();
            if (ch != NULL) switch (*ch) {
                case 'w': 
                    set_token_percentage_mode(
                        token,
                        PERCENTAGE_MODE_WIDTH
                    );
                    break;
                case 'h':
                    set_token_percentage_mode(
                        token,
                        PERCENTAGE_MODE_HEIGHT
                    );
                    break;
                default:
                    buffer_retreat();
            }

            set_token_integer(token, atoi(string_data(lexeme)));
            string_append(lexeme, *ch);
            return true;
        }

        else if (!isdigit(*ch)) {
            buffer_retreat();
            break;
        }

        string_append(lexeme, *ch);
    }

    set_token_integer(token, atoi(string_data(lexeme)));

    *err = create_config_error(
        ANGEL_GOOD, 
        get_token_line_no(token),
        get_token_col_no(token)
    );

    return true;
}

ConfigError lex_unknown(Token* token, string* string) {
    if (token == NULL || string == NULL) 
        return create_config_error(ANGEL_BAD_LEX, -1, -1);
    return create_config_error(
        ANGEL_UNKNOWN_TOKEN, 
        get_token_line_no(token),
        get_token_col_no(token)
    );
}

ConfigError lex_comment(Token* token, string* string) {
    if (token == NULL || string == NULL) 
        return create_config_error(ANGEL_BAD_LEX, -1, -1);

    set_token_type(token, TOKEN_COMMENT);

    const char* c;
    while (((c = buffer_get_next_char()) != NULL) && *c != '\n' && !buffer_eof()) {
        string_append(string, *c);
    }

    ConfigError err = {0};
    if (buffer_eof())
        err = create_config_error(
            ANGEL_UNEXPECTED_EOF,
            get_token_line_no(token),
            get_token_col_no(token)
        );

    else 
        err = create_config_error(
            ANGEL_GOOD,
            get_token_line_no(token),
            get_token_col_no(token)
        );

    return err;
}

ConfigError lex_single_character(Token* token, TokenType type) {
    if (token == NULL) 
        return create_config_error(ANGEL_BAD_LEX, -1, -1);

    set_token_type(token, type);
    return create_config_error(
        ANGEL_GOOD,
        get_token_line_no(token),
        get_token_col_no(token)
    );
}

ConfigError lex_semicolon(Token* token, string* string) {
    (void)string;
    return lex_single_character(token, TOKEN_SEMICOLON);
}

ConfigError lex_newline(Token* token, string* string) {
    (void)string;
    return lex_single_character(token, TOKEN_NEWLINE);
}

ConfigError lex_comma(Token* token, string* string) {
    (void)string;
    return lex_single_character(token, TOKEN_COMMA);
}

ConfigError lex_lbrace(Token* token, string* string) {
    (void)string;
    return lex_single_character(token, TOKEN_LBRACE);
}

ConfigError lex_rbrace(Token* token, string* string) {
    (void)string;
    return lex_single_character(token, TOKEN_RBRACE);
}

ConfigError lex_lbracket(Token* token, string* string) {
    (void)string;
    return lex_single_character(token, TOKEN_LBRACKET);
}

ConfigError lex_rbracket(Token* token, string* string) {
    (void)string;
    return lex_single_character(token, TOKEN_RBRACKET);
}

ConfigError lex_equal(Token* token, string* string) {
    (void)string;
    return lex_single_character(token, TOKEN_EQUAL);
}

ConfigError lex_dollar_sign(Token* token, string* string) {
    (void)string;
    return lex_single_character(token, TOKEN_DOLLAR_SIGN);
}

ConfigError lex_arrow(Token* token, string* lexeme) {
    if (token == NULL) 
        return create_config_error(ANGEL_BAD_LEX, -1, -1);

    const char* c = buffer_get_next_char();
    if (*c != '>') {
        buffer_retreat();
        return create_config_error(
            ANGEL_UNKNOWN_TOKEN,
            get_token_line_no(token),
            get_token_col_no(token)
        );
    }

    set_token_type(token, TOKEN_ARROW);
    string_append(lexeme, *c);
    return create_config_error(
        ANGEL_GOOD,
        get_token_line_no(token),
        get_token_col_no(token)
    );
}

_Bool lex_init() {
    lex_destroy();
    return init_buffer(); 
}

void get_next_token_print_error(Token* token) {
    if (token == NULL) return;
    ConfigError lex_err = get_next_token(token);
    if (!config_error_is_good(&lex_err) && !config_error_is_eof(&lex_err)) {
        print_error_message(&lex_err);
        // skip_to_next_statement(token);
    }
}

ConfigError get_next_token(Token* token) {
    token_destroy(token);

    const char* c = buffer_get_next_char();
    while (c != NULL && *c != '\n' && isspace(*c) && !buffer_eof())
        c = buffer_get_next_char();

    if (buffer_eof()) {
        set_token_type(token, TOKEN_EOF);
        set_token_line_col(token, line_no, col_no);
        return create_config_error(ANGEL_EOF, line_no, col_no);
    }

    string lexeme = create_string();
    string_append(&lexeme, *c);
    set_token_line_col(token, line_no, col_no);

    ConfigError err = {0};
    _Bool lexed = try_lex_identifier(*c, token, &lexeme, &err) 
        || try_lex_string(*c, token, &lexeme, &err) 
        || try_lex_integer(*c, token, &lexeme, &err);

    if (!lexed)
        err = find_lex_fn(*c)(token,&lexeme);

    if (!config_error_is_good(&err)) {
        token_destroy(token);
        string_destroy(&lexeme);
        return err;
    }

    set_token_lexeme(token, lexeme);
    if (token_is_ident(token)) {
        set_token_identifier(token, string_copy(&lexeme));
    }

    if (token_is_comment(token)) {
        return get_next_token(token);
    }

    return create_config_error(
        ANGEL_GOOD,
        get_token_line_no(token),
        get_token_col_no(token)
    );
}

void lex_destroy() {
    buffer_destroy();
}
