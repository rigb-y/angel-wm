/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_UTILS_H
#define ANGEL_UTILS_H

#include <X11/Xlib.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

typedef struct ClientList ClientList;
typedef struct Client Client;
typedef struct DetachedClient DetachedClient;
typedef struct FloatingClients FloatingClients;
typedef struct MinimizedClient MinimizedClient;
typedef struct MinimizedList MinimizedList;
typedef struct UnmappedClient UnmappedClient;
typedef struct UnmappedClients UnmappedClients;
typedef struct Monitor Monitor;
typedef struct Monitors Monitors;
typedef struct Docks Docks;

void dump_window(Window);
void dump_existing_windows(Window*, unsigned int);

int max(int, int);
int min(int, int);
int fastpow(int, int);
int abs(int);

size_t get_raw_str_size(const char*);

void __DEBUG_EMPTY_FN();

void __debug_dump_cl(ClientList*);
void __debug_dump_fl(FloatingClients*);
void __debug_dump_ml(MinimizedList*);
void __debug_dump_ul(UnmappedClients*);
void __debug_dump_dl(Docks*);
void __debug_dump_monitor(Monitor*);
void __debug_dump_monitors(Monitors*);
void __debug_dump_workspaces(int);

const char* expand_path(const char*, int);

_Bool less(int, int);
_Bool greater(int, int);

#endif
