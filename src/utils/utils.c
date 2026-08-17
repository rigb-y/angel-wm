/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "utils.h"
#include "windows.h"
#include "workspaces.h"
#include "client_list.h"
#include "client.h"
#include "detached.h"
#include "float_list.h"
#include "minimized_list.h"
#include "minimized_client.h"
#include "unmapped_client.h"
#include "unmapped_list.h"
#include "monitor.h"
#include "monitors.h"
#include "angel.h"
#include "logging.h"
#include "docks.h"

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "string.h"

static int get_past_next_dsign(const char*, int, int, int*);
static int get_size_of_expanded(const char*, int);

void dump_window(Window window) {
    char* window_name = get_window_name(window);
    printf("Name: %s", window_name);

    XWindowAttributes* attrs = get_win_attrs(window);
    if (attrs == NULL)
        return;

    printf(
        "name: %s\n"
        "Resource ID: %lu\n"
        "x: %d\n"
        "y: %d\n"
        "width: %d\n"
        "height: %d\n"
        "map_state: %d\n"
        "override_redirect: %d\n\n",
        window_name, window,
        attrs->x, attrs->y,
        attrs->width, attrs->height,
        attrs->map_state, attrs->override_redirect
    );

    free(window_name);
    free(attrs);
}

void dump_existing_windows(Window* windows, unsigned int nwindows) {
    for (unsigned int i = 0; i < nwindows; ++i) {
        dump_window(windows[i]);
    }
}

int max(int a, int b) {
    return a >= b ? a : b;
}

int min(int a, int b) {
    return a <= b ? a : b;
}

int fastpow(int base, int exp) {
    int res = 1;
    while (exp > 0) {
        if (exp & 1)
            res*=base;

        base *= base;
        exp >>= 1;
    }

    return res;
}

int abs(int x) {
    return x < 0 ? -x : x;
}

size_t get_raw_str_size(const char* name) {
    if (name == NULL) return 0;

    int k = -1; while (name[++k] != '\0') (void)1; return k;
}

void __DEBUG_EMPTY_FN() { (void)0; }

void __debug_dump_cl(ClientList* cl) {
    if (cl == NULL) return;

    printf("\n\nTiled:\n");
    Client* curr = cl_head(cl);
    while (curr != NULL) {
        printf("%p\n", curr);
        curr = curr->next;
    }
}

void __debug_dump_fl(FloatingClients* fl) {
    if (fl == NULL) return;

    printf("\n\nFloating:\n");
    DetachedClient* curr = fl_head(fl);
    while (curr != NULL) {
        printf("%p Client(%p)\n", 
            curr,
            get_client_from_detached(curr)
        );
        curr = curr->next;
    }
}

void __debug_dump_ml(MinimizedList* ml) {
    if (ml == NULL) return;

    printf("\n\nMinimized:\n");
    MinimizedClient* curr = ml_head(ml);
    while (curr != NULL) {
        printf("%p Client(%p)\n", 
            curr,
            get_client_from_minimized(curr)
        );
        curr = curr->next;
    }
}

void __debug_dump_ul(UnmappedClients* ul) {
    if (ul == NULL) return;

    printf("\n\nUnmapped:\n");
    UnmappedClient* curr = ul_head(ul);
    while (curr != NULL) {
        printf("%p Client(%p)\n", 
            curr,
            get_client_from_unmapped(curr)
        );
        curr = curr->next;
    }
}

void __debug_dump_dl(Docks* dl) {
    if (dl == NULL) return;

    printf("\n\nDocks:\n");
    Dock* curr = dl->head;
    while (curr != NULL) {
        printf("%p Client(%p)\n", 
            curr,
            get_client_from_dock(curr)
        );
        curr = curr->next;
    }
}

void __debug_dump_monitor(Monitor* monitor) {
    if (monitor == NULL) return;

    const char* monitor_name = XGetAtomName(
        dp,
        get_monitor_name(monitor)
    );

    printf("\n\nMonitor: %s\n", monitor_name);
    __debug_dump_cl(get_monitor_cl(monitor));
    __debug_dump_fl(get_monitor_fl(monitor));
    __debug_dump_ml(get_monitor_ml(monitor));
    __debug_dump_ul(get_monitor_ul(monitor));
    __debug_dump_dl(get_monitor_dl(monitor));

    free((char*)monitor_name);
}

void __debug_dump_monitors(Monitors* monitors) {
    if (monitors == NULL) return;

    Monitor* curr = monitors_head(monitors);
    while (curr != NULL) {
        __debug_dump_monitor(curr);
        curr = curr->next;
    }
}

void __debug_dump_workspaces(int limit) {
    if (!workspace_is_valid(limit))
        limit = N_WORKSPACES;

    for (int i = 0; i < limit; ++i) {
        printf("Workspace: %d\n", i);
        __debug_dump_monitors(
            get_workspace_monitors(i)
        );
    }
}

int get_past_next_dsign(const char* str, int start, int n, int* skipped) {
    if (str == NULL || start < 0 || start >= n) return -1;

    for (; start < n; ++start) {
        if (str[start] == '$')
            return start+1;
        ++(*skipped);
    }

    return -1;
}

int get_size_of_expanded(const char* s, int n) {
    if (s == NULL) 
        return 0;

    char buf[n+1]; int total = 0, k = 0, dsign = 0;
    while ((dsign = get_past_next_dsign(s, dsign, n, &total)) != -1) {
        for (; dsign < n && s[dsign] != '/'; ++dsign)
            buf[k++] = s[dsign];

        buf[k] = '\0';

        const char* expanded; 
        if ((expanded = getenv(buf)) == NULL) {
            lerr("Failed to expand environment variables in given path");
            return -1;
        }

        total += strlen(expanded);
        memset(buf, 0, k+1);
        k=0;

        if (dsign == n) break;
    }

    return total;
}

const char* expand_path(const char* path, int n) {
    int total_size = get_size_of_expanded(path, n);
    if (total_size == -1)
        return NULL;

    char* expanded = malloc(sizeof(char) * total_size + 1);

    char buf[n+1];
    int start = 0, skipped = 0, dsign = 0, k = 0, ell = 0;
    while ((dsign = get_past_next_dsign(path, dsign, n, &skipped)) != -1) {
        for (; start < n && start < skipped; ++start)
            expanded[k++] = path[start];

        for (; dsign < n && path[dsign] != '/'; ++dsign)
            buf[ell++] = path[dsign];

        buf[ell] = '\0';
        const char* expand = getenv(buf);
        if (expand == NULL) {
            lerr("Failed to expand environment variables in given path");

            free(expanded);
            return NULL;
        }

        memset(buf, 0, ell+1);
        ell = 0;

        memcpy(expanded + k, expand, strlen(expand));
        k+=strlen(expand);

        start = dsign;

        if (dsign == n) break;
    }

    for (; start < n; ++start) 
        expanded[k++] = path[start];

    expanded[total_size] = '\0';
    return expanded;
}

_Bool less(int a, int b) {
    return a < b;
}

_Bool greater(int a, int b) {
    return a > b;
}
