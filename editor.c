#include "editor.h"

struct editor editor_new(void) {
    return (struct editor) {
        .filebufs = vector_new(sizeof(struct filebuf), filebuf_destroy),
        .cmdline = strbuf_new(),
        .cmdline_cursor = 0,
        .focus = 0,
        .state = editor_state_cmdline,
    };
}

void editor_free(struct editor *editor) {
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
