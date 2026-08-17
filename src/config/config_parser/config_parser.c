/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "config_parser.h"
#include "lexer.h"
#include "token.h"
#include "config_error.h"
#include "types.h"
#include "token.h"
#include "utils.h"
#include "parser_utils.h"
#include "defaults.h"
#include "shell.h"
#include "symbol_table.h"
#include "workspaces.h"
#include "keymaps.h"
#include "keyboard.h"
#include "events.h"
#include "logging.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define BIND_COMMAND true
#define BIND_ACTION false

static _Bool HEALTH_CHECK = false;

static Token next_token = {0};

typedef struct IdentifierPair {
    IdentifierType type;
    ParseFn fn;
} IdentifierPair;

static ConfigError parse_definition(IsValidFn);
static ConfigError parse_layout_change();
static ConfigError parse_layout_changes();
static ConfigError decompose_bind(const char*, unsigned int*, KeySym*, int, int);
static size_t count_dashes(const char*);
static const char* get_var_name_in_bind(const Token*, int*);
static const char* get_remainder_of_str(const char*, int);
static ConfigError reconcile_bind(Token*);
static unsigned int get_bind_state(const char*, int, int*);
static KeySym get_bind_key(const char*, int);
static ConfigError sub_parse_bind(const char**, const char**, KeySym*, unsigned int*, int, IdentifierType*);
static _Bool check_done(const char*);

static ConfigError parse_unknown_ident();
static ConfigError parse_exec();
static ConfigError parse_let();
static ConfigError parse_set_background();
static ConfigError parse_define();
static ConfigError parse_set();
static ConfigError parse_color();
static ConfigError parse_layouts();
static ConfigError parse_bind_command();
static ConfigError parse_declare();
static ConfigError parse_bind();

static const IdentifierPair ident_to_parse_map[] = {
    {IDENT_UNKNOWN, parse_unknown_ident},
    {IDENT_EXEC, parse_exec},
    {IDENT_LET, parse_let},
    {IDENT_SET_BACKGROUND, parse_set_background},
    {IDENT_DEFINE, parse_define},
    {IDENT_SET, parse_set},
    {IDENT_COLOR, parse_color},
    {IDENT_LAYOUTS, parse_layouts},
    {IDENT_BIND_COMMAND, parse_bind_command},
    {IDENT_DECLARE, parse_declare},
    {IDENT_BIND, parse_bind},
};

static ParseFn get_parse_fn(const Token*);

static _Bool validate_path(const char*);
static ConfigError handle_variable(const char**, Token*);

static void parser_destroy();
static _Bool parser_init();
static ConfigError parse_statement();

_Bool check_config_health() {
    HEALTH_CHECK = true;

    _Bool healthy = parse_config();

    HEALTH_CHECK = false;

    return healthy;
}

ParseFn get_parse_fn(const Token* token) {
    for (int i = 0; i < ARRAY_SIZE(ident_to_parse_map); ++i)
        if (ident_to_parse_map[i].type == get_token_identifier_type(token))
            return ident_to_parse_map[i].fn;
    return parse_unknown_ident;
}

_Bool parser_init() {
    parser_destroy();
    return lex_init();
}

_Bool validate_path(const char* path) {
    if (path == NULL) return false;
    return access(path, F_OK) == 0;
}

ConfigError handle_variable(const char** value, Token* t_return) {
    if (value == NULL) 
        return create_config_error(
            ANGEL_INTERNAL_ERROR,
            -1, -1
        );

    get_next_token_print_error(&next_token);
    if (!token_is_ident(&next_token)) {
        return create_error_skip_statement(
            &next_token,
            ANGEL_EXPECTED_IDENTIFIER
        );
    }

    *value = get_symbol_value_copy(
        string_data(
            get_token_identifier(&next_token)
        )
    );

    if (*value == NULL) {
        return create_error_skip_statement(
            &next_token,
            ANGEL_SYMBOL_NOT_FOUND
        );
    }

    if (t_return != NULL) {
        Token phony_token = {0};
        string identifier = create_string_with_data(*value);
        string lexeme = string_copy(&identifier);
        phony_token = create_token(
            get_symbol_token_type(
                string_data(
                    get_token_identifier(&next_token)
                )
            ),
            get_token_line_no(&next_token),
            get_token_col_no(&next_token)
        );

        set_token_identifier(&phony_token, identifier);
        set_token_lexeme(&phony_token, lexeme);

        *t_return = phony_token;
    }

    return create_config_error(
        ANGEL_GOOD,
        0, 0
    );
}

