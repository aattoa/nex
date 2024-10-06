#include "editor.h"
#include "editline.h"
#include <stdarg.h>

struct editor editor_new(void) {
    return (struct editor) {
        .filebufs = vector_new(sizeof(struct filebuf), filebuf_destroy),
        .message = strbuf_new(),
        .cmdline = strbuf_new(),
        .editline = NULL,
        .mode = editor_mode_cmdline,
        .cmdline_cursor = 0,
        .editline_cursor = 0,
        .focus = 0,
        .line_index = 0,
    };
}

void editor_free(struct editor *editor) {
    vector_free(&editor->filebufs);
    strbuf_free(&editor->message);
    strbuf_free(&editor->cmdline);
    *editor = editor_new();
}

enum filebuf_status editor_add_filebuf(struct editor *editor, const char *path) {
    struct filebuf filebuf = filebuf_new();
    if (!strbuf_append(&filebuf.path, view_from(path))) {
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

bool editor_set_message(struct editor *editor, struct view message) {
    strbuf_clear(&editor->message);
    return strbuf_append(&editor->message, message);
}

bool editor_print_message(struct editor *editor, const char *restrict fmt, ...) {
    char message[256];

    va_list args;
    va_start(args, fmt);
    int result = vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    return result > 0 && editor_set_message(editor, view_from(message));
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
    enum editline_status status = editline_handle_key(&editor->cmdline, &editor->cmdline_cursor, key);
    if (status == editline_accept) {
        bool result = editor_execute_command(editor, strbuf_view(editor->cmdline));
        strbuf_clear(&editor->cmdline);
        editor->cmdline_cursor = 0;
        return result;
    }
    return status == editline_ok;
}

bool editor_handle_key_editline(struct editor *editor, int key) {
    enum editline_status status = editline_handle_key(editor->editline, &editor->editline_cursor, key);
    if (status == editline_accept) {
        editor->editline = NULL;
        editor->editline_cursor = 0;
        editor->mode = editor_mode_cmdline;
        return true;
    }
    return status == editline_ok;
}
