#ifndef NEX_FILEBUF_H
#define NEX_FILEBUF_H

#include "buffer.h"
#include "vector.h"
#include <stdio.h>

struct filebuf {
    struct vector lines; // Element: `struct buffer`
    char *path; // Potentially null, owning null terminated string
};

enum filebuf_status {
    filebuf_ok,
    filebuf_no_path,
    filebuf_bad_alloc,
    filebuf_bad_read,
    filebuf_bad_write,
    filebuf_bad_open_read,
    filebuf_bad_open_write,
};

struct filebuf filebuf_new(void);

void filebuf_free(struct filebuf *filebuf);

void filebuf_destroy(void *filebuf);

enum filebuf_status filebuf_read(struct filebuf *filebuf, const char *path);

enum filebuf_status lines_read_stream(struct vector *lines, FILE *stream);

enum filebuf_status lines_read(struct vector *lines, const char *path);

enum filebuf_status lines_write_stream(struct vector lines, FILE *stream);

enum filebuf_status lines_write(struct vector lines, const char *path);

const char *filebuf_status_describe(enum filebuf_status status);

#endif // NEX_FILEBUF_H