ConfigError parse_exec() {
    get_next_token_print_error(&next_token);

    const char* value = NULL;
    if (token_is_dollar_sign(&next_token)) {
        ConfigError err = handle_variable(&value, NULL);
        if (!config_error_is_good(&err)) {
            skip_to_next_statement(&next_token);
            return err;
        }
    }

    else if (!token_is_ident(&next_token) && !token_is_string(&next_token)) {
        free((char*)value);
        return create_error_skip_statement(
            &next_token,
            ANGEL_UNEXPECTED_TOKEN
        );
    }

    const char* command = value == NULL 
        ? copy_lexeme(&next_token)
        : value;

    get_next_token_print_error(&next_token);

    if (!token_is_newline(&next_token) && !token_is_semicolon(&next_token)) {
        free((char*)command);
        return create_error_skip_statement(
            &next_token,
            ANGEL_EXPECTED_DELIMITER
        );
    }

    exec_command(command);
    get_next_token_print_error(&next_token);

    free((char*)command);
    return create_config_error(
        ANGEL_GOOD, 
        0, 0
    );
}

ConfigError parse_let() {
    get_next_token_print_error(&next_token);
    if (!token_is_ident(&next_token))
        create_error_skip_statement(
            &next_token,
            ANGEL_EXPECTED_IDENTIFIER
        );

    int symbol_line_no = get_token_line_no(&next_token);
    int symbol_col_no = get_token_col_no(&next_token);

    // Make a copy that the symbol table can own
    const char* var = string_data_copy(
        get_token_identifier(&next_token)
    );

    get_next_token_print_error(&next_token);

    if (!token_is_equal(&next_token)) {
        free((char*)var);
        return create_error_skip_statement(
            &next_token,
            ANGEL_UNEXPECTED_TOKEN
        );
    }

    get_next_token_print_error(&next_token);
    TokenType token_type = get_token_type(&next_token);

    if (!token_is_ident(&next_token) 
        && !token_is_string(&next_token) 
        && !token_is_integer(&next_token)
    ) {
        free((char*)var);
        return create_error_skip_statement(
            &next_token,
            ANGEL_UNEXPECTED_TOKEN
        );
    }

    const char* value = string_data_copy(
        get_token_lexeme(&next_token)
    );

    int value_line_no = get_token_line_no(&next_token);
    int value_col_no = get_token_col_no(&next_token);

    get_next_token_print_error(&next_token);
    if (!token_is_newline(&next_token) && !token_is_semicolon(&next_token)) {
        free((char*)var);
        free((char*)value);
        return create_error_skip_statement(
            &next_token,
            ANGEL_EXPECTED_DELIMITER
        );
    }

    if (!install_symbol(&var, &value, token_type)) {
        free((char*)var);
        free((char*)value);

        skip_to_next_statement(&next_token);
        return create_config_error(
            ANGEL_SYMBOL_EXISTS,
            symbol_line_no,
            symbol_col_no
        );
    }

    if (strcmp(var, "mod") == 0) {
        int modifier = get_modifier_from_str(value);
        if (modifier == UNKNOWN_MODIFIER) {
            return create_config_error(
                ANGEL_BAD_MOD,
                value_line_no,
                value_col_no
            );
        }

        set_modifier(modifier);
    }

    return create_config_error(
        ANGEL_GOOD,
        0, 0
    );
}

