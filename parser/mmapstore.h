#ifndef MMAPSTORE_H_
#define MMAPSTORE_H_

#include "ps_parser.h"
#include "ps_parser_store.h"

int ps_mmap_file_init(struct ps_parser_state *state, void *data);
int ps_mmap_file_fini(struct ps_parser_state *state);

#endif /* FILESTORE_H_ */

