#include "filestore.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define _err(...) do { \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
} while (0)

struct filestate {
    FILE *f;
    char *data;
    size_t len;
    size_t start, end;
};

static const char* _chunker(void *data, unsigned long offset, size_t count)
{
    struct filestate *state = data;

    /*
     * This code could be optimized, as it might read the same data more than
     * once. However, we know that the particular usage of this program will
     * read data unidirectionally in small chunks, so optimization and / or
     * generalization are probably unwarranted.
     */

    if (offset < state->start || offset + count >= state->end) {
        if (count > state->len) {
            void *temp = realloc(state->data, state->len = count * 2);
            if (!temp)
                return NULL;
            state->data = temp;
        }

        if (fseek(state->f, state->start = offset, SEEK_SET))
            return NULL;
        state->end = state->start + fread(state->data, 1, state->len, state->f);
        if (state->end <= state->start)
            return NULL;
    }

    return &state->data[offset - state->start];
}

int ps_read_file_init(struct ps_parser_state *parser_state, void *userdata)
{
    int rc = 0;

    const char *filename = userdata;

    struct filestate *state = malloc(sizeof *state);

    if (!strcmp(filename, "-")) {
        state->f = stdin;
    } else {
        state->f = fopen(filename, "r");
        if (!state->f) {
            _err("File '%s' could not be opened (%d: %s)",
                 filename, errno, strerror(errno));
            return -1;
        }
    }

    state->len = BUFSIZ;
    state->data = malloc(state->len);
    state->start = state->end = 0;

    ps_set_userdata(parser_state, state);
    ps_set_chunker(parser_state, _chunker);

    return rc;
}

int ps_read_file_fini(struct ps_parser_state *parser_state)
{
    struct filestate *state = ps_get_userdata(parser_state);

    if (!state) return -1;

    if (state->f != stdin)
        fclose(state->f);
    free(state);

    return 0;
}

/* vim:set ts=4 sw=4 syntax=c.doxygen: */

