/**
 * Converts a file in PHP-serialized format format to YAML or formatted
 * PHP-serialized format.
 *
 * Takes one argument, the file to parse. The output file may be specified with
 * the @c -o option. If no such option is provided, the output is written to @c
 * stdout. The default output option is YAML; this can also be specified with
 * the @c -f @c yaml option. The alternate format, pretty-printed PHPser, can
 * be specified with @c -f @c pretty.
 */

#include "ps_parser.h"
#include "mmapstore.h"
#include "filestore.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <yaml.h>

#define _err(...) do { \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
} while (0)

void usage(const char *me)
{
    printf("Usage:\n"
           "  %s [ OPTIONS ] [ filename ... ]\n"
           "where OPTIONS are among\n"
           "  -f fmt    select output format (\"yaml\" or \"pretty\")\n"
           "  -h        show this usage message\n"
           "  -o file   write output to this filename (default: stdout)\n"
           "filename may be '-' or may be omitted to read from stdin\n"
           "Note that if the -o option is specified and more than one input\n"
           "file is given, the output will be repeatedly overwritten, not\n"
           "appended to."
           , me);
}

static int node_emitter(yaml_emitter_t *e, const ps_node *node)
{
    int rc = 0;

    enum { NONE, SCALAR, COLLECTION } mode = NONE;

    yaml_event_t event;

    yaml_char_t *tag = NULL;
    const struct array *where = NULL;
    char *what;
    char tempbuf[20];
    int len;
    switch (node->type) {
        case NODE_INT:
            mode = SCALAR;
            len  = sprintf(what = tempbuf, "%lld", node->val.i);
            break;
        case NODE_BOOL:
            mode = SCALAR;
            what = node->val.b ? "true" : "false";
            len  = strlen(what);
            break;
        case NODE_STRING:
            mode = SCALAR;
            what = node->val.s.val;
            len  = node->val.s.len;
            break;
        case NODE_NULL:
            mode = SCALAR;
            what = "~";
            len = 1;
            break;
        case NODE_OBJECT:
            tag = (yaml_char_t*)node->val.o.type;
            where = &node->val.o.val;
            goto inside_array;
        case NODE_ARRAY:
            where = &node->val.a;
        inside_array:
            mode = COLLECTION;
            yaml_mapping_start_event_initialize(&event, NULL, tag, 0,
                    YAML_BLOCK_MAPPING_STYLE);
            yaml_emitter_emit(e, &event);

            for (int i = 0; i < where->len; i++) {
                if (!rc) rc = node_emitter(e, where->pairs[i].key);
                if (!rc) rc = node_emitter(e, where->pairs[i].val);
            }

            yaml_mapping_end_event_initialize(&event);
            yaml_emitter_emit(e, &event);
            break;
        default:
            _err("Unrecognized node type '%c' (%d)", node->type, node->type);
            return -1;
    }

    if (mode == SCALAR) {
        yaml_scalar_event_initialize(&event, NULL, NULL, (yaml_char_t*)what,
                len, true, true, YAML_PLAIN_SCALAR_STYLE);
        yaml_emitter_emit(e, &event);
    }

    return rc;
}

int ps_yaml(FILE *f, const ps_node *node, int flags)
{
    int rc = 0;

    yaml_emitter_t emitter;
    yaml_event_t event;

    yaml_emitter_initialize(&emitter);

    yaml_emitter_set_output_file(&emitter, f);

    yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING);
    if (!yaml_emitter_emit(&emitter, &event))
        goto error;

    yaml_document_start_event_initialize(&event, NULL, NULL, NULL, 0);
    if (!yaml_emitter_emit(&emitter, &event))
        goto error;

    rc = node_emitter(&emitter, node);

    yaml_document_end_event_initialize(&event, 1);
    if (!yaml_emitter_emit(&emitter, &event))
        goto error;

    yaml_stream_end_event_initialize(&event);
    if (!yaml_emitter_emit(&emitter, &event))
        goto error;

done:
    yaml_emitter_delete(&emitter);
    return rc;

error:
    rc = -1;

    goto done;
}

static int process(struct ps_parser_state *state, ps_dumper_t dumper,
        const char *in, const char *out)
{
    int rc = 0;

    ps_store_init p_init;
    ps_store_fini p_fini;
#if USE_MMAP
    p_init = ps_mmap_file_init;
    p_fini = ps_mmap_file_fini;
#else
    p_init = ps_read_file_init;
    p_fini = ps_read_file_fini;
#endif
    rc = ps_init(&state);
    rc = (*p_init)(state, (void*)in);
    if (rc) {
        fprintf(stderr, "Failed to open input file '%s'\n", in);
        return EXIT_FAILURE;
    }

    FILE *f;
    if (out && out[0] && strcmp(out, "-")) {
        f = fopen(out, "w");
        if (!f) {
            fprintf(stderr, "Failed to open output file '%s'\n", out);
            return EXIT_FAILURE;
        }
    } else {
        f = stdout;
    }

    ps_node *result = ps_parse(state);
    if (result == PS_PARSE_FAILURE)
        goto error;

    rc = dumper(f, result, PS_PRINT_PRETTY);
    rc = (*p_fini)(state);
    rc = ps_fini(&state);
    ps_free(result);

    fclose(f);

done:
    return rc;
error:
    rc = -1;
    goto done;
}

int main(int argc, char *argv[])
{
    int rc = EXIT_SUCCESS;

    struct ps_parser_state *state;
    ps_dumper_t dumper = ps_yaml;

    char *out = NULL;

    extern char *optarg;
    extern int optind, optopt;
    int ch;
    while ((ch = getopt(argc, argv, "f:ho:")) != -1) {
        switch (ch) {
            case 'f': 
                     if (!strcasecmp(optarg, "yaml"  )) dumper = ps_yaml;
                else if (!strcasecmp(optarg, "pretty")) dumper = ps_dump;
                else {
                    fprintf(stderr, "Invalid format '%s'\n", optarg);
                    return EXIT_FAILURE;
                }
                break;
            case 'o':
                out = strdup(optarg);
                break;
            default : rc = EXIT_FAILURE; /* FALLTHROUGH */
            case 'h': usage(argv[0]); return EXIT_FAILURE;
        }
    }

    if (argc - optind < 1)
        rc = process(state, dumper, "-", out);

    for (int i = optind; i < argc && !rc; i++)
        rc = process(state, dumper, argv[i], out);

    free(out);

    return rc;
}

/* vim:set et ts=4 sw=4 syntax=c.doxygen: */

