#include "util.h"
#include "editor.h"
#include "keycodes.h"
#include <ctype.h>
#include <limits.h>

static bool cmdline_insert_printable(struct editor *editor, char character) {
    if (!strbuf_insert(&editor->cmdline, editor->cmdline_cursor, character)) {
        return false;
    }
    ++editor->cmdline_cursor;
    return true;
}

static bool cmdline_backspace(struct editor *editor) {
    if (!strbuf_erase(&editor->cmdline, editor->cmdline_cursor - 1)) {
        return false;
    }
    --editor->cmdline_cursor;
    return true;
}

static bool cmdline_cursor_left(struct editor *editor) {
    if (editor->cmdline_cursor == 0) {
        return false;
    }
    --editor->cmdline_cursor;
    return true;
}

static bool cmdline_cursor_right(struct editor *editor) {
    if (editor->cmdline_cursor == editor->cmdline.len) {
        return false;
    }
    ++editor->cmdline_cursor;
    return true;
}

bool editor_cmdline_handle_key(struct editor *editor, int key) {
    if (key == NEX_KEY_LEFT) {
        return cmdline_cursor_left(editor);
    }
    if (key == NEX_KEY_RIGHT) {
        return cmdline_cursor_right(editor);
    }
    if (key == NEX_KEY_BACKSPACE) {
        return cmdline_backspace(editor);
    }
    if (key >= 0 && key <= UCHAR_MAX && isprint(key)) {
        return cmdline_insert_printable(editor, key);
    }
    return false;
}
