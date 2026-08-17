/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "logging.h"

#include <stdio.h>

void lerr(char* message) {
    fprintf(stderr, "%s: %s\n", "\033[31m[ERROR]\033[0m", message);
}

void lwarn(char* message) {
    fprintf(stderr, "%s: %s\n", "\033[33m[WARNING]\033[0m", message);
}

void linfo(char* message) {
    fprintf(stderr, "%s: %s\n", "\033[32m[INFO]\033[0m", message);
}
