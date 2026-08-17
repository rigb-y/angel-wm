/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_ERROR_H
#define ANGEL_ERROR_H

#include "types.h"

#include <X11/Xlib.h>

extern _Bool wm_present;

Handler set_error_handler(Handler);
int error_handler(Display*, XErrorEvent*);
int io_error_handler(Display*);
IOHandler set_io_error_handler(IOHandler);

#endif
