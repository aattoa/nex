#include "filebuf.h"
#include "util.h"
#include <stdlib.h>

static enum filebuf_status filebuf_read_line(bool *eof, struct strbuf *line, FILE *stream) {
    long position = ftell(stream);
    size_t length = 0;
    for (;;) {
        int character = fgetc(stream);
        if (character == EOF || character == '\n') {
            *eof = (character == EOF);
            break;
        }
        ++length;
    }
    if (ferror(stream) != 0) {
        return filebuf_bad_read;
    }
    if (fseek(stream, position, SEEK_SET) != 0) {
        return filebuf_bad_read;
    }
    if (!strbuf_resize(line, length)) {
        return filebuf_bad_alloc;
    }
    if (fread(line->ptr, 1, length, stream) != length) {
        strbuf_free(line);
        return filebuf_bad_read;
    }
    if (!*eof) {
        (void)fgetc(stream); // Skip the newline
    }
    return filebuf_ok;
}

struct filebuf filebuf_new(void) {
    return (struct filebuf) {
        .lines = vector_new(sizeof(struct strbuf), strbuf_destroy),
        .path = strbuf_new(),
    };
}

void filebuf_free(struct filebuf *filebuf) {
    vector_free(&filebuf->lines);
    strbuf_free(&filebuf->path);
}

void filebuf_destroy(void *filebuf) {
    filebuf_free((struct filebuf*)filebuf);
}

enum filebuf_status filebuf_read(struct filebuf *filebuf) {
    return filebuf->path.len == 0 ? filebuf_no_path : lines_read(&filebuf->lines, filebuf->path.ptr);
}

enum filebuf_status filebuf_write(struct filebuf filebuf) {
    return filebuf.path.len == 0 ? filebuf_no_path : lines_write(filebuf.lines, filebuf.path.ptr);
}

enum filebuf_status lines_read_stream(struct vector *lines, FILE *stream) {
    bool eof = false;
    while (!eof) {
        struct strbuf line = strbuf_new();
        enum filebuf_status status = filebuf_read_line(&eof, &line, stream);
        if (status != filebuf_ok) {
            return status;
        }
        if (!vector_push(lines, &line)) {
            strbuf_free(&line);
            return filebuf_bad_alloc;
        }
    }
    return filebuf_ok;
}

enum filebuf_status lines_read(struct vector *lines, const char *path) {
    FILE *stream = fopen(path, "r");
    if (stream == NULL) {
        return filebuf_bad_open_read;
    }
    enum filebuf_status status = lines_read_stream(lines, stream);
    fclose(stream);
    return status;
}

enum filebuf_status lines_write_stream(struct vector lines, FILE *stream) {
    for (size_t i = 0; i != lines.len; ++i) {
        struct strbuf *line = vector_at(&lines, i);
        if (fwrite(line->ptr, 1, line->len, stream) != line->len) {
            return filebuf_bad_write;
        }
        if (i + 1 != lines.len && fputc('\n', stream) == EOF) {
            return filebuf_bad_write;
        }
    }
    return filebuf_ok;
}

enum filebuf_status lines_write(struct vector lines, const char *path) {
    FILE *stream = fopen(path, "w");
    if (stream == NULL) {
        return filebuf_bad_open_write;
    }
    enum filebuf_status status = lines_write_stream(lines, stream);
    fclose(stream);
    return status;
}

bool lines_split_line(struct vector *lines, struct position position) {
    struct strbuf *line = vector_at(lines, position.y);
    if (line == NULL) {
        return false;
    }
    struct strbuf new_line = strbuf_new();
    if (!strbuf_push_view(&new_line, view_subview(strbuf_view(*line), position.x, SIZE_MAX))) {
        return false;
    }
    strbuf_resize(line, position.x);
    if (!vector_insert(lines, position.y + 1, &new_line)) {
        strbuf_free(&new_line);
        return false;
    }
    return true;
}

