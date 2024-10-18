#include "editline.h"
#include "keycodes.h"

static size_t consume_count(struct editline_state *state) {
    size_t count = state->count;
    state->count = 0;
    return count != 0 ? count : 1;
}

static enum editline_status cursor_left(struct editline_state *state) {
    if (state->cursor != 0) {
        state->cursor -= min_uz(state->cursor, consume_count(state));
        return editline_ok;
    }
    return editline_fail;
}

static enum editline_status cursor_right(struct strbuf *line, struct editline_state *state) {
    if (state->cursor < line->len) {
        state->cursor = min_uz(state->cursor + consume_count(state), line->len);
        return editline_ok;
    }
    return editline_fail;
}

static enum editline_status backspace(struct strbuf *line, struct editline_state *state) {
    if (strbuf_erase(line, state->cursor - 1)) {
        --state->cursor;
        return editline_ok;
    }
    return editline_fail;
}

static enum editline_status erase(struct strbuf *line, struct editline_state *state) {
    return strbuf_erase(line, state->cursor) ? editline_ok : editline_fail;
}

static enum editline_status insert(struct strbuf *line, struct editline_state *state, int character) {
    if (strbuf_insert(line, state->cursor, character)) {
        ++state->cursor;
        return editline_ok;
    }
    return editline_fail;
}

static enum editline_status enter_mode(struct editline_state *state, enum editline_mode mode) {
    state->mode = mode;
    state->count = 0;
    return editline_ok;
}

static enum editline_status start_insert(struct strbuf *line, struct editline_state *state, int key) {
    if (key == 'i') {}
    if (key == 'a') cursor_right(line, state);
    if (key == 'I') state->cursor = 0;
    if (key == 'A') state->cursor = line->len;
    return enter_mode(state, editline_mode_insert);
}

static enum editline_status add_count(size_t *count, char digit) {
    *count *= 10;
    *count += digit - 48;
    return editline_ok;
}

static enum editline_status set_line(struct strbuf *line, struct view new_line, struct editline_state *state) {
    strbuf_clear(line);
    if (strbuf_push_view(line, new_line)) {
        state->cursor = line->len;
        return editline_ok;
    }
    else {
        state->cursor = 0;
        return editline_fail;
    }
}

static enum editline_status set_history_line(struct strbuf *line, struct editline_state *state, struct filebuf *history) {
    struct strbuf *history_line = vector_at(&history->lines, state->history_index - 1);
    if (history_line == NULL) {
        die("bad history index\n");
    }
    return set_line(line, strbuf_view(*history_line), state);
}

static enum editline_status history_previous(struct strbuf *line, struct editline_state *state, struct filebuf *history) {
    if (history == NULL || state->history_index < 2) {
        return editline_fail;
    }
    --state->history_index;
    return set_history_line(line, state, history);
}

static enum editline_status history_next(struct strbuf *line, struct editline_state *state, struct filebuf *history) {
    if (history == NULL || state->history_index == 0) {
        return editline_fail;
    }
    if (state->history_index == history->lines.len) {
        return editline_fail;
    }
    ++state->history_index;
    return set_history_line(line, state, history);
}

enum editline_status editline_handle_key(struct strbuf *line, struct editline_state *state, struct filebuf *history, int key) {
    if (state->mode == editline_mode_insert) {
        if (is_print(key)) {
            return insert(line, state, key);
        }
        switch (key) {
        case NEX_KEY_ESCAPE:    cursor_left(state); return enter_mode(state, editline_mode_normal);
        case NEX_KEY_LEFT:      return cursor_left(state);
        case NEX_KEY_RIGHT:     return cursor_right(line, state);
        case NEX_KEY_UP:        return history_previous(line, state, history);
        case NEX_KEY_DOWN:      return history_next(line, state, history);
        case NEX_KEY_BACKSPACE: return backspace(line, state);
        case NEX_KEY_ENTER:     return editline_accept;
        }
        return editline_fail;
    }
    else {
        assert(state->mode == editline_mode_normal);
        switch (key) {
        case NEX_KEY_ENTER:                     return editline_accept;
        case 'x': case NEX_KEY_BACKSPACE:       return erase(line, state);
        case 'h': case NEX_KEY_LEFT:            return cursor_left(state);
        case 'l': case NEX_KEY_RIGHT:           return cursor_right(line, state);
        case 'k': case NEX_KEY_UP:              return history_previous(line, state, history);
        case 'j': case NEX_KEY_DOWN:            return history_next(line, state, history);
        case 'i': case 'I': case 'a': case 'A': return start_insert(line, state, key);
        case '$':                               state->cursor = line->len; return editline_ok;
        case NEX_KEY_ESCAPE:                    state->count = 0; return editline_ok;
        }
        if (is_digit(key)) {
            if (key == '0' && state->count == 0) {
                state->cursor = 0;
                return editline_ok;
            }
            return add_count(&state->count, key);
        }
        return editline_fail;
    }
}

struct editline_state editline_state_new(void) {
    return (struct editline_state) {
        .history_index = 0,
        .cursor = 0,
        .count = 0,
        .leftmost_column = 0,
        .mode = editline_mode_insert,
    };
}
