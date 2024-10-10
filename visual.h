#ifndef NEX_VISUAL_H
#define NEX_VISUAL_H

#include "util.h"
#include "filebuf.h"
#include "registers.h"

enum vi_mode {
    vi_mode_normal,
    vi_mode_insert,
    vi_mode_cmdline,
    vi_mode_register_pending,
};

struct vi_frame {
    size_t top, left;
};

struct vi_cursor {
    size_t x, y;
};

struct vi_state {
    struct vi_cursor cursor;
    struct vi_frame frame;
    enum vi_mode mode;
    size_t count;
    char regname;
};

enum vi_status {
    vi_ok,
    vi_fail,
    vi_leave,
};

enum vi_status vi_handle_key(struct filebuf *filebuf, struct vi_state *state, struct registers *registers, int key) NEX_NONNULL;

struct vi_state vi_state_new(void) NEX_CONST;

#endif // NEX_VISUAL_H
