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

bool strbuf_push(struct strbuf *strbuf, char character) NEX_NONNULL;

bool strbuf_push_view(struct strbuf *strbuf, struct view view) NEX_NONNULL;

bool strbuf_insert(struct strbuf *strbuf, size_t index, char character) NEX_NONNULL;

bool strbuf_insert_view(struct strbuf *strbuf, size_t index, struct view view) NEX_NONNULL;

bool strbuf_pop(struct strbuf *strbuf) NEX_NONNULL;

bool strbuf_pop_n(struct strbuf *strbuf, size_t n) NEX_NONNULL;

bool strbuf_erase(struct strbuf *strbuf, size_t index) NEX_NONNULL;

bool strbuf_erase_n(struct strbuf *strbuf, size_t index, size_t n) NEX_NONNULL;

bool strbuf_vformat(struct strbuf *strbuf, size_t max, const char* restrict fmt, va_list args) NEX_NONNULL;

bool strbuf_format(struct strbuf *strbuf, size_t max, const char* restrict fmt, ...) NEX_NONNULL NEX_GNU_ATTRIBUTE(format(printf, 3, 4));

#endif // NEX_STRBUF_H
