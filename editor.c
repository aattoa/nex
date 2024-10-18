#include "editor.h"
#include "editline.h"
#include <stdlib.h>

static struct termsize validate_size(struct termsize size) {
    return (struct termsize) { .width = max_u16(size.width, 6), .height = max_u16(size.height, 2) };
}

struct editor editor_new(struct termsize size) {
    return (struct editor) {
        .registers = registers_new(),
        .cmdline_history = filebuf_new(),
        .filebufs = vector_new(sizeof(struct filebuf), filebuf_destroy),
        .message = strbuf_new(),
        .cmdline = strbuf_new(),
        .editline = NULL,
        .cmdline_state = editline_state_new(),
        .editline_state = editline_state_new(),
        .size = validate_size(size),
        .settings = nex_settings_new(),
        .mode = editor_mode_cmdline,
        .focus = 0,
        .line_index = 0,
    };
}

void editor_free(struct editor *editor) {
    registers_free(&editor->registers);
    filebuf_free(&editor->cmdline_history);
    vector_free(&editor->filebufs);
    strbuf_free(&editor->message);
    strbuf_free(&editor->cmdline);
    *editor = editor_new(editor->size);
}

enum filebuf_status editor_add_filebuf(struct editor *editor, const char *path) {
    struct filebuf filebuf = filebuf_new();
    if (!strbuf_push_view(&filebuf.path, view_from(path))) {
        return filebuf_bad_alloc;
    }
    enum filebuf_status status = filebuf_read(&filebuf);
    if (status != filebuf_ok) {
        filebuf_free(&filebuf);
        return status;
    }
    if (!vector_push(&editor->filebufs, &filebuf)) {
        filebuf_free(&filebuf);
        return filebuf_bad_alloc;
    }
    return filebuf_ok;
}

enum filebuf_status editor_read_current_filebuf(struct editor *editor) {
    struct filebuf *filebuf = editor_current_filebuf(editor);
    if (filebuf != NULL) {
        vector_clear(&filebuf->lines);
        return filebuf_read(filebuf);
    }
    return filebuf_no_path;
}

enum filebuf_status editor_write_current_filebuf(struct editor *editor) {
    struct filebuf *filebuf = editor_current_filebuf(editor);
    if (filebuf != NULL) {
        return filebuf_write(*filebuf);
    }
    return filebuf_no_path;
}

struct filebuf *editor_current_filebuf(struct editor *editor) {
    return vector_at(&editor->filebufs, editor->focus);
}

struct strbuf *editor_current_line(struct editor *editor) {
    struct filebuf *filebuf = editor_current_filebuf(editor);
    if (filebuf == NULL) {
        editor_print_message(editor, "No file focused");
        return NULL;
    }
    return vector_at(&filebuf->lines, editor->line_index);
}

bool editor_set_focus(struct editor *editor, size_t focus) {
    if (focus < editor->filebufs.len) {
        editor->line_index = 0;
        editor->focus = focus;
        editor_show_filename(editor);
        return true;
    }
    return false;
}

bool editor_print_message(struct editor *editor, const char *restrict fmt, ...) {
    strbuf_clear(&editor->message);
    va_list args;
    va_start(args, fmt);
    bool result = strbuf_vformat(&editor->message, 256, fmt, args);
    va_end(args);
    return result;
}

bool editor_show_filename(struct editor *editor) {
    struct filebuf *filebuf = editor_current_filebuf(editor);
    if (filebuf != NULL && filebuf->path.len != 0) {
        return editor_print_message(editor, "Editing '%.*s'", (int)filebuf->path.len, filebuf->path.ptr);
    }
    else {
        return editor_print_message(editor, "No file");
    }
}

bool editor_handle_key_cmdline(struct editor *editor, int key) {
    enum editline_status status = editline_handle_key(&editor->cmdline, &editor->cmdline_state, &editor->cmdline_history, key);
    if (status == editline_accept) {
        struct view command = strbuf_view(editor->cmdline);
        editor_history_add(&editor->cmdline_history, command);
        bool result = editor_execute_command(editor, command);
        strbuf_clear(&editor->cmdline);
        editor->cmdline_state = editline_state_new();
        editor->cmdline_state.history_index = editor->cmdline_history.lines.len;
        return result;
    }
    return status == editline_ok;
}

bool editor_handle_key_editline(struct editor *editor, int key) {
    enum editline_status status = editline_handle_key(editor->editline, &editor->editline_state, NULL, key);
    if (status == editline_accept) {
        editor->editline_state = editline_state_new();
        editor->mode = editor_mode_cmdline;
        return true;
    }
    return status == editline_ok;
}

bool editor_handle_key_vi(struct editor *editor, int key) {
    struct filebuf *filebuf = editor_current_filebuf(editor);
    if (filebuf == NULL) {
        return false;
    }
    enum vi_status status = vi_handle_key(filebuf, &editor->vi_state, &editor->registers, key);
    if (status == vi_leave) {
        editor->vi_state = vi_state_new();
        editor->mode = editor_mode_cmdline;
        return true;
    }
    return status == vi_ok;
}

void editor_cursor_scroll(size_t *first, size_t dimension, size_t cursor, size_t scrolloff) {
#define first (*first)
#define last (first + dimension - 1)
    if (cursor < first) {
        first -= (first - cursor);
    }
    else if (cursor > last) {
        first += (cursor - last);
    }
    if ((cursor - first) < scrolloff) {
        first -= min_uz(first, scrolloff - (cursor - first));
    }
    else if ((last - cursor) < scrolloff) {
        first += (scrolloff - (last - cursor));
    }
#undef last
#undef first
}

void editor_history_add(struct filebuf *history, struct view entry) {
    if (entry.len == 0) {
        return;
    }
    struct strbuf copy = strbuf_new();
    if (!strbuf_push_view(&copy, entry)) {
        return;
    }
    if (!vector_push(&history->lines, &copy)) {
        strbuf_free(&copy);
        return;
    }
    if (history->path.len == 0) {
        return;
    }
    FILE *stream = fopen(history->path.ptr, "a");
    if (stream == NULL) {
        return;
    }
    fprintf(stream, "%.*s\n", (int)entry.len, entry.ptr);
    fclose(stream);
}

struct strbuf editor_history_path(void) {
    // https://specifications.freedesktop.org/basedir-spec/latest/

    struct strbuf path = strbuf_new();
    const char *var;
    if ((var = getenv("NEX_HISTORY"))) {
        strbuf_push_view(&path, view_from(var));
    }
    else if ((var = getenv("XDG_STATE_HOME"))) {
        if (strbuf_push_view(&path, view_from(var))) {
            if (!strbuf_push_view(&path, view_from("/nex_history"))) {
                strbuf_free(&path);
            }
        }
    }
    else if ((var = getenv("HOME"))) {
        if (strbuf_push_view(&path, view_from(var))) {
            if (!strbuf_push_view(&path, view_from("/.local/state/nex_history"))) {
                strbuf_free(&path);
            }
        }
    }
    return path;
}
