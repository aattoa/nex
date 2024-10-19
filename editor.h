#ifndef NEX_EDITOR_H
#define NEX_EDITOR_H

#include "util.h"
#include "filebuf.h"
#include "terminal.h"
#include "settings.h"
#include "registers.h"
#include "visual.h"

enum editor_mode {
    editor_mode_quit,
    editor_mode_cmdline,
    editor_mode_editline,
    editor_mode_vi,
};

struct editor {
    struct registers registers;
    struct vector filebufs; // Element: `struct filebuf`
    struct strbuf message;
    struct filebuf cmdline_filebuf;
    struct filebuf *editline_filebuf;
    struct vi_state cmdline_state;
    struct vi_state editline_state;
    struct vi_state vi_state;
    struct termsize size;
    struct nex_settings settings;
    enum editor_mode mode;
    size_t focus;
};

struct editor editor_new(struct termsize size) NEX_CONST;

void editor_free(struct editor *editor) NEX_NONNULL;

enum filebuf_status editor_add_filebuf(struct editor *editor, const char *path) NEX_NONNULL;

enum filebuf_status editor_read_current_filebuf(struct editor *editor) NEX_NONNULL;

enum filebuf_status editor_write_current_filebuf(struct editor *editor) NEX_NONNULL;

struct filebuf *editor_current_filebuf(struct editor *editor) NEX_NONNULL NEX_NODISCARD;

struct strbuf *editor_current_editline(struct editor *editor) NEX_NONNULL NEX_NODISCARD;

bool editor_set_focus(struct editor *editor, size_t focus) NEX_NONNULL;

bool editor_execute_command(struct editor *editor, struct view command) NEX_NONNULL;

bool editor_print_message(struct editor *editor, const char *restrict fmt, ...) NEX_NONNULL NEX_GNU_ATTRIBUTE(format(printf, 2, 3));

bool editor_show_filename(struct editor *editor) NEX_NONNULL;

bool editor_handle_key_cmdline(struct editor *editor, int key) NEX_NONNULL;

bool editor_handle_key_editline(struct editor *editor, int key) NEX_NONNULL;

bool editor_handle_key_vi(struct editor *editor, int key) NEX_NONNULL;

void editor_cmdline_new(struct editor *editor) NEX_NONNULL;

void editor_cmdline_history_append(struct editor *editor, struct view entry) NEX_NONNULL;

void editor_cursor_scroll(size_t *first, size_t dimension, size_t cursor, size_t scrolloff);

struct strbuf editor_history_path(void) NEX_NODISCARD;

#endif // NEX_EDITOR_H
