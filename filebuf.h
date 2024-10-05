#ifndef NEX_FILEBUF_H
#define NEX_FILEBUF_H

#include "strbuf.h"
#include "vector.h"
#include <stdio.h>

struct filebuf {
    struct vector lines; // Element: `struct strbuf`
    struct strbuf path;
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

struct filebuf filebuf_new(void) NEX_CONST;

void filebuf_free(struct filebuf *filebuf) NEX_NONNULL;

void filebuf_destroy(void *filebuf) NEX_NONNULL;

enum filebuf_status filebuf_read(struct filebuf *filebuf) NEX_NONNULL;

enum filebuf_status filebuf_write(struct filebuf filebuf);

enum filebuf_status lines_read_stream(struct vector *lines, FILE *stream) NEX_NONNULL;

enum filebuf_status lines_read(struct vector *lines, const char *path) NEX_NONNULL;

enum filebuf_status lines_write_stream(struct vector lines, FILE *stream) NEX_NONNULL;

enum filebuf_status lines_write(struct vector lines, const char *path) NEX_NONNULL;

const char *filebuf_status_describe(enum filebuf_status status) NEX_CONST;

#endif // NEX_FILEBUF_H
