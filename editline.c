#include "editline.h"
#include "keycodes.h"
#include <ctype.h>
#include <limits.h>

static const int step = 1;

static enum editline_status editline_cursor_left(size_t *cursor) {
    if (*cursor != 0) {
        *cursor -= min_uz(*cursor, step);
        return editline_ok;
    }
    return editline_fail;
}

static enum editline_status editline_cursor_right(struct strbuf *line, size_t *cursor) {
    if (*cursor < line->len) {
        *cursor = min_uz(*cursor + step, line->len);
        return editline_ok;
    }
    return editline_fail;
}

static enum editline_status editline_backspace(struct strbuf *line, size_t *cursor) {
    if (strbuf_erase(line, *cursor - 1)) {
        --*cursor;
        return editline_ok;
    }
    return editline_fail;
}

static enum editline_status editline_insert(struct strbuf *line, size_t *cursor, int character) {
    if (strbuf_insert(line, *cursor, character)) {
        ++*cursor;
        return editline_ok;
    }
    return editline_fail;
}

enum editline_status editline_handle_key(struct strbuf *line, size_t *cursor, int key) {
    if (key >= 0 && key <= UCHAR_MAX && isprint(key)) {
        return editline_insert(line, cursor, key);
    }
    switch (key) {
    case NEX_KEY_LEFT:      return editline_cursor_left(cursor);
    case NEX_KEY_RIGHT:     return editline_cursor_right(line, cursor);
    case NEX_KEY_BACKSPACE: return editline_backspace(line, cursor);
    case NEX_KEY_ENTER:     return editline_accept;
    default:                return editline_fail;
    }
}
