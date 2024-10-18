#ifndef NEX_EDITLINE_H
#define NEX_EDITLINE_H

#include "util.h"
#include "strbuf.h"
#include "filebuf.h"

enum editline_status {
    editline_ok,
    editline_fail,
    editline_accept,
};

enum editline_mode {
    editline_mode_normal,
    editline_mode_insert,
};

struct editline_state {
    size_t history_index; // 1-based
    size_t cursor;
    size_t count;
    size_t leftmost_column;
    enum editline_mode mode;
};

enum editline_status editline_handle_key(struct strbuf *line, struct editline_state *state, struct filebuf *history, int key) NEX_GNU_ATTRIBUTE(nonnull(1, 2));

struct editline_state editline_state_new(void) NEX_CONST;

#endif // NEX_EDITLINE_H