ConfigError parse_set_background() {
    get_next_token_print_error(&next_token);

    const char* value = NULL;
    Token t_return = {0};
    
    if (token_is_dollar_sign(&next_token)) {
        ConfigError err = handle_variable(&value, &t_return);
        if (!config_error_is_good(&err))
            return err;
    }

    if (!token_is_ident(&next_token) && !token_is_string(&next_token)) {
        token_destroy(&t_return);
        free((char*)value);
        return create_error_skip_statement(
            &next_token,
            ANGEL_UNEXPECTED_TOKEN
        );
    }

    const char* path = copy_lexeme(value == NULL ? &next_token : &t_return);
    const char* expanded_path = expand_path(path, get_raw_str_size(path));
    free((char*)path);

    size_t path_size = get_raw_str_size(expanded_path);

    token_destroy(&t_return);
    free((char*)value);

    if (!validate_path(expanded_path)) {
        free((char*)expanded_path);
        return create_error_skip_statement(
            &next_token,
            ANGEL_BAD_PATH
        );
    }
    string command = create_string();

    char fh[] = "xwallpaper --zoom ";
    string_append_chars(&command, fh, ARRAY_SIZE(fh)-1);
    string_append_chars(&command, expanded_path, path_size);

    get_next_token_print_error(&next_token);
    if (!token_is_newline(&next_token) && !token_is_semicolon(&next_token)) {
        string_destroy(&command);
        free((char*)expanded_path);
        return create_error_skip_statement(
            &next_token,
            ANGEL_EXPECTED_DELIMITER
        );
    } 

    get_next_token_print_error(&next_token);
    exec_command(string_data(&command));

    string_destroy(&command);
    free((char*)expanded_path);
    return create_config_error(
        ANGEL_GOOD,
        0, 0
    );
}

ConfigError parse_definition(IsValidFn is_valid_fn) {
    DefinitionType def_type = get_def_type(&next_token);
    get_next_token_print_error(&next_token); 

    if (!token_is_ident(&next_token)) {
        return create_error_skip_statement(
            &next_token,
            ANGEL_EXPECTED_IDENTIFIER
        );
    }

    if (!is_valid_fn(&next_token)) {
        return create_error_skip_statement(
            &next_token,
            ANGEL_UNKNOWN_DEFINE
        );
    }

    OverrideDefaultFn override_fn = get_override_fn(&next_token);

    get_next_token_print_error(&next_token);

    if (!token_is_equal(&next_token)) {
        return create_error_skip_statement(
            &next_token,
            ANGEL_UNEXPECTED_TOKEN
        );
    }

    get_next_token_print_error(&next_token);

    const char* value = NULL;
    Token phony_token = {0}; 
    if (token_is_dollar_sign(&next_token)) {
        ConfigError err = handle_variable(&value, &phony_token); 
        if (!config_error_is_good(&err))
            return err;
    }

    int value_line_no = get_token_line_no(&next_token);
    int value_col_no = get_token_col_no(&next_token);

    OverrideParameters op = create_override_parameters();
    set_override_definition_type(&op, def_type);
    ConfigError err = fill_override_parameters(
        &op,
        value == NULL ? &next_token : &phony_token
    );

    if (!config_error_is_good(&err)) {
        token_destroy(&phony_token);
        free((char*)value);
        destroy_override_parameters(&op);
        skip_to_next_statement(&next_token);

        return err;
    }

    get_next_token_print_error(&next_token);

    if (!token_is_newline(&next_token) && !token_is_semicolon(&next_token)) {
        token_destroy(&phony_token);
        free((char*)value);
        destroy_override_parameters(&op);

        return create_error_skip_statement(
            &next_token,
            ANGEL_UNEXPECTED_TOKEN
        );
    }

    get_next_token_print_error(&next_token);

    _Bool st = true;
    if (HEALTH_CHECK == false)
        st = override_fn(&op);

    if (!st) {
        token_destroy(&phony_token);
        free((char*)value);
        destroy_override_parameters(&op);

        return create_config_error(
            ANGEL_BAD_VALUE, 
            value_line_no,
            value_col_no
        );
    }

    token_destroy(&phony_token);
    free((char*)value);
    destroy_override_parameters(&op);
    return create_config_error(
        ANGEL_GOOD,
        0, 0
    );
}

ConfigError parse_define() {
    return parse_definition(is_define_valid);
}

ConfigError parse_set() {
    return parse_definition(is_set_valid);
}

ConfigError parse_color() {
    return parse_definition(is_color_valid);
}

