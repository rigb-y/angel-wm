/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_LEXER_H
#define ANGEL_LEXER_H

typedef struct Token Token;
typedef struct ConfigError ConfigError;

_Bool lex_init();
ConfigError get_next_token(Token*);
void get_next_token_print_error(Token*);
void lex_destroy();

#endif
