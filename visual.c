#include "visual.h"
#include "keycodes.h"

static size_t consume_count(struct vi_state *state) {
    size_t count = state->count;
    state->count = 0;
    return count != 0 ? count : 1;
}

static char consume_regname(struct vi_state *state) {
    char name = state->regname;
    state->regname = 0;
    return name != 0 ? name : REGISTER_UNNAMED;
}

static struct strbuf *current_line(struct filebuf *filebuf, struct vi_state *state) {
    struct strbuf *line = vector_at(&filebuf->lines, state->cursor.y);
    if (line == NULL) {
        die("cursor.y out of range");
    }
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

static enum vi_status erase(struct filebuf *filebuf, struct vi_state *state, struct registers *registers) {
    struct strbuf *line = current_line(filebuf, state);
    if (line->len == 0) {
        return vi_fail;
    }
    size_t count = min_uz(line->len - state->cursor.x, consume_count(state));
    struct strbuf *reg = register_get(registers, consume_regname(state));
    if (reg == NULL) {
        die("null register");
    }
    strbuf_clear(reg);
    strbuf_push_view(reg, view_subview(strbuf_view(*line), state->cursor.x, count));
    strbuf_erase_n(line, state->cursor.x, count);
    return vi_ok;
}

static enum vi_status paste(struct filebuf *filebuf, struct vi_state *state, struct registers *registers, int offset) {
    struct strbuf *reg = register_get(registers, consume_regname(state));
    strbuf_insert_view(current_line(filebuf, state), state->cursor.x + offset, strbuf_view(*reg));
    return vi_ok;
}

static enum vi_status add_count(size_t *count, char digit) {
    *count *= 10;
    *count += digit - 48;
    return vi_ok;
}

static enum vi_status start_insert(struct vi_state *state) {
    state->mode = vi_mode_insert;
    return vi_ok;
}

static void insert_line(struct filebuf *filebuf, size_t index) {
    struct strbuf line = strbuf_new();
    vector_insert(&filebuf->lines, index, &line);
}

static enum vi_status handle_key_normal(struct filebuf *filebuf, struct vi_state *state, struct registers *registers, int key) {
    switch (key) {
    case 'q':
        return vi_leave;
    case NEX_KEY_LEFT: case 'h':
        return cursor_left(state);
    case NEX_KEY_DOWN: case 'j':
        return cursor_down(filebuf, state);
    case NEX_KEY_UP: case 'k':
        return cursor_up(filebuf, state);
    case NEX_KEY_RIGHT: case 'l':
        return cursor_right(filebuf, state);
    case NEX_KEY_ESCAPE:
        state->count = 0;
        return vi_ok;
    case '$':
        state->cursor.x = current_line(filebuf, state)->len;
        return vi_ok;
    case 'i':
        return start_insert(state);
    case 'I':
        state->cursor.x = 0;
        return start_insert(state);
    case 'a':
        cursor_right(filebuf, state);
        return start_insert(state);
    case 'A':
        state->cursor.x = current_line(filebuf, state)->len;
        return start_insert(state);
    case 'o':
        cursor_down(filebuf, state); // fallthrough
    case 'O':
        insert_line(filebuf, state->cursor.y);
        state->cursor.x = 0;
        return start_insert(state);
    case '"':
        state->mode = vi_mode_register_pending;
        return vi_ok;
    case 'p':
        return paste(filebuf, state, registers, 1);
    case 'P':
        return paste(filebuf, state, registers, 0);
    case 'x':
        return erase(filebuf, state, registers);
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
    if (strbuf_insert(current_line(filebuf, state), state->cursor.x, key)) {
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

static enum vi_status handle_key_cmdline(struct filebuf *filebuf, struct vi_state *state, struct registers *registers, int key) {
    (void)filebuf;
    (void)state;
    (void)registers;
    (void)key;
    return vi_fail;
}

static enum vi_status handle_key_register(struct vi_state *state, struct registers *registers, int key) {
    state->mode = vi_mode_normal;
    if (key == NEX_KEY_ESCAPE) {
        return vi_ok;
    }
    if (register_get(registers, key) != NULL) {
        state->regname = key;
        return vi_ok;
    }
    return vi_fail;
}

enum vi_status vi_handle_key(struct filebuf *filebuf, struct vi_state *state, struct registers *registers, int key) {
    switch (state->mode) {
    case vi_mode_normal: return handle_key_normal(filebuf, state, registers, key);
    case vi_mode_insert: return handle_key_insert(filebuf, state, key);
    case vi_mode_cmdline: return handle_key_cmdline(filebuf, state, registers, key);
    case vi_mode_register_pending: return handle_key_register(state, registers, key);
    default: die("unhandled mode");
    }
}

struct vi_state vi_state_new(void) {
    return (struct vi_state) {
        .cursor = (struct vi_cursor) { .x = 0, .y = 0 },
        .frame = (struct vi_frame) { .top = 0, .left = 0 },
        .mode = vi_mode_normal,
        .count = 0,
        .regname = 0,
    };
}