ConfigError parse_layout_change() {
    if (!token_is_integer(&next_token)) {
        ConfigError err = create_config_error(
            ANGEL_EXPECTED_WORKSPACE_NUMBER,
            get_token_line_no(&next_token),
            get_token_col_no(&next_token)
        );
        skip_block(&next_token);
        return err;
    }

    int space = get_token_integer(&next_token);

    if (space <= 0 || space > N_WORKSPACES) {
        ConfigError err = create_config_error(
            ANGEL_BAD_WORKSPACE,
            get_token_line_no(&next_token),
            get_token_col_no(&next_token)
        );
        skip_block(&next_token);
        return err;
    }

    get_next_token_print_error(&next_token);

    if (!token_is_arrow(&next_token)) {
        ConfigError err = create_config_error(
            ANGEL_UNEXPECTED_TOKEN,
            get_token_line_no(&next_token),
            get_token_col_no(&next_token)
        );
        skip_block(&next_token);
        return err;
    }

    get_next_token_print_error(&next_token);

    const char* value = NULL;
    Token t_return = {0};
    if (token_is_dollar_sign(&next_token)) {
        ConfigError err = handle_variable(&value, &t_return);
        if (!config_error_is_good(&err)) {
            skip_to_next_statement(&next_token);
            return err;
        }
    }

    if (!is_layout_valid(value == NULL ? &next_token : &t_return)) {
        ConfigError err = create_config_error(
            ANGEL_UNKNOWN_LAYOUT,
            get_token_line_no(value == NULL ? &next_token : &t_return),
            get_token_col_no(value == NULL ? &next_token : &t_return)
        );
        token_destroy(&t_return);
        free((char*)value);
        skip_block(&next_token);
        return err;
    }

    Layout layout = get_layout_from_ident_type(
        value == NULL ? &next_token : &t_return
    );

    set_workspace_layout(space-1, layout);
    get_next_token_print_error(&next_token);

    token_destroy(&t_return);
    free((char*)value);
    return create_config_error(
        ANGEL_GOOD,
        0, 0
    );
}

ConfigError parse_layout_changes() {
    while (!token_is_rbrace(&next_token) && !token_is_eof(&next_token)) {
        if (token_is_newline(&next_token) || token_is_comma(&next_token)) {
            get_next_token_print_error(&next_token);
            continue;
        }

        ConfigError err = parse_layout_change();
        if (!config_error_is_good(&err))
            return err;
    }

    return create_config_error(
        ANGEL_GOOD,
        0, 0
    );
}

ConfigError parse_layouts() {
    get_next_token_print_error(&next_token);

    if (!token_is_lbrace(&next_token))
        return create_error_skip_statement(
            &next_token,
            ANGEL_EXPECTED_LBRACE
        );

    get_next_token_print_error(&next_token);

    ConfigError err = parse_layout_changes();
    if (!config_error_is_good(&err)) {
        return err;
    }

    if (!token_is_rbrace(&next_token))
        return create_error_skip_statement(
            &next_token,
            ANGEL_EXPECTED_RBRACE
        );

    get_next_token_print_error(&next_token);

    return create_config_error(
        ANGEL_GOOD,
        0, 0
    );
}

unsigned int get_bind_state(const char* bind, int nd, int* final_k) {
    if (bind == NULL) return 0;

    unsigned int state_return = 0x0;
    string current_modifier = create_string();

    size_t c = 0; int k = 0;
    while (c != nd) {
        if (bind[k] == '-') {
            int state = get_modifier_from_str(
                string_data(&current_modifier)
            ); 

            if (state == UNKNOWN_MODIFIER)
                return 0x0;

            string_refresh(&current_modifier);
            state_return |= state;

            ++c; ++k;
            continue;
        }
        string_append(&current_modifier, bind[k++]);
    }
    string_destroy(&current_modifier);

    *final_k = k;

    return state_return;
}

KeySym get_bind_key(const char* bind, int k) {
    if (bind == NULL) return (KeySym)0;

    string key_str = create_string();
    while (bind[k] != '\0')
        string_append(&key_str, bind[k++]);

    KeySym key = get_keysym_from_str(
        string_data(&key_str)
    );

    string_destroy(&key_str);
    return key;
}

