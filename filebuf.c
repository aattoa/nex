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

struct filebuf filebuf_new(void) {
    return (struct filebuf) {
        .path = buffer_new(),
        .lines = vector_new(sizeof(struct buffer), buffer_destroy),
    };
}

void filebuf_free(struct filebuf *filebuf) {
    buffer_free(&filebuf->path);
    vector_free(&filebuf->lines);
}

void filebuf_destroy(void *filebuf) {
    filebuf_free((struct filebuf*)filebuf);
}

enum filebuf_status filebuf_read(struct filebuf *filebuf, const char *path) {
    *filebuf = filebuf_new();
    enum filebuf_status status = lines_read(&filebuf->lines, path);
    if (status != filebuf_ok) {
        return status;
    }
    if (!buffer_copy(&filebuf->path, view_from(path))) {
        vector_free(&filebuf->lines);
        return filebuf_bad_alloc;
    }
    return filebuf_ok;
}

enum filebuf_status lines_read_stream(struct vector *lines, FILE *stream) {
    bool eof = false;
    while (!eof) {
        struct buffer line = buffer_new();
        enum filebuf_status status = filebuf_read_line(&eof, &line, stream);
        if (status != filebuf_ok) {
            return status;
        }
        if (!vector_push(lines, &line)) {
            buffer_free(&line);
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
        struct buffer *line = vector_at(&lines, i);
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
