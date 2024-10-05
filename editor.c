#include "editor.h"
#include <stdarg.h>

struct editor editor_new(void) {
    return (struct editor) {
        .filebufs = vector_new(sizeof(struct filebuf), filebuf_destroy),
        .message = strbuf_new(),
        .cmdline = strbuf_new(),
        .cmdline_cursor = 0,
        .focus = 0,
        .state = editor_state_cmdline,
    };
}

void editor_free(struct editor *editor) {
    strbuf_free(&editor->message);
    strbuf_free(&editor->cmdline);
    vector_free(&editor->filebufs);
    *editor = editor_new();
}

struct filebuf *editor_current_filebuf(struct editor *editor) {
    return vector_at(&editor->filebufs, editor->focus - 1);
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