size_t count_dashes(const char* s) {
    if (s == NULL) return 0;

    size_t nd = 0; int k = 0;
    while (s[k] != '\0') if (s[k++] == '-') 
        ++nd;

    return nd;
}

ConfigError decompose_bind(
    const char* bind,
    unsigned int* state_return,
    KeySym* keysym_return,
    int bind_line_no,
    int bind_col_no
) {
    if (bind == NULL 
        || state_return == NULL 
        || keysym_return == NULL
    ) {
        return create_config_error(
            ANGEL_INTERNAL_ERROR,
            -1, -1
        );
    }
    *state_return = 0x0;
    size_t nd = count_dashes(bind);

    if (nd == 0) {
        KeySym sym = get_keysym_from_str(bind);
        if (sym == (KeySym)0) {
            skip_to_next_statement(&next_token);
            return create_config_error(
                ANGEL_BAD_VALUE,
                bind_line_no,
                bind_col_no
            );
        }

        *keysym_return = sym;
        return create_config_error(
            ANGEL_GOOD,
            0, 0
        );
    }

    int k = 0;
    unsigned int state = get_bind_state(bind, nd, &k);
    if (state == 0x0) {
        skip_to_next_statement(&next_token);
        return create_config_error(
            ANGEL_BAD_VALUE,
            bind_line_no,
            bind_col_no
        );
    }

    KeySym key = get_bind_key(bind, k);
    if (key == (KeySym)0) {
        skip_to_next_statement(&next_token);
        return create_config_error(
            ANGEL_BAD_VALUE,
            bind_line_no,
            bind_col_no
        );
    }

    *state_return = state;
    *keysym_return = key;
    return create_config_error(
        ANGEL_GOOD, 
        0, 0
    );
}

const char* get_var_name_in_bind(const Token* token, int* ended_at) {
    if (token == NULL) return NULL;

    string name = create_string();

    int k = 0;
    const char* token_lexeme_data = string_data(
        get_token_lexeme(token)
    );

    while (token_lexeme_data[k] != '-' && token_lexeme_data[k] != '\0')
        string_append(&name, token_lexeme_data[k++]);

    *ended_at = k;

    const char* ret = string_data_copy(&name);
    string_destroy(&name);
    return ret;
}

const char* get_remainder_of_str(const char* str, int k) {
    if (str == NULL || k < 0 || k > get_raw_str_size(str)) return "";

    string remainder = create_string();
    while (str[k] != '\0')
        string_append(&remainder, str[k++]);
        
    const char* ret = string_data_copy(&remainder);
    string_destroy(&remainder);
    return ret;
}

_Bool check_done(const char* str) {
    if (str == NULL) return false;
    string check = create_string();
    size_t n = get_raw_str_size(str);

    int I=n-1; for (; I >= 0 && str[I] != '-'; --I) (void)1; ++I;
    while (str[I] != '\0')
        string_append(&check, str[I++]);

    _Bool done = get_keysym_from_str(string_data(&check)) != (KeySym)0;
    string_destroy(&check);
    return done;
}

