#include "editor.h"

static bool cmdline_write(struct editor *editor) {
    enum filebuf_status status = editor_write_current_filebuf(editor);
    if (status == filebuf_ok) {
        return true;
    }
    editor_print_message(editor, "Unable to write: %s", filebuf_status_describe(status));
    return false;
}

static bool cmdline_set_line_index(struct editor *editor, size_t line) {
    struct filebuf *filebuf = editor_current_filebuf(editor);
    if (filebuf != NULL && line < filebuf->lines.len) {
        editor->line_index = line;
        return true;
    }
    return false;
}

static bool cmdline_edit_line(struct editor *editor) {
    struct strbuf *line = editor_current_line(editor);
    if (line != NULL) {
        editor->editline = line;
        editor->editline_state = editline_state_new();
        editor->frame.leftmost_column = 0;
        editor->mode = editor_mode_editline;
        return true;
    }
    editor_print_message(editor, "Line number %zu is out of range", editor->line_index + 1);
    return false;
}

bool editor_execute_command(struct editor *editor, struct view cmd) {
    view_trim_whitespace(&cmd);
    if (cmd.len == 0) {
        return true;
    }
    if (view_deep_equal(cmd, view_from("q")) || view_deep_equal(cmd, view_from("quit"))) {
        editor->mode = editor_mode_quit;
        return true;
    }
    if (view_deep_equal(cmd, view_from("el")) || view_deep_equal(cmd, view_from("editline"))) {
        return cmdline_edit_line(editor);
    }
    if (view_deep_equal(cmd, view_from("w")) || view_deep_equal(cmd, view_from("write"))) {
        return cmdline_write(editor);
    }
    if (view_deep_equal(cmd, view_from("n")) || view_deep_equal(cmd, view_from("next"))) {
        return editor_set_focus(editor, editor->focus + 1);
    }
    if (view_deep_equal(cmd, view_from("b")) || view_deep_equal(cmd, view_from("back"))) {
        return editor_set_focus(editor, editor->focus - 1);
    }
    if (view_deep_equal(cmd, view_from("j"))) {
        return cmdline_set_line_index(editor, editor->line_index + 1);
    }
    if (view_deep_equal(cmd, view_from("k"))) {
        return cmdline_set_line_index(editor, editor->line_index - 1);
    }
    editor_print_message(editor, "Unrecognized command: %.*s", (int)cmd.len, cmd.ptr);
    return false;
}
