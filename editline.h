#ifndef NEX_EDITLINE_H
#define NEX_EDITLINE_H

#include "util.h"
#include "strbuf.h"

enum editline_status {
    editline_ok,
    editline_fail,
    editline_accept,
};

enum editline_status editline_handle_key(struct strbuf *line, size_t *cursor, int key) NEX_NONNULL;

#endif // NEX_EDITLINE_H
