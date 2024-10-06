#ifndef NEX_EDITOR_H
#define NEX_EDITOR_H

#include "util.h"
#include "filebuf.h"

enum editor_mode {
    editor_mode_quit,
    editor_mode_cmdline,
    editor_mode_editline,
};

struct editor {
    struct vector filebufs; // Element: `struct filebuf`
    struct strbuf message;
    struct strbuf cmdline;
    struct strbuf *editline;
    enum editor_mode mode;
    size_t cmdline_cursor;
    size_t editline_cursor;
    size_t focus;
    size_t line_index;
};

struct editor editor_new(void) NEX_CONST;

void editor_free(struct editor *editor) NEX_NONNULL;

enum filebuf_status editor_add_filebuf(struct editor *editor, const char *path) NEX_NONNULL;

enum filebuf_status editor_read_current_filebuf(struct editor *editor) NEX_NONNULL;

enum filebuf_status editor_write_current_filebuf(struct editor *editor) NEX_NONNULL;

struct filebuf *editor_current_filebuf(struct editor *editor) NEX_NONNULL NEX_NODISCARD;

struct strbuf *editor_current_line(struct editor *editor) NEX_NONNULL NEX_NODISCARD;

bool editor_set_focus(struct editor *editor, size_t focus) NEX_NONNULL;

bool editor_execute_command(struct editor *editor, struct view command) NEX_NONNULL;

bool editor_set_message(struct editor *editor, struct view message) NEX_NONNULL;

NEX_GNU_ATTRIBUTE(format(printf, 2, 3))
bool editor_print_message(struct editor *editor, const char *restrict fmt, ...) NEX_NONNULL;

bool editor_show_filename(struct editor *editor) NEX_NONNULL;

bool editor_handle_key_cmdline(struct editor *editor, int key) NEX_NONNULL;

bool editor_handle_key_editline(struct editor *editor, int key) NEX_NONNULL;

#endif // NEX_EDITOR_H
