#ifndef NEX_FILEBUF_H
#define NEX_FILEBUF_H

#include "strbuf.h"
#include "vector.h"
#include <stdio.h>

struct position {
    size_t x, y;
};

struct range {
    struct position begin, end;
};

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

bool lines_split_line(struct vector *lines, struct position position) NEX_NONNULL;

bool lines_collect_range(struct vector *lines, struct range range, struct strbuf *output) NEX_NONNULL;

bool lines_erase_range(struct vector *lines, struct range range) NEX_NONNULL;

bool lines_insert(struct vector *lines, struct position position, struct view view) NEX_NONNULL;

bool position_less_than(struct position left, struct position right) NEX_CONST;

struct range range_new(struct position a, struct position b) NEX_CONST;

const char *filebuf_status_describe(enum filebuf_status status) NEX_CONST;

#endif // NEX_FILEBUF_H
