#ifndef NEX_STRBUF_H
#define NEX_STRBUF_H

#include "util.h"
#include "view.h"
#include <stdbool.h>
#include <stddef.h>

// A growable, null terminated, owning string type.
struct strbuf {
    char *ptr;
    size_t len;
    size_t cap;
};

struct strbuf strbuf_new(void) NEX_CONST;

struct view strbuf_view(struct strbuf strbuf) NEX_CONST;

void strbuf_destroy(void *strbuf) NEX_NONNULL;

void strbuf_free(struct strbuf *strbuf) NEX_NONNULL;

void strbuf_clear(struct strbuf *strbuf) NEX_NONNULL;

bool strbuf_resize(struct strbuf *strbuf, size_t size) NEX_NONNULL;

bool strbuf_reserve(struct strbuf *strbuf, size_t capacity) NEX_NONNULL;

bool strbuf_append(struct strbuf *strbuf, struct view view) NEX_NONNULL;

bool strbuf_push(struct strbuf *strbuf, char character) NEX_NONNULL;

bool strbuf_pop(struct strbuf *strbuf) NEX_NONNULL;

bool strbuf_insert(struct strbuf *strbuf, size_t index, char character) NEX_NONNULL;

bool strbuf_erase(struct strbuf *strbuf, size_t index) NEX_NONNULL;

#endif // NEX_STRBUF_H
