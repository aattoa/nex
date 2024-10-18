#include "visual.h"
#include "keycodes.h"

enum virtualedit {
    virtualedit_none,
    virtualedit_one,
    virtualedit_full,
};

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

static void clamp_cursor_x(struct filebuf *filebuf, struct vi_state *state, enum virtualedit virtualedit) {
    struct strbuf *line = vi_current_line(filebuf, state);
    if (virtualedit == virtualedit_none) {
        state->cursor.x = min_uz(state->cursor.x, sat_sub_uz(line->len, 1));
    }
    else if (virtualedit == virtualedit_one) {
        state->cursor.x = min_uz(state->cursor.x, line->len);
    }
}

static enum vi_status cursor_left(struct vi_state *state) {
    if (state->cursor.x != 0) {
        state->cursor.x = sat_sub_uz(state->cursor.x, consume_count(state));
        return vi_ok;
    }
    return vi_fail;
}

static enum vi_status cursor_right(struct filebuf *filebuf, struct vi_state *state, enum virtualedit virtualedit) {
    struct strbuf *line = vi_current_line(filebuf, state);
    if (state->cursor.x < line->len) {
        state->cursor.x += consume_count(state);
        clamp_cursor_x(filebuf, state, virtualedit);
        return vi_ok;
    }
    return vi_fail;
}

static enum vi_status cursor_up(struct filebuf *filebuf, struct vi_state *state, enum virtualedit virtualedit) {
    if (state->cursor.y != 0) {
        state->cursor.y -= min_uz(state->cursor.y, consume_count(state));
        clamp_cursor_x(filebuf, state, virtualedit);
        return vi_ok;
    }
    return vi_fail;
}

static enum vi_status cursor_down(struct filebuf *filebuf, struct vi_state *state, enum virtualedit virtualedit) {
    if (state->cursor.y < filebuf->lines.len) {
        state->cursor.y = min_uz(state->cursor.y + consume_count(state), filebuf->lines.len - 1);
        clamp_cursor_x(filebuf, state, virtualedit);
        return vi_ok;
    }
    return vi_fail;
}

static enum vi_status backspace(struct filebuf *filebuf, struct vi_state *state) {
    if (strbuf_erase(vi_current_line(filebuf, state), state->cursor.x - 1)) {
        --state->cursor.x;
        return vi_ok;
    }
    return vi_fail;
}

static enum vi_status erase(struct filebuf *filebuf, struct vi_state *state, struct registers *registers) {
    struct strbuf *line = vi_current_line(filebuf, state);
    if (line->len == 0) {
        return vi_fail;
    }
    size_t count = min_uz(line->len - state->cursor.x, consume_count(state));
    struct strbuf *reg = register_get(registers, consume_regname(state));
    if (reg == NULL) {
        die("null register\n");
    }
    strbuf_clear(reg);
    strbuf_push_view(reg, view_subview(strbuf_view(*line), state->cursor.x, count));
    strbuf_erase_n(line, state->cursor.x, count);
    return vi_ok;
}

static enum vi_status paste(struct filebuf *filebuf, struct vi_state *state, struct registers *registers, struct position position) {
    struct strbuf *reg = register_get(registers, consume_regname(state));
    return lines_insert(&filebuf->lines, position, strbuf_view(*reg)) ? vi_ok : vi_fail;
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
    case 'h': case NEX_KEY_LEFT:
        return cursor_left(state);
    case 'j': case NEX_KEY_DOWN:
        return cursor_down(filebuf, state, virtualedit_none);
    case 'k': case NEX_KEY_UP:
        return cursor_up(filebuf, state, virtualedit_none);
    case 'l': case NEX_KEY_RIGHT:
        return cursor_right(filebuf, state, virtualedit_none);
    case '$':
        state->cursor.x = sat_sub_uz(vi_current_line(filebuf, state)->len, 1);
        return vi_ok;
    case 'i':
        return start_insert(state);
    case 'I':
        state->cursor.x = 0;
        return start_insert(state);
    case 'a':
        ++state->cursor.x;
        return start_insert(state);
    case 'A':
        state->cursor.x = vi_current_line(filebuf, state)->len;
        return start_insert(state);
    case 'o':
        cursor_down(filebuf, state, virtualedit_one); // fallthrough
    case 'O':
        insert_line(filebuf, state->cursor.y);
        state->cursor.x = 0;
        return start_insert(state);
    case 'v':
        state->select_start = state->cursor;
        state->mode = vi_mode_select;
        return vi_ok;
    case 'p':
        return paste(filebuf, state, registers, (struct position) { .x = state->cursor.x + 1, .y = state->cursor.y });
    case 'P':
        return paste(filebuf, state, registers, state->cursor);
    case 'x':
        return erase(filebuf, state, registers);
    case '"':
        state->mode = vi_mode_register_pending;
        return vi_ok;
    case NEX_KEY_ESCAPE:
        state->count = 0;
        return vi_ok;
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
    if (strbuf_insert(vi_current_line(filebuf, state), state->cursor.x, key)) {
        ++state->cursor.x;
        return vi_ok;
    }
    return vi_fail;
}

