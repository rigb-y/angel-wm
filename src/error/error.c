/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "error.h"
#include "types.h"
#include "setup.h"
#include "logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

_Bool wm_present = false;

/**
 * @brief Sets the error handler for X server error handling
 *
 * @param Handler pointer to handler function
 * @return Handler old handler replaced by handler
 */
Handler set_error_handler(Handler handler) {
    return XSetErrorHandler(handler);
}

int error_handler(Display* dp, XErrorEvent* error_event) {
    if (error_event->error_code == BadAccess) {
        wm_present = true;
    }

    char error_str[DEFAULT_ERROR_STR_LEN];

    XGetErrorText(
        dp, 
        error_event->error_code,
        error_str,
        DEFAULT_ERROR_STR_LEN
    );

    fprintf(
        stderr, 
        "%s:\n%s: %lu\n%s: %d\n%s: %d\n%s: %lu\n", 
        error_str, 
        "Serial", error_event->serial,
        "Minor code", error_event->minor_code,
        "Request code", error_event->request_code,
        "Resource ID", error_event->resourceid
    );

    return 0;
}

IOHandler set_io_error_handler(IOHandler handler) {
    return XSetIOErrorHandler(handler);
}

int io_error_handler(Display* dp) {
    lerr("IO error");

    return 0;
}