ConfigError reconcile_bind(Token* t_return) {
    if (t_return == NULL)
        return create_config_error(
            ANGEL_INTERNAL_ERROR,
            -1, -1
        );

    string expanded = create_string();
    while (!token_is_string(&next_token) 
            && !token_is_newline(&next_token) 
            && !token_is_semicolon(&next_token)
            && !token_is_eof(&next_token)
            && get_action_fn(get_token_identifier_type(&next_token)) == action_noop
    ) {
        if (token_is_dollar_sign(&next_token)) {
            get_next_token_print_error(&next_token);

            // Manage
            int ended_at = 0;
            const char* variable_name = get_var_name_in_bind(
                &next_token,
                &ended_at
            );

            if (variable_name == NULL)
                return create_error_skip_statement(
                    &next_token,
                    ANGEL_EXPECTED_IDENTIFIER
                );

            const char* var_value = get_symbol_value(variable_name);
            if (var_value == NULL) {
                free((char*)variable_name);
                return create_error_skip_statement(
                    &next_token,
                    ANGEL_SYMBOL_NOT_FOUND
                );
            }

            free((char*)variable_name);

            string_append_chars(
                &expanded,
                var_value,
                get_raw_str_size(var_value)
            );


            const char* remainder = get_remainder_of_str(
                string_data(get_token_lexeme(&next_token)),
                ended_at
            );

            _Bool done = check_done(remainder);

            if (remainder != NULL) {
                string_append_chars(
                    &expanded,
                    remainder,
                    get_raw_str_size(remainder)
                );

                free((char*)remainder);
            }

            if (done) {
                get_next_token_print_error(&next_token);
                break;
            }
        }

        else {
            string_append_chars(
                &expanded,
                string_data(get_token_lexeme(&next_token)),
                get_raw_str_size(
                    string_data(get_token_lexeme(&next_token))
                )
            );

            if (check_done(string_data(get_token_lexeme(&next_token)))) {
                get_next_token_print_error(&next_token);
                break;
            }
        }

        get_next_token_print_error(&next_token);
    }

    set_token_lexeme(t_return, expanded);
    set_token_identifier(t_return, string_copy(&expanded));
    return create_config_error(
        ANGEL_GOOD,
        0, 0
    );
}

ConfigError sub_parse_bind(
    const char** bind,
    const char** command,
    KeySym* keysym_return,
    unsigned int* state_return,
    int bind_type, 
    IdentifierType* idtype_return
) {
    if (bind == NULL 
        || keysym_return == NULL 
        || state_return == NULL 
    ) {
        return create_config_error(
            ANGEL_INTERNAL_ERROR,
            -1, -1
        );
    }

    get_next_token_print_error(&next_token);

    if (!token_is_ident(&next_token) 
        && !token_is_string(&next_token) 
        && !token_is_dollar_sign(&next_token)
    ) {
        return create_error_skip_statement(
            &next_token,
            ANGEL_UNEXPECTED_TOKEN
        );
    }

    int bind_line_no = get_token_line_no(&next_token);
    int bind_col_no = get_token_col_no(&next_token);

    *bind = NULL;
    if (token_is_string(&next_token))  {
        *bind = copy_lexeme(&next_token);
        get_next_token_print_error(&next_token);
    }

    else {
        Token phony_token = {0};
        ConfigError err = reconcile_bind(&phony_token);
        if (!config_error_is_good(&err)) {
            token_destroy(&phony_token);
            return err;
        }

        *bind = copy_lexeme(&phony_token);
        token_destroy(&phony_token);
    }

    ConfigError bind_parse_error = decompose_bind(
        *bind,
        state_return,
        keysym_return,
        bind_line_no, 
        bind_col_no
    );

    if (!config_error_is_good(&bind_parse_error)) {
        free((char*)*bind);
        return bind_parse_error;
    }

    if (bind_type == BIND_COMMAND) {
        if (!token_is_ident(&next_token) 
            && !token_is_dollar_sign(&next_token) 
            && !token_is_string(&next_token)
        ) {
            free((char*)*bind);
            return create_error_skip_statement(
                &next_token,
                ANGEL_UNEXPECTED_TOKEN
            );
        }
    }

    else {
        if (!token_is_ident(&next_token) && !token_is_dollar_sign(&next_token)) {
            free((char*)*bind);
            return create_error_skip_statement(
                &next_token,
                ANGEL_UNEXPECTED_TOKEN
            );
        }
    }

    const char* value = NULL; 
    Token t_return = {0};
    if (token_is_dollar_sign(&next_token)) {
        ConfigError err = handle_variable(&value, &t_return);     
        if (!config_error_is_good(&err)) {
            free((char*)bind);
            return err;
        }
    }

    if (command != NULL)
        *command = copy_lexeme(value == NULL ? &next_token : &t_return);

    if (idtype_return != NULL)
        *idtype_return = get_token_identifier_type(value == NULL ? &next_token : &t_return);

    get_next_token_print_error(&next_token);
    if (!token_is_newline(&next_token) && !token_is_semicolon(&next_token)) {
        token_destroy(&t_return);
        free((char*)value);
        free((char*)*command);
        free((char*)*bind);
        return create_error_skip_statement(
            &next_token,
            ANGEL_EXPECTED_IDENTIFIER
        );
    }

    token_destroy(&t_return);
    free((char*)value);
    return create_config_error(
        ANGEL_GOOD,
        0, 0
    );
}

