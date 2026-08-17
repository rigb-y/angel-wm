/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "symbol_table.h"
#include "token.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

static SymbolTable sym_table = {0};

static void bucket_destroy(Bucket*);
static Bucket* get_bucket(int);
static void set_symbol(Symbol*, const char*, const char*, TokenType);
static Symbol* create_symbol(const char**, const char**, TokenType);
static int hash(const char*);
static size_t get_name_size(const char*);
static void free_symbol_resources(Symbol*);

_Bool symbol_is_string(Symbol* sym) {
	if (sym == NULL) return false;
    return sym->token_type == TOKEN_STRING;
}

_Bool symbol_is_identifier(Symbol* sym) {
	if (sym == NULL) return false;
    return sym->token_type == TOKEN_IDENT;
}

_Bool symbol_is_integer(Symbol* sym) {
	if (sym == NULL) return false;
    return sym->token_type == TOKEN_INTEGER;
}

void set_symbol(Symbol* sym, const char* name, const char* value, TokenType type) {
    if (sym == NULL) return;
    sym->name = name;
    sym->value = value;
    sym->token_type = type;
    sym->next = NULL;
}

Symbol* create_symbol(const char** name, const char** value, TokenType type) {
    if (name == NULL 
        || value == NULL 
        || *name == NULL 
        || *value == NULL
    ) return NULL;

    Symbol* sym;
    if ((sym = calloc(1, sizeof(Symbol))) == NULL) {
        free((char*)*name);
        free((char*)*value);
        *name = NULL;
        *value = NULL;

        return NULL;
    }

    set_symbol(sym, *name, *value, type);
    return sym;
}

void bucket_push(Bucket* bucket, Symbol* sym) {
    if (bucket == NULL || sym == NULL) return;

    Symbol* head = bucket->head;
    bucket->head = sym;
    bucket->head->next = head;
}

_Bool symbol_exists(const char* name) {
    if (name == NULL) return false;

    Symbol* curr = get_bucket(hash(name))->head;
    while (curr != NULL) {
        if (strcmp(curr->name, name) == 0)
            return true;
        curr = curr->next;
    }

    return false;
}

TokenType get_symbol_token_type(const char* name) {
    if (name == NULL) return TOKEN_NULL;

    Symbol* curr = get_bucket(hash(name))->head;
    while (curr != NULL) {
        if (strcmp(curr->name, name) == 0)
            return curr->token_type;
        curr = curr->next;
    }

    return TOKEN_NULL;
}

const char* get_symbol_value(const char* name) {
    if (name == NULL) return NULL;

    Symbol* curr = get_bucket(hash(name))->head;
    while (curr != NULL) {
        if (strcmp(curr->name, name) == 0)
            return curr->value;
        curr = curr->next;
    }
    return NULL;
}

const char* get_symbol_value_copy(const char* name) {
    const char* value;
    if ((value = get_symbol_value(name)) == NULL)
        return NULL;

    size_t n = get_name_size(value);

    char* copy; 
    if ((copy = malloc((sizeof(char) * n) + 1)) == NULL)
        return NULL;

    memcpy(copy, value, n);
    copy[n] = '\0';
    return copy;
}

Bucket* get_bucket(int hx) {
    return (hx > 0 && hx < SYM_TABLE_SIZE)
        ? &sym_table.table[hx]
        : NULL;
}

void free_symbol_resources(Symbol* sym) {
    if (sym == NULL) return;
    free((char*)sym->name);
    free((char*)sym->value);
    sym->name = NULL;
    sym->value = NULL;
}

void bucket_destroy(Bucket* bucket) {
    if (bucket == NULL) return; 

    Symbol* curr = bucket->head;
    while  (curr != NULL) {
        Symbol* tmp = curr;
        curr = curr->next;
        free_symbol_resources(tmp);
        free(tmp);
    }

    bucket->head = NULL;
}

size_t get_name_size(const char* name) {
    if (name == NULL) return 0;

    int k = -1; while (name[++k] != '\0') (void)1; return k;
}

int hash(const char* name) {
    int sum = 0; size_t n = get_name_size(name);

    int prev = name[n-1];
    for (int i = (int)n-2; i >= 0; --i)
        prev = (PRIME * prev + (unsigned char) name[i]) 
            & (SYM_TABLE_SIZE-1);

    return prev & (SYM_TABLE_SIZE-1);
}

_Bool install_symbol(const char** name, const char** value, TokenType type) {
    if (name == NULL 
        || value == NULL 
        || *name == NULL
        || *value == NULL
        || symbol_exists(*name)
    ) return false;

    bucket_push(
        get_bucket(hash(*name)),
        create_symbol(name, value, type)
    );

    return true;
}

void sym_table_destroy() {
    for (int i = 0; i < SYM_TABLE_SIZE; ++i)
        bucket_destroy(&sym_table.table[i]);
}
