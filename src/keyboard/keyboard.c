/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "keyboard.h"
#include "keymaps.h"
#include "utils.h"

#define XK_TECHNICAL

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>

static int modifier = DEFAULT_MOD;

const KeySym key_syms[] = {
    XK_a, XK_A, XK_b, XK_B,
    XK_c, XK_C, XK_d, XK_D,
    XK_e, XK_E, XK_f, XK_F,
    XK_g, XK_G, XK_h, XK_H,
    XK_i, XK_I, XK_j, XK_J,
    XK_k, XK_K, XK_l, XK_L,
    XK_m, XK_M, XK_n, XK_N,
    XK_o, XK_O, XK_p, XK_P,
    XK_q, XK_Q, XK_r, XK_R,
    XK_s, XK_S, XK_t, XK_T,
    XK_u, XK_U, XK_v, XK_V,
    XK_w, XK_W, XK_x, XK_X,
    XK_y, XK_Y, XK_z, XK_Z,
    XK_1, XK_2, XK_3, XK_4,
    XK_5, XK_6, XK_7, XK_8,
    XK_9, XK_0, XK_space, XK_Return,
    XK_Tab, XK_uparrow, XK_downarrow,
    XK_leftarrow, XK_rightarrow, 
    XK_semicolon, XK_space, XK_exclam,
    XK_quotedbl, XK_numbersign, XK_dollar,
    XK_percent, XK_ampersand, XK_apostrophe,
    XK_quoteright, XK_parenleft, XK_parenright,
    XK_asterisk, XK_plus, XK_comma,
    XK_minus, XK_period, XK_slash,
    XK_colon, XK_less, XK_equal,
    XK_greater, XK_question, XK_at,
    XK_bracketleft, XK_backslash, XK_bracketright,
    XK_asciicircum, XK_underscore, XK_grave,
    XK_braceleft, XK_bar, XK_braceright, XK_asciitilde,
};

KeyCode key_codes[ARRAY_SIZE(key_syms)];

typedef struct StringKeySymPair {
    const char* str;
    KeySym sym;
} StringKeySymPair;

static const StringKeySymPair string_to_key_sym[] = {
    {"a", XK_a}, {"A", XK_A}, {"b", XK_b}, {"B", XK_B},
    {"c", XK_c}, {"C", XK_C}, {"d", XK_d}, {"D", XK_D},
    {"e", XK_e}, {"E", XK_E}, {"f", XK_f}, {"F", XK_F},
    {"g", XK_g}, {"G", XK_G}, {"h", XK_h}, {"H", XK_H},
    {"i", XK_i}, {"I", XK_I}, {"j", XK_j}, {"J", XK_J},
    {"k", XK_k}, {"K", XK_K}, {"l", XK_l}, {"L", XK_L},
    {"m", XK_m}, {"M", XK_M}, {"n", XK_n}, {"N", XK_N},
    {"o", XK_o}, {"O", XK_O}, {"p", XK_p}, {"P", XK_P},
    {"q", XK_q}, {"Q", XK_Q}, {"r", XK_r}, {"R", XK_R},
    {"s", XK_s}, {"S", XK_S}, {"t", XK_t}, {"T", XK_T},
    {"u", XK_u}, {"U", XK_U}, {"v", XK_v}, {"V", XK_V},
    {"w", XK_w}, {"W", XK_W}, {"x", XK_x}, {"X", XK_X},
    {"y", XK_y}, {"Y", XK_Y}, {"z", XK_z}, {"Z", XK_Z},
    {"1", XK_1}, {"2", XK_2}, {"3", XK_3}, {"4", XK_4},
    {"5", XK_5}, {"6", XK_6}, {"7", XK_7}, {"8", XK_8},
    {"9", XK_9}, {"0", XK_0}, {";", XK_semicolon},
    {"!", XK_exclam}, {"\"", XK_quotedbl}, {"#", XK_numbersign},
    {"$", XK_dollar}, {"%", XK_percent}, {"&", XK_ampersand},
    {"\\", XK_apostrophe}, {"(", XK_parenleft}, {")", XK_parenright},
    {"*", XK_asterisk}, {"+", XK_plus}, {",", XK_comma},
    {"-", XK_minus}, {".", XK_period}, {"/", XK_slash},
    {":", XK_colon}, {"<", XK_less}, {"=", XK_equal},
    {">", XK_greater}, {"?", XK_question}, {"@", XK_at},
    {"[", XK_bracketleft}, {"\\", XK_backslash}, {"]", XK_bracketright},
    {"^", XK_asciicircum}, {"_", XK_underscore}, {"`", XK_grave},
    {"{", XK_braceleft}, {"|", XK_bar}, {"}", XK_braceright },
    {"~", XK_asciitilde}, {"ret", XK_Return}, {"tab", XK_Tab},
    {"uparrow", XK_uparrow}, {"downarrow", XK_downarrow},
    {"leftarrow", XK_leftarrow}, {"rightarrow", XK_rightarrow},
    {"space", XK_space},
};

