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

NEX_GNU_ATTRIBUTE(const)
struct strbuf strbuf_new(void);

NEX_GNU_ATTRIBUTE(const)
struct view strbuf_view(struct strbuf strbuf);

void strbuf_destroy(void *strbuf);

void strbuf_free(struct strbuf *strbuf);

void strbuf_clear(struct strbuf *strbuf);

bool strbuf_resize(struct strbuf *strbuf, size_t size);

bool strbuf_reserve(struct strbuf *strbuf, size_t capacity);

bool strbuf_append(struct strbuf *strbuf, struct view view);

bool strbuf_push(struct strbuf *strbuf, char character);

bool strbuf_pop(struct strbuf *strbuf);

bool strbuf_insert(struct strbuf *strbuf, size_t index, char character);

bool strbuf_erase(struct strbuf *strbuf, size_t index);

#endif // NEX_STRBUF_H
