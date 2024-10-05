#ifndef NEX_EDITOR_H
#define NEX_EDITOR_H

#include "filebuf.h"

enum editor_state {
    editor_state_cmdline,
};

struct editor {
    struct vector filebufs; // Element: `struct filebuf`
    struct strbuf cmdline;
    size_t cmdline_cursor;
    size_t focus; // Zero means no focus
    enum editor_state state;
};

struct editor editor_new(void) NEX_CONST;

void editor_free(struct editor *editor) NEX_NONNULL;

struct filebuf *editor_current_filebuf(struct editor *editor) NEX_NONNULL NEX_NODISCARD;

enum filebuf_status editor_add_filebuf(struct editor *editor, const char *path) NEX_NONNULL;

enum filebuf_status editor_read_current_filebuf(struct editor *editor) NEX_NONNULL;

enum filebuf_status editor_write_current_filebuf(struct editor *editor) NEX_NONNULL;

bool editor_cmdline_handle_key(struct editor *editor, int key) NEX_NONNULL;

#endif // NEX_EDITOR_H
