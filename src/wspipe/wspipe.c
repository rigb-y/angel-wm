#define _GNU_SOURCE

#include "wspipe.h"

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* pipe_path = NULL;

static const char *states[] = {
    [0] = "<fc=#000000,#FF73D4><box type=Full color=#FF73D4 width=1> 1 </box></fc>  2  3  4  5  6  7  8  9  10",
    [1] = "1  <fc=#000000,#FF73D4><box type=Full color=#FF73D4 width=1> 2 </box></fc>  3  4  5  6  7  8  9  10",
    [2] = "1  2  <fc=#000000,#FF73D4><box type=Full color=#FF73D4 width=1> 3 </box></fc>  4  5  6  7  8  9  10",
    [3] = "1  2  3  <fc=#000000,#FF73D4><box type=Full color=#FF73D4 width=1> 4 </box></fc>  5  6  7  8  9  10",
    [4] = "1  2  3  4  <fc=#000000,#FF73D4><box type=Full color=#FF73D4 width=1> 5 </box></fc>  6  7  8  9  10",
    [5] = "1  2  3  4  5  <fc=#000000,#FF73D4><box type=Full color=#FF73D4 width=1> 6 </box></fc>  7  8  9  10",
    [6] = "1  2  3  4  5  6  <fc=#000000,#FF73D4><box type=Full color=#FF73D4 width=1> 7 </box></fc>  8  9  10",
    [7] = "1  2  3  4  5  6  7  <fc=#000000,#FF73D4><box type=Full color=#FF73D4 width=1> 8 </box></fc>  9  10",
    [8] = "1  2  3  4  5  6  7  8  <fc=#000000,#FF73D4><box type=Full color=#FF73D4 width=1> 9 </box></fc>  10",
    [9] = "1  2  3  4  5  6  7  8  9  <fc=#000000,#FF73D4><box type=Full color=#FF73D4 width=1> 10 </box></fc>",
};

int create_pipe() {
    const char* runtime_dir = getenv("XDG_RUNTIME_DIR");
    if (runtime_dir == NULL) {
        fprintf(stderr, "XDG_RUNTIME_DIR nonexistent");
        return -1;
    }

    char path[512];

    int n = snprintf(
        path,
        sizeof(path),
        "%s/angel-wspipe",
        runtime_dir
    );

    if (n < 0 || (size_t)n >= sizeof(path)) {
        fprintf(stderr, "wspipe path too long");
        return -1;
    }

    char* pp; 
    if ((pp = malloc(sizeof(char) * n + 1)) == NULL) {
        perror("malloc");
        return -1;
    }

    memcpy(pp, path, n);
    pp[n] = '\0';

    if (mkfifo(path, 0600) == 0) {
        pipe_path = pp;
        return 0;
    }

    if (errno != EEXIST) {
        perror("mkfifo");
        free(pp);
        return -1;
    }

    struct stat st;

    if (stat(path, &st) == -1) {
        perror("stat");
        free(pp);
        return -1;
    }

    if (!S_ISFIFO(st.st_mode)) {
        fprintf(stderr, "%s exists but is not fifo", path);
        free(pp);
        return -1;
    }

    pipe_path = pp;
    return 0;
}

void write_ws_state(int cws) {
    if (pipe_path == NULL) return;

    int fd = open(pipe_path, O_WRONLY | O_NONBLOCK);

    if (fd == -1)
        return;

    const char* s = states[cws];
    if (dprintf(fd, "%s\n", s) == -1) {
        perror("write wspipe");
    }

    close(fd);
}

void destroy_pipe() {
    free((char*)pipe_path);
    pipe_path = NULL;
}
