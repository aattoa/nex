#include "filebuf.h"
#include "util.h"
#include <stdlib.h>

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

static void destroy_buffer(void *ptr) {
    buffer_free((struct buffer*)ptr);
}

struct filebuf filebuf_new(void) {
    return (struct filebuf) { .lines = vector_new(sizeof(struct buffer), destroy_buffer) };
}

void filebuf_free(struct filebuf *filebuf) {
    vector_free(&filebuf->lines);
    *filebuf = filebuf_new();
}

enum filebuf_status filebuf_read_stream(struct filebuf *filebuf, FILE *stream) {
    *filebuf = filebuf_new();
    bool eof = false;
    while (!eof) {
        struct buffer line;
        enum filebuf_status status = filebuf_read_line(&eof, &line, stream);
        if (status != filebuf_ok) {
            return status;
        }
        if (!vector_push(&filebuf->lines, &line)) {
            return filebuf_bad_alloc;
        }
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
    for (size_t i = 0; i != filebuf.lines.len; ++i) {
        struct buffer *line = vector_at(&filebuf.lines, i);
        if (fwrite(line->ptr, 1, line->len, stream) != line->len) {
            return filebuf_bad_write;
        }
        if (i + 1 != filebuf.lines.len && fputc('\n', stream) == EOF) {
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
