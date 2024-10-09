#include "visual.h"
#include "keycodes.h"

static size_t consume_count(struct vi_state *state) {
    size_t count = state->count;
    state->count = 0;
    return count != 0 ? count : 1;
}

static struct strbuf *current_line(struct filebuf *filebuf, struct vi_state *state) {
    struct strbuf *line = vector_at(&filebuf->lines, state->cursor.y);
    assert(line != NULL);
    return line;
}

static enum vi_status cursor_left(struct vi_state *state) {
    if (state->cursor.x != 0) {
        state->cursor.x -= min_uz(state->cursor.x, consume_count(state));
        return vi_ok;
    }
    return vi_fail;
}

static enum vi_status cursor_right(struct filebuf *filebuf, struct vi_state *state) {
    struct strbuf *line = current_line(filebuf, state);
    if (state->cursor.x < line->len) {
        state->cursor.x = min_uz(state->cursor.x + consume_count(state), line->len);
        return vi_ok;
    }
    return vi_fail;
}

static enum vi_status cursor_up(struct filebuf *filebuf, struct vi_state *state) {
    if (state->cursor.y != 0) {
        state->cursor.y -= min_uz(state->cursor.y, consume_count(state));
        state->cursor.x = min_uz(state->cursor.x, current_line(filebuf, state)->len);
        return vi_ok;
    }
    return vi_fail;
}

static enum vi_status cursor_down(struct filebuf *filebuf, struct vi_state *state) {
    if (state->cursor.y < filebuf->lines.len) {
        state->cursor.y = min_uz(state->cursor.y + consume_count(state), filebuf->lines.len - 1);
        state->cursor.x = min_uz(state->cursor.x, current_line(filebuf, state)->len);
        return vi_ok;
    }
    return vi_fail;
}

static enum vi_status backspace(struct filebuf *filebuf, struct vi_state *state) {
    if (strbuf_erase(current_line(filebuf, state), state->cursor.x - 1)) {
        --state->cursor.x;
        return vi_ok;
    }
    return vi_fail;
}

static enum vi_status add_count(size_t *count, char digit) {
    *count *= 10;
    *count += digit - 48;
    return vi_ok;
}

static enum vi_status start_insert(struct filebuf *filebuf, struct vi_state *state, int key) {
    if (key == 'i') {} // do nothing
    if (key == 'I') state->cursor.x = 0;
    if (key == 'a') cursor_right(filebuf, state);
    if (key == 'A') state->cursor.x = current_line(filebuf, state)->len;
    if (key == 'o') {
        struct strbuf line = strbuf_new();
        vector_insert(&filebuf->lines, state->cursor.y == 0 ? 0 : state->cursor.y + 1, &line);
        cursor_down(filebuf, state);
    }
    if (key == 'O') {
        struct strbuf line = strbuf_new();
        vector_insert(&filebuf->lines, state->cursor.y, &line);
        state->cursor.x = 0;
    }
    state->mode = vi_mode_insert;
    return vi_ok;
}

static enum vi_status handle_key_normal(struct filebuf *filebuf, struct vi_state *state, int key) {
    switch (key) {
    case NEX_KEY_LEFT:  case 'h': return cursor_left(state);
    case NEX_KEY_DOWN:  case 'j': return cursor_down(filebuf, state);
    case NEX_KEY_UP:    case 'k': return cursor_up(filebuf, state);
    case NEX_KEY_RIGHT: case 'l': return cursor_right(filebuf, state);
    case NEX_KEY_ESCAPE:          state->count = 0; return vi_ok;
    case '$':                     state->cursor.x = current_line(filebuf, state)->len; return vi_ok;
    case 'q':                     return vi_leave;
    case 'i': case 'I':
    case 'a': case 'A':
    case 'o': case 'O': return start_insert(filebuf, state, key);
    }
    if (is_digit(key)) {
        if (key == '0' && state->count == 0) {
            state->cursor.x = 0;
            return vi_ok;
        }
        return add_count(&state->count, key);
    }
    return vi_fail;
}

static enum vi_status insert(struct filebuf *filebuf, struct vi_state *state, int key) {
    struct strbuf *line = current_line(filebuf, state);
    assert(line != NULL);
    if (strbuf_insert(line, state->cursor.x, key)) {
        ++state->cursor.x;
        return vi_ok;
    }
    return vi_fail;
}

static enum vi_status handle_key_insert(struct filebuf *filebuf, struct vi_state *state, int key) {
    if (is_print(key)) {
        return insert(filebuf, state, key);
    }
    switch (key) {
    case NEX_KEY_ESCAPE:    cursor_left(state); state->mode = vi_mode_normal; return vi_ok;
    case NEX_KEY_BACKSPACE: return backspace(filebuf, state);
    }
    return vi_fail;
}

static enum vi_status handle_key_cmdline(struct filebuf *filebuf, struct vi_state *state, int key) {
    (void)filebuf;
    (void)state;
    (void)key;
    return vi_fail;
}

enum vi_status vi_handle_key(struct filebuf *filebuf, struct vi_state *state, int key) {
    switch (state->mode) {
    case vi_mode_normal: return handle_key_normal(filebuf, state, key);
    case vi_mode_insert: return handle_key_insert(filebuf, state, key);
    case vi_mode_cmdline: return handle_key_cmdline(filebuf, state, key);
    default: die("unhandled mode");
    }
}

struct vi_state vi_state_new(void) {
    return (struct vi_state) {
        .cursor = (struct vi_cursor) { .x = 0, .y = 0 },
        .frame = (struct vi_frame) { .top = 0, .left = 0 },
        .mode = vi_mode_normal,
        .count = 0,
    };
}