KeySym get_keysym_from_str(const char* s) {
    if (s == NULL) return (KeySym)0;
    for (int i = 0; i < ARRAY_SIZE(string_to_key_sym); ++i)
        if (strcmp(string_to_key_sym[i].str, s) == 0)
            return string_to_key_sym[i].sym;
    return (KeySym)0;
}

typedef struct StringModifierPair {
    const char* str;
    int modifier;
} StringModifierPair;

static const StringModifierPair string_to_modifier[] = {
    {"ctrl", CONTROL}, 
    {"super", SUPER}, 
    {"alt", ALT}, 
    {"shift", SHIFT}
};

int get_modifier_from_str(const char* s) {
    if (s == NULL) return UNKNOWN_MODIFIER;
    for (int i = 0; i < ARRAY_SIZE(string_to_modifier); ++i) 
        if (strcmp(string_to_modifier[i].str, s) == 0)
            return string_to_modifier[i].modifier;
    return UNKNOWN_MODIFIER;
}

static int key_syms_index(KeySym);
static void establish_rpgs_from_action_list(ActionBind*);
static void establish_rpgs_from_command_list(CommandBind*);

void get_keycodes() {
    for (int i = 0; i < ARRAY_SIZE(key_syms); ++i) {
        key_codes[i] = get_keycode(key_syms[i]);
    }
}

void init_keyboard() {
    get_keycodes();
    set_default_binds();
}

void establish_rpgs_from_action_list(ActionBind* head) {
    ActionBind* default_bind = head;

    unsigned int ignored_states = LockMask | Mod2Mask;
    while (default_bind != NULL) {
        for (unsigned int extra = ignored_states;;
            extra = (extra - 1) & ignored_states
        ) {
            establish_root_passive_key_grab(
                default_bind->map.key_sym, 
                default_bind->map.state | extra
            );

            if (extra == 0)
                break;
        }

        default_bind = default_bind->next;
    }
}

void establish_rpgs_from_command_list(CommandBind* head) {
    CommandBind* command = head;

    unsigned int ignored_states = LockMask | Mod2Mask;
    while (command != NULL) {
        for (unsigned int extra = ignored_states;;
            extra = (extra - 1) & ignored_states
        ) {
            establish_root_passive_key_grab(
                command->map.key_sym,
                command->map.state | extra
            );

            if (extra == 0)
                break;
        }

        command = command->next;
    }
}

void establish_root_passive_key_grabs() {
    establish_rpgs_from_action_list(key_maps.default_head);
    establish_rpgs_from_action_list(key_maps.remap_head);
    establish_rpgs_from_command_list(key_maps.command_head);
}

void ungrab_all_root_passive_grabs() {
    XUngrabKey(dp, AnyKey, AnyModifier, root);
}

void establish_root_passive_key_grab(KeySym key, unsigned int state) {
    XGrabKey(
        dp,
        get_keycode(key),
        state,
        root,
        true,
        GrabModeAsync,
        GrabModeAsync
    );
}

int key_code_index(KeyCode code) {
    for (int i = 0; i < ARRAY_SIZE(key_codes); ++i) {
        if (key_codes[i] == code)
            return i;
    }
    return -1;
}

KeyCode get_keycode(KeySym ks) {
    return XKeysymToKeycode(dp, ks);
}

KeySym get_keysym(KeyCode kc) {
    return key_code_index(kc) != -1 
        ? key_syms[key_code_index(kc)] 
        : XK_A;
}

void refresh_keyboard(const XMappingEvent* mapping_event) {
    XRefreshKeyboardMapping((XMappingEvent*)mapping_event);
}

int get_modifier() {
    return modifier;
}

void set_modifier(int mod) {
    if (mod != SUPER
        && mod != ALT
        && mod != SHIFT
        && mod != CONTROL
    ) return;

    modifier = mod;
}