ConfigError parse_bind_command() {
    const char* bind = NULL;
    const char* command = NULL;
    KeySym keysym_return;
    unsigned int state_return;

    ConfigError err = sub_parse_bind(
        &bind, &command,
        &keysym_return, &state_return,
        BIND_COMMAND, NULL
    );

    if (!config_error_is_good(&err)) 
        return err;

    // Command is now owned by KeyMaps
    command_bind_create(
        create_keymap(keysym_return, state_return),
        command
    );

    free((char*)bind);
    get_next_token_print_error(&next_token);
    return create_config_error(
        ANGEL_GOOD,
        0, 0
    );
}

ConfigError parse_declare() {
    get_next_token_print_error(&next_token);

    const char* value = NULL;
    Token t_return = {0};
    if (token_is_dollar_sign(&next_token)) {
        ConfigError err = handle_variable(&value, &t_return);
        if (!config_error_is_good(&err))
            return err;
    }

    if (!token_is_ident(&next_token) && !token_is_string(&next_token))
        return create_error_skip_statement(
            &next_token,
            ANGEL_UNEXPECTED_TOKEN
        );

    const char* name = copy_lexeme(value == NULL ? &next_token : &t_return);
    token_destroy(&t_return);

    get_next_token_print_error(&next_token);

    if (!token_is_ident(&next_token))  {
        free((char*)name);
        return create_error_skip_statement(
            &next_token,
            ANGEL_UNEXPECTED_TOKEN
        );
    }

    InitialState state = get_initial_state_from_ident_type(&next_token);
    if (state == NULL_STATE) {
        free((char*)name);
        return create_error_skip_statement(
            &next_token,
            ANGEL_BAD_STATE
        );
    }

    get_next_token_print_error(&next_token);

    if (!token_is_newline(&next_token) && !token_is_semicolon(&next_token)) {
        free((char*)name);
        return create_error_skip_statement(
            &next_token,
            ANGEL_EXPECTED_DELIMITER
        );
    }

    create_initial_state(name, state);

    return create_config_error(
        ANGEL_GOOD,
        0, 0
    );
}

ConfigError parse_bind() {
    const char* bind = NULL;
    IdentifierType action_type;
    KeySym keysym_return;
    unsigned int state_return;

    ConfigError err = sub_parse_bind(
        &bind, NULL,
        &keysym_return, &state_return,
        BIND_ACTION, &action_type
    );

    if (!config_error_is_good(&err)) 
        return err;

    Remap remap = {
        keysym_return, 
        state_return,
        get_action_fn(action_type)
    };

    install_remap(&remap);

    free((char*)bind);
    get_next_token_print_error(&next_token);
    return create_config_error(
        ANGEL_GOOD,
        0, 0
    );
}

ConfigError parse_unknown_ident() {
    int line_no = get_token_line_no(&next_token);
    int col_no = get_token_col_no(&next_token);
    skip_to_next_statement(&next_token);
    return create_config_error(
        ANGEL_UNKNOWN_TOKEN,
        line_no, col_no
    );
}

ConfigError parse_statement() {
    return get_parse_fn(&next_token)();
}

_Bool parse_config() {
    if (!parser_init()) {
        lerr("Failed to initialize config parser");
        return false;
    }

    _Bool healthy = true;

    get_next_token_print_error(&next_token);
    while (!token_is_eof(&next_token)) {
        if (token_is_newline(&next_token) || token_is_semicolon(&next_token)) {
            get_next_token_print_error(&next_token);
            continue;
        }

        ConfigError config_err = parse_statement();
        print_error_message(&config_err);

        if (!config_error_is_good(&config_err) 
            && !config_error_is_eof(&config_err)
        ) {
            healthy = false;
        }
    }

    parser_destroy();
    return healthy;
}

void parser_destroy() {
    token_destroy(&next_token);
    lex_destroy();
    sym_table_destroy();
}
