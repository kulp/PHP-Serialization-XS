#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include "ps_parser.h"
#include "ps_parser_internal.h"

SV *
_convert_recurse(const ps_node *node, const char *prefix)
{
    SV *result = &PL_sv_undef;

    const union nodeval *v = &node->val;
    char *typename = NULL;
    const struct array *what = NULL;
    SV *a = NULL;
    switch (node->type) {
        case NODE_STRING: result = newSVpv(v->s.val, v->s.len);            break;
        case NODE_INT:    result = newSViv(v->i);                          break;
        case NODE_FLOAT:  result = newSVnv(v->d);                          break;
        case NODE_BOOL:   result = newSVsv(v->b ? &PL_sv_yes : &PL_sv_no); break;
        case NODE_NULL:   result = newSVsv(&PL_sv_undef);                  break;
        case NODE_OBJECT:
            what = &node->val.o.val;
            typename = v->o.type;
            goto inside_array;
        case NODE_ARRAY: {
            what = &node->val.a;
        inside_array:
            // len == 0 could be hash still
            if (what->is_array && what->len != 0) {
                a = (SV*)newAV();
                av_extend((AV*)a, what->len - 1);
                for (int i = 0; i < what->len; i++)
                    av_push((AV*)a, _convert_recurse(what->pairs[i].val, prefix));
            } else {
                a = (SV*)newHV();
                for (int i = 0; i < what->len; i++) {
                    int len;
                    char *key = SvPV(_convert_recurse(what->pairs[i].key, prefix), len);
                    SV   *val =      _convert_recurse(what->pairs[i].val, prefix);

                    hv_store((HV*)a, key, len, val, 0);
                }
            }

            result = newRV(a);
            if (typename) {
                bool should_free = false;
                char *built = typename;
                if (prefix) {
                    should_free = true;
                    size_t size = snprintf(NULL, 0, "%s::%s", prefix, typename);
                    built = malloc(size + 1);
                    snprintf(built, size + 1, "%s::%s", prefix, typename);
                }

                sv_bless(result, gv_stashpv(built, true));

                if (should_free)
                    free(built);
            }

            break;
        }
    }

    return result;
}

