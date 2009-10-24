#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include "ps_parser.h"
#include "stringstore.h"

MODULE = PHP::Serialization::XS		PACKAGE = PHP::Serialization::XS		

SV *
_c_decode(input, ....)
        SV *input
    CODE:
        struct ps_parser_state *ps_state;
        if (ps_init(&ps_state))
            // TODO differentiate error undef from top-level null
            XSRETURN_UNDEF;

        const char *str = SvPV_nolen_const(input);
        ps_read_string_init(ps_state, (void*)str);
        struct ps_node *node = ps_parse(ps_state);

        const char *claxx = NULL;
        if (items > 1 && SvOK(ST(1)))
            claxx = (char *)SvPV_nolen(ST(1));

        extern SV* _convert_recurse(const ps_node *, const char *);
        RETVAL = _convert_recurse(node, claxx);

        ps_free(node);
        ps_fini(&ps_state);
    OUTPUT:
        RETVAL
