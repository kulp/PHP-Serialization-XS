#ifndef FILESTORE_H_
#define FILESTORE_H_

#include "ps_parser.h"
#include "ps_parser_store.h"

int ps_read_file_init(struct ps_parser_state *state, void *data);
int ps_read_file_fini(struct ps_parser_state *state);

#endif /* FILESTORE_H_ */

