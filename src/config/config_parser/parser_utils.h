/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_PARSER_UTILS_H
#define ANGEL_PARSER_UTILS_H

#include "types.h"
#include "layouts.h"
#include "defaults.h"
#include "token.h"
#include "workspaces.h"

typedef struct Token Token;
typedef struct ConfigError ConfigError;

_Bool skip_block(Token*);
void skip_to_next_statement(Token*);

_Bool is_define_valid(const Token*);
_Bool is_set_valid(const Token*);
_Bool is_color_valid(const Token*);
_Bool is_layout_valid(const Token*);
_Bool is_declare_valid(const Token*);
_Bool is_bind_valid(const Token*);

ConfigError fill_override_parameters(OverrideParameters*, Token*);

OverrideDefaultFn get_override_fn(const Token*);
KeybindActionFn get_action_fn(IdentifierType);

InitialState get_initial_state_from_ident_type(const Token*);
Layout get_layout_from_ident_type(const Token*);

_Bool is_minimized_position_identifier(const Token*);
_Bool is_focus_start_position_identifier(const Token*);
_Bool is_focus_end_position_identifier(const Token*);

DefinitionType get_def_type(const Token*);

#endif
