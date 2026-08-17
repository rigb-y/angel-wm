/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#include "shell.h"
#include "angel.h"

#include <X11/Xlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>

void exec_command(const char* command) {
    if (command == NULL) return;

    pid_t pid = fork();
    if (pid == 0) {
        close(XConnectionNumber(dp));
        setsid();
        execl("/bin/sh", "sh", "-c", command, (char*)NULL);
        _exit(127);
    }
}

void exec_program(const char* command) {
    if (command == NULL) return;

    pid_t pid = fork();
    if (pid == 0) {
        close(XConnectionNumber(dp));
        setsid();
        execlp(command, command, (char*)NULL);
        _exit(127);
    }
}
