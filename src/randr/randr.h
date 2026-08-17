/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_RANDR_H
#define ANGEL_RANDR_H

#include <X11/extensions/Xrandr.h>

typedef struct RandrExtensionInfo {
    int event_base, error_base;
} RandrExtensionInfo;

extern RandrExtensionInfo randr_bases;

_Bool ensure_randr_exists_and_get_bases(int*, int*);

int get_active_monitor_count();
XRRMonitorInfo* get_active_monitor_info(int*);
void free_active_monitor_info(XRRMonitorInfo*);

const char* get_monitor_name_str(XRRMonitorInfo*, int, int);
void dump_monitor_info();

#endif
