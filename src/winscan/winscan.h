/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_WINSCAN_H
#define ANGEL_WINSCAN_H

#include <X11/Xlib.h>

Status scan_existing_windows(Window, Window*, Window*, Window**, unsigned int*);

#endif
