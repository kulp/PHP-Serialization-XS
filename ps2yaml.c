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
    rc = dumper(f, result, HD_PRINT_PRETTY);
    rc = (*p_fini)(state);
    rc = ps_fini(&state);
    ps_free(result);

    fclose(f);

    return rc;
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

