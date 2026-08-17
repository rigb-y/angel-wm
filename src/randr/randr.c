/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "randr.h"
#include "angel.h"

#include <X11/extensions/Xrandr.h>
#include <stdio.h>
#include <stdbool.h>

RandrExtensionInfo randr_bases = {0};

_Bool ensure_randr_exists_and_get_bases(int* event_base, int* error_base) {
    return XRRQueryExtension(dp, event_base, error_base);
}

int get_active_monitor_count() {
    int n = 0;
    XRRMonitorInfo* info = XRRGetMonitors(dp, root, true, &n);

    XRRFreeMonitors(info);
    return n;
}

XRRMonitorInfo* get_active_monitor_info(int* n) {
    return XRRGetMonitors(dp, root, true, n);
}

void free_active_monitor_info(XRRMonitorInfo* info) {
    if (info == NULL) return;
    XRRFreeMonitors(info);
}

const char* get_monitor_name_str(XRRMonitorInfo* info, int k, int n) {
    if (k < 0 || k >= n) return NULL;
    return XGetAtomName(dp, info[k].name);
}

void dump_monitor_info() {
    int n = 0;
    XRRMonitorInfo* info = get_active_monitor_info(&n);

    for (int i = 0; i < n; ++i) {
        const char* primary = info[i].primary ? "true" : "false";
        const char* name = get_monitor_name_str(info, i, n);
        printf("Monitor: %s (%d)\n"
            "Primary: %s\n"
            "x: %d\n" 
            "y: %d\n"
            "width: %d\n"
            "height: %d\n\n",
            name, 
            i,
            primary,
            info[i].x,
            info[i].y,
            info[i].width,
            info[i].height
        );
        XFree((char*)name);
    }

    XRRFreeMonitors(info);
}
