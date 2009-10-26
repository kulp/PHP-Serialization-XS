#include "mmapstore.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define _err(...) do { \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
} while (0)

struct filestate {
    int fd;
    char *data;
    struct stat stat;
};

static const char* _chunker(void *data, unsigned long offset, size_t count)
{
    struct filestate *state = data;

    return &state->data[offset];
}

int ps_mmap_file_fini(struct ps_parser_state *parser_state)
{
    struct filestate *state = ps_get_userdata(parser_state);

    if (!state) return -1;

    munmap(state->data, state->stat.st_size);
    close(state->fd);
    free(state);

    return 0;
}

int ps_mmap_file_init(struct ps_parser_state *parser_state, void *userdata)
{
    int rc = 0;

    const char *filename = userdata;

    struct filestate *state = malloc(sizeof *state);

    state->fd = open(filename, O_RDONLY);
    if (state->fd < 0) {
        _err("File '%s' could not be opened (%d: %s)",
             filename, errno, strerror(errno));
        return -1;
    }

    rc = fstat(state->fd, &state->stat);
    if (rc) {
        _err("fstat: %d: %s", errno, strerror(errno));
        return rc;
    }

    state->data = mmap(NULL, state->stat.st_size, PROT_READ, MAP_PRIVATE, state->fd, 0);

    ps_set_userdata(parser_state, state);
    ps_set_chunker(parser_state, _chunker);

    return rc;
}

/* vim:set ts=4 sw=4 syntax=c.doxygen: */

