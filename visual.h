#ifndef NEX_VISUAL_H
#define NEX_VISUAL_H

#include "util.h"
#include "filebuf.h"
#include "registers.h"

enum vi_mode {
    vi_mode_normal,
    vi_mode_insert,
    vi_mode_select,
    vi_mode_cmdline,
    vi_mode_register_pending,
};

enum vi_context {
    vi_context_file,
    vi_context_line,
};

struct vi_frame {
    size_t top, left;
};

struct vi_state {
    struct position select_start;
    struct position cursor;
    struct vi_frame frame;
    enum vi_mode mode;
    enum vi_context context;
    size_t count;
    char regname;
};

enum vi_status {
    vi_ok,
    vi_fail,
    vi_leave,
    vi_line_accept,
};

enum vi_status vi_handle_key(struct filebuf *filebuf, struct vi_state *state, struct registers *registers, int key) NEX_NONNULL;

struct strbuf *vi_current_line(struct filebuf *filebuf, struct vi_state *state) NEX_NONNULL;

struct vi_state vi_state_new(enum vi_context context) NEX_CONST;

const char *vi_mode_describe(enum vi_mode mode) NEX_CONST;

#endif // NEX_VISUAL_H