bool lines_collect_range(struct vector *lines, struct range range, struct strbuf *output) {
    if (range.begin.y == range.end.y) {
        struct strbuf *line = vector_at(lines, range.begin.y);
        return line != NULL
            && range.begin.x <= range.end.x
            && strbuf_push_view(output, view_subview(strbuf_view(*line), range.begin.x, range.end.x - range.begin.x + 1));
    }
    struct strbuf *first_line = vector_at(lines, range.begin.y);
    if (first_line == NULL || range.begin.y > range.end.y) {
        return false;
    }
    strbuf_push_view(output, view_subview(strbuf_view(*first_line), range.begin.x, sat_sub_uz(first_line->len, range.begin.x)));
    for (size_t i = range.begin.y + 1; i < range.end.y; ++i) {
        struct strbuf *line = vector_at(lines, i);
        if (line == NULL || !strbuf_push(output, '\n') || !strbuf_push_view(output, strbuf_view(*line))) {
            return false;
        }
    }
    struct strbuf *last_line = vector_at(lines, range.end.y);
    return last_line != NULL
        && strbuf_push(output, '\n')
        && strbuf_push_view(output, view_subview(strbuf_view(*last_line), 0, range.end.x + 1));
}

bool lines_erase_range(struct vector *lines, struct range range) {
    if (range.begin.y == range.end.y) {
        struct strbuf *line = vector_at(lines, range.begin.y);
        return line != NULL
            && range.begin.x <= range.end.x
            && strbuf_erase_n(line, range.begin.x, range.end.x - range.begin.x + 1);
    }
    struct strbuf *first_line = vector_at(lines, range.begin.y);
    struct strbuf *last_line = vector_at(lines, range.end.y);
    return first_line != NULL
        && last_line != NULL
        && range.begin.y < range.end.y
        && range.begin.x <= first_line->len
        && range.end.x <= last_line->len
        && strbuf_pop_n(first_line, first_line->len - range.begin.x)
        && strbuf_push_view(first_line, view_subview(strbuf_view(*last_line), range.end.x + 1, SIZE_MAX))
        && vector_erase_n(lines, range.begin.y + 1, range.end.y - range.begin.y);
}

bool lines_insert(struct vector *lines, struct position position, struct view view) {
    struct strbuf *first_line = vector_at(lines, position.y);
    if (first_line == NULL) {
        return false;
    }
    struct view left, right;
    if (!view_split_char(view, &left, &right, '\n')) {
        return strbuf_insert_view(first_line, position.x, view);
    }
    if (!lines_split_line(lines, position)) {
        return false;
    }
    if (!strbuf_push_view(vector_at(lines, position.y), left)) {
        return false;
    }
    while (view_split_char(right, &left, &right, '\n')) {
        struct strbuf new_line = strbuf_new();
        if (!strbuf_push_view(&new_line, left)) {
            return false;
        }
        if (!vector_insert(lines, ++position.y, &new_line)) {
            strbuf_free(&new_line);
            return false;
        }
    }
    return strbuf_insert_view(vector_at(lines, position.y + 1), 0, right);
}

bool position_less_than(struct position left, struct position right) {
    return (left.y != right.y) ? (left.y < right.y) : (left.x < right.x);
}

struct range range_new(struct position a, struct position b) {
    return position_less_than(a, b) ? (struct range) { a, b } : (struct range) { b, a };
}

const char *filebuf_status_describe(enum filebuf_status status) {
    switch (status) {
    case filebuf_ok:             return "ok";
    case filebuf_no_path:        return "no path";
    case filebuf_bad_alloc:      return "bad alloc";
    case filebuf_bad_read:       return "bad read";
    case filebuf_bad_write:      return "bad write";
    case filebuf_bad_open_read:  return "bad open read";
    case filebuf_bad_open_write: return "bad open write";
    default:                     return "unknown filebuf status";
    }
}
