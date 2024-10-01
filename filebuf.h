#ifndef NEX_FILEBUF_H
#define NEX_FILEBUF_H

#include "buffer.h"
#include "vector.h"
#include <stdio.h>

struct filebuf {
    struct vector lines; // element: `struct buffer`
};

enum filebuf_status {
    filebuf_ok,
    filebuf_bad_alloc,
    filebuf_bad_read,
    filebuf_bad_write,
    filebuf_bad_open_read,
    filebuf_bad_open_write,
};

struct filebuf filebuf_new(void);

void filebuf_free(struct filebuf *filebuf);

enum filebuf_status filebuf_read_stream(struct filebuf *filebuf, FILE *stream);

enum filebuf_status filebuf_read(struct filebuf *filebuf, const char *path);

enum filebuf_status filebuf_write_stream(struct filebuf filebuf, FILE *stream);

enum filebuf_status filebuf_write(struct filebuf filebuf, const char *path);

const char *filebuf_status_describe(enum filebuf_status status);

#endif // NEX_FILEBUF_H
