/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "angel.h"
#include "config_parser.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {
    _Bool check_config_health_flag = false;
    int matched_args = 0;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--check-config-health") == 0
            || strcmp(argv[i], "-c") == 0
        ) {
            check_config_health_flag = true;
            ++matched_args;
        }
    }

    if (matched_args != argc-1) {
        printf("Usage: angel [-c | --check-config-health]");
        return EXIT_FAILURE;
    }

    if (check_config_health_flag) {
        _Bool healthy = check_config_health();

        if (healthy)
            printf("\033[32m[Config parsed without error]\033[0m");
        else
            printf("\033[31m[Config parsed with error]\033[0m");

        return EXIT_SUCCESS;
    }

    return angel_manage_windows(argc, argv);
}
