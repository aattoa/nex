#include "editor.h"
#include "keycodes.h"
#include <ctype.h>
#include <limits.h>

static bool cmdline_insert_printable(struct editor *editor, char character) {
    if (strbuf_insert(&editor->cmdline, editor->cmdline_cursor, character)) {
        ++editor->cmdline_cursor;
        return true;
    }
    return false;
}

static bool cmdline_backspace(struct editor *editor) {
    if (strbuf_erase(&editor->cmdline, editor->cmdline_cursor - 1)) {
        --editor->cmdline_cursor;
        return true;
    }
    return false;
}

static bool cmdline_cursor_left(struct editor *editor) {
    if (editor->cmdline_cursor != 0) {
        --editor->cmdline_cursor;
        return true;
    }
    return false;
}

static bool cmdline_cursor_right(struct editor *editor) {
    if (editor->cmdline_cursor != editor->cmdline.len) {
        ++editor->cmdline_cursor;
        return true;
    }
    return false;
}

static bool cmdline_write(struct editor *editor) {
    enum filebuf_status status = editor_write_current_filebuf(editor);
    if (status == filebuf_ok) {
        return true;
    }
    editor_print_message(editor, "Unable to write: %s", filebuf_status_describe(status));
    return false;
}

static void show_filename(struct editor *editor) {
    struct filebuf *filebuf = vector_at(&editor->filebufs, editor->focus);
    if (filebuf != NULL && filebuf->path.len != 0) {
        editor_print_message(editor, "Editing '%.*s'", (int)filebuf->path.len, filebuf->path.ptr);
    }
    else {
        editor_print_message(editor, "No file");
    }
}

static bool cmdline_next(struct editor *editor) {
    if (editor->focus < editor->filebufs.len) {
        ++editor->focus;
        show_filename(editor);
        return true;
    }
    return false;
}

static bool cmdline_back(struct editor *editor) {
    if (editor->focus != 0) {
        --editor->focus;
        show_filename(editor);
        return true;
    }
    return false;
}

static bool cmdline_execute(struct editor *editor, struct view view) {
    view_trim_whitespace(&view);
    if (view.len == 0) {
        return true;
    }
    if (view_deep_equal(view, view_from("q")) || view_deep_equal(view, view_from("quit"))) {
        editor->state = editor_state_quit;
        return true;
    }
    if (view_deep_equal(view, view_from("w")) || view_deep_equal(view, view_from("write"))) {
        return cmdline_write(editor);
    }
    if (view_deep_equal(view, view_from("n")) || view_deep_equal(view, view_from("next"))) {
        return cmdline_next(editor);
    }
    if (view_deep_equal(view, view_from("b")) || view_deep_equal(view, view_from("back"))) {
        return cmdline_back(editor);
    }
    editor_print_message(editor, "Unrecognized command: %.*s", (int)view.len, view.ptr);
    return false;
}

static bool cmdline_enter(struct editor *editor) {
    bool result = cmdline_execute(editor, strbuf_view(editor->cmdline));
    strbuf_clear(&editor->cmdline);
    editor->cmdline_cursor = 0;
    return result;
}

bool editor_cmdline_handle_key(struct editor *editor, int key) {
    if (key >= 0 && key <= UCHAR_MAX && isprint(key)) {
        return cmdline_insert_printable(editor, key);
    }
    switch (key) {
    case NEX_KEY_LEFT:      return cmdline_cursor_left(editor);
    case NEX_KEY_RIGHT:     return cmdline_cursor_right(editor);
    case NEX_KEY_BACKSPACE: return cmdline_backspace(editor);
    case NEX_KEY_ENTER:     return cmdline_enter(editor);
    default:                return false;
    }
}
