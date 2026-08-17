/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_FOCUS_STACK
#define ANGEL_FOCUS_STACK

#include <X11/Xlib.h>

#define FS_REMOVE true
#define NO_FS_REMOVE false

typedef struct Client Client;
typedef struct FocusStack FocusStack;
typedef struct Monitor Monitor;
typedef struct Monitors Monitors;

// The focus stack does not own any clients. 
// Managing client memory will always remain with 
// the client list.
typedef struct FocusStack {
    Client* head;
    int n;
} FocusStack;

Client* fs_next_non_floating(FocusStack*);
_Bool fs_find(FocusStack*, Client*);
Client* fs_find_parent(FocusStack*, Client*);
void fs_remove(FocusStack*, Client*);
Client* fs_pop(FocusStack*);
void fs_push(FocusStack*, Client*);
Client* fs_head(FocusStack*);
_Bool fs_empty(FocusStack*);
_Bool fs_single(FocusStack*);

void justify_focus(Monitor*, Client*, Time);
Client* focus_from_cl_head_or_tail(Monitors*, Monitor*, int);
Client* get_next_focus(Monitor*, _Bool);

#endif
