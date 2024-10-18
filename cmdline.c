#include "editor.h"

static bool cmdline_write(struct editor *editor) {
    enum filebuf_status status = editor_write_current_filebuf(editor);
    if (status == filebuf_ok) {
        return true;
    }
    editor_print_message(editor, "Unable to write: %s", filebuf_status_describe(status));
    return false;
}

static bool cmdline_edit_line(struct editor *editor) {
    editor->editline_filebuf = editor_current_filebuf(editor);
    if (editor->editline_filebuf == NULL) {
        editor_print_message(editor, "No file to edit");
        return false;
    }
    editor->editline_state = vi_state_new(vi_context_line);
    editor->mode = editor_mode_editline;
    return true;
}

static bool cmdline_visual(struct editor *editor) {
    if (editor_current_filebuf(editor) != NULL) {
        editor->vi_state = vi_state_new(vi_context_file);
        editor->mode = editor_mode_vi;
        return true;
    }
    editor_print_message(editor, "No file in focus");
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
    if (view_deep_equal(cmd, view_from("vi")) || view_deep_equal(cmd, view_from("visual"))) {
        return cmdline_visual(editor);
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
    editor_print_message(editor, "Unrecognized command: %.*s", (int)cmd.len, cmd.ptr);
    return false;
}
