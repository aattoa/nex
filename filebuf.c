#include "filebuf.h"
#include "util.h"
#include <assert.h>
#include <stdlib.h>

static enum filebuf_status filebuf_allocate(struct filebuf *filebuf, size_t line_capacity) {
    void *ptr = malloc(line_capacity * sizeof(struct buffer));
    if (ptr == NULL) {
        return filebuf_bad_alloc;
    }
    filebuf->lines = ptr;
    filebuf->line_count = 0;
    filebuf->line_capacity = line_capacity;
    return filebuf_ok;
}

static enum filebuf_status filebuf_grow_lines(struct filebuf *filebuf) {
    if (filebuf->lines == NULL) {
        return filebuf_allocate(filebuf, 64);
    }
    size_t new_capacity = min_uz(filebuf->line_capacity * 2, filebuf->line_capacity + 1028);
    if (new_capacity < filebuf->line_capacity) {
        return filebuf_bad_alloc; // size_t overflow
    }
    void *ptr = realloc(filebuf->lines, new_capacity * sizeof(struct buffer));
    if (ptr == NULL) {
        return filebuf_bad_alloc;
    }
    filebuf->lines = ptr;
    filebuf->line_capacity = new_capacity;
    return filebuf_ok;
}

static enum filebuf_status filebuf_read_line(bool *eof, struct buffer *line, FILE *stream) {
    long position = ftell(stream);
    size_t length = 0;
    for (;;) {
        int character = fgetc(stream);
        if (character == EOF || character == '\n') {
            *eof = (character == EOF);
            break;
        }
        else {
            ++length;
        }
    }
    if (ferror(stream) != 0) {
        return filebuf_bad_read;
    }
    if (fseek(stream, position, SEEK_SET) != 0) {
        return filebuf_bad_read;
    }
    if (!buffer_allocate(line, length)) {
        return filebuf_bad_alloc;
    }
    if (fread(line->ptr, 1, length, stream) != length) {
        buffer_free(line);
        return filebuf_bad_read;
    }
    if (!*eof) {
        (void)fgetc(stream); // Skip the newline
    }
    return filebuf_ok;
}

struct filebuf filebuf_new(void) {
    return (struct filebuf) { .lines = NULL, .line_count = 0, .line_capacity = 0 };
}

void filebuf_free(struct filebuf *filebuf) {
    for (size_t i = 0; i != filebuf->line_count; ++i) {
        buffer_free(&filebuf->lines[i]);
    }
    free(filebuf->lines);
    *filebuf = filebuf_new();
}

enum filebuf_status filebuf_read_stream(struct filebuf *filebuf, FILE *stream) {
    *filebuf = filebuf_new();
    bool eof = false;
    while (!eof) {
        if (filebuf->line_count == filebuf->line_capacity) {
            enum filebuf_status status = filebuf_grow_lines(filebuf);
            if (status != filebuf_ok) {
                return status;
            }
        }
        assert(filebuf->line_count < filebuf->line_capacity);

        struct buffer line;
        enum filebuf_status status = filebuf_read_line(&eof, &line, stream);
        if (status != filebuf_ok) {
            return status;
        }
        filebuf->lines[filebuf->line_count++] = line;
    }
    return filebuf_ok;
}

enum filebuf_status filebuf_read(struct filebuf *filebuf, const char *path) {
    FILE *stream = fopen(path, "r");
    if (stream == NULL) {
        return filebuf_bad_open_read;
    }
    enum filebuf_status status = filebuf_read_stream(filebuf, stream);
    fclose(stream);
    return status;
}

enum filebuf_status filebuf_write_stream(struct filebuf filebuf, FILE *stream) {
    for (size_t i = 0; i != filebuf.line_count; ++i) {
        struct buffer line = filebuf.lines[i];
        if (fwrite(line.ptr, 1, line.len, stream) != line.len) {
            return filebuf_bad_write;
        }
        if (i + 1 != filebuf.line_count && fputc('\n', stream) == EOF) {
            return filebuf_bad_write;
        }
    }
    return filebuf_ok;
}

enum filebuf_status filebuf_write(struct filebuf filebuf, const char *path) {
    FILE *stream = fopen(path, "w");
    if (stream == NULL) {
        return filebuf_bad_open_write;
    }
    enum filebuf_status status = filebuf_write_stream(filebuf, stream);
    fclose(stream);
    return status;
}

const char *filebuf_status_describe(enum filebuf_status status) {
    switch (status) {
    case filebuf_ok:             return "ok";
    case filebuf_bad_alloc:      return "bad alloc";
    case filebuf_bad_read:       return "bad read";
    case filebuf_bad_write:      return "bad write";
    case filebuf_bad_open_read:  return "bad open read";
    case filebuf_bad_open_write: return "bad open write";
    default:                     return "unknown filebuf status";
    }
}
