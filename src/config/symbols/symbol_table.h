/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_SYMBOL_TABLE_H
#define ANGEL_SYMBOL_TABLE_H

#include "token.h"

#define SYM_TABLE_SIZE 1024
#define PRIME 67


typedef struct Symbol {
    const char* name;
    const char* value;
    TokenType token_type;
    struct Symbol* next;
} Symbol;

_Bool symbol_is_string(Symbol*);
_Bool symbol_is_identifier(Symbol*);
_Bool symbol_is_integer(Symbol*);

typedef struct Bucket {
    Symbol* head;
} Bucket;

void bucket_push(Bucket*, Symbol*);

typedef struct SymbolTable {
    Bucket table[SYM_TABLE_SIZE]; 
} SymbolTable;

_Bool symbol_exists(const char*);
TokenType get_symbol_token_type(const char*);
const char* get_symbol_value(const char*);
const char* get_symbol_value_copy(const char*);
_Bool install_symbol(const char**, const char**, TokenType);
void sym_table_destroy();

#endif