static enum vi_status enter(struct filebuf *filebuf, struct vi_state *state) {
    if (state->context == vi_context_line) {
        return vi_line_accept;
    }
    if (!lines_split_line(&filebuf->lines, state->cursor)) {
        return vi_fail;
    }
    ++state->cursor.y;
    state->cursor.x = 0;
    return vi_ok;
}

static enum vi_status handle_key_insert(struct filebuf *filebuf, struct vi_state *state, int key) {
    if (is_print(key)) {
        return insert(filebuf, state, key);
    }
    switch (key) {
    case NEX_KEY_LEFT:      return cursor_left(state);
    case NEX_KEY_DOWN:      return cursor_down(filebuf, state, virtualedit_one);
    case NEX_KEY_UP:        return cursor_up(filebuf, state, virtualedit_one);
    case NEX_KEY_RIGHT:     return cursor_right(filebuf, state, virtualedit_one);
    case NEX_KEY_ESCAPE:    cursor_left(state); state->mode = vi_mode_normal; return vi_ok;
    case NEX_KEY_BACKSPACE: return backspace(filebuf, state);
    case NEX_KEY_ENTER:     return enter(filebuf, state);
    }
    return vi_fail;
}

static enum vi_status yank_range(struct filebuf *filebuf, struct vi_state *state, struct registers *registers) {
    struct strbuf *reg = register_get(registers, consume_regname(state));
    struct range range = range_new(state->select_start, state->cursor);
    state->mode = vi_mode_normal;
    strbuf_clear(reg);
    return lines_collect_range(&filebuf->lines, range, reg) ? vi_ok : vi_fail;
}

static enum vi_status erase_range(struct filebuf *filebuf, struct vi_state *state, struct registers *registers) {
    struct strbuf *reg = register_get(registers, consume_regname(state));
    struct range range = range_new(state->select_start, state->cursor);
    state->cursor = range.begin;
    state->mode = vi_mode_normal;
    strbuf_clear(reg);
    return lines_collect_range(&filebuf->lines, range, reg) && lines_erase_range(&filebuf->lines, range) ? vi_ok : vi_fail;
}

static enum vi_status swap_select_cursor(struct vi_state *state) {
    struct position tmp = state->cursor;
    state->cursor = state->select_start;
    state->select_start = tmp;
    return vi_ok;
}

static enum vi_status handle_key_select(struct filebuf *filebuf, struct vi_state *state, struct registers *registers, int key) {
    switch (key) {
    case NEX_KEY_ESCAPE:
        state->mode = vi_mode_normal;
        return vi_ok;
    case 'y':
        return yank_range(filebuf, state, registers);
    case 'd': case 'x':
        return erase_range(filebuf, state, registers);
    case 'o':
        return swap_select_cursor(state);
    }
    return handle_key_normal(filebuf, state, registers, key);
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
    case vi_mode_select: return handle_key_select(filebuf, state, registers, key);
    case vi_mode_cmdline: return handle_key_cmdline(filebuf, state, registers, key);
    case vi_mode_register_pending: return handle_key_register(state, registers, key);
    default: die("unhandled mode\n");
    }
}

struct strbuf *vi_current_line(struct filebuf *filebuf, struct vi_state *state) {
    struct strbuf *line = vector_at(&filebuf->lines, state->cursor.y);
    if (line == NULL) {
        die("cursor out of range: lines:%zu, cursor.y:%zu\n", filebuf->lines.len, state->cursor.y);
    }
    return line;
}

struct vi_state vi_state_new(enum vi_context context) {
    return (struct vi_state) {
        .select_start = (struct position) { .x = 0, .y = 0 },
        .cursor = (struct position) { .x = 0, .y = 0 },
        .frame = (struct vi_frame) { .top = 0, .left = 0 },
        .mode = context == vi_context_file ? vi_mode_normal : vi_mode_insert,
        .context = context,
        .count = 0,
        .regname = 0,
    };
}

const char *vi_mode_describe(enum vi_mode mode) {
    switch (mode) {
    case vi_mode_normal:           return "Normal";
    case vi_mode_insert:           return "Insert";
    case vi_mode_select:           return "Select";
    case vi_mode_cmdline:          return "Command line";
    case vi_mode_register_pending: return "Register pending";
    default:                       return "Unknown";
    }
}
