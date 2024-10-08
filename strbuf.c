#include "strbuf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct strbuf strbuf_new(void) {
    return (struct strbuf) { .ptr = NULL, .len = 0, .cap = 0 };
}

struct view strbuf_view(struct strbuf strbuf) {
    return (struct view) { .ptr = strbuf.ptr, .len = strbuf.len };
}

void strbuf_destroy(void *strbuf) {
    strbuf_free((struct strbuf*)strbuf);
}

void strbuf_free(struct strbuf *strbuf) {
    free(strbuf->ptr);
    *strbuf = strbuf_new();
}

void strbuf_clear(struct strbuf *strbuf) {
    if (strbuf->ptr != NULL) {
        strbuf->len = 0;
        *strbuf->ptr = 0;
    }
}

NEX_GNU_ATTRIBUTE(const)
static size_t strbuf_grow_capacity(size_t capacity) {
    return capacity == 0 ? 16 : min_uz(capacity * 2, capacity + 1028);
}

static bool strbuf_grow_if_at_capacity(struct strbuf *strbuf) {
    if (strbuf->len != strbuf->cap) {
        return true;
    }
    size_t new_cap = strbuf_grow_capacity(strbuf->cap);
    return new_cap > strbuf->cap && strbuf_reserve(strbuf, new_cap);
}

bool strbuf_resize(struct strbuf *strbuf, size_t size) {
    if (size < strbuf->len) {
        strbuf->ptr[size] = 0;
        strbuf->len = size;
    }
    else if (size > strbuf->len) {
        if (!strbuf_reserve(strbuf, size)) {
            return false;
        }
        memset(strbuf->ptr + strbuf->len, 0, size - strbuf->len + 1);
        strbuf->len = size;
    }
    return true;
}

bool strbuf_reserve(struct strbuf *strbuf, size_t capacity) {
    if (strbuf->cap < capacity) {
        void *ptr = realloc(strbuf->ptr, capacity + 1);
        if (ptr == NULL) {
            return false;
        }
        strbuf->ptr = ptr;
        strbuf->cap = capacity;
    }
    return true;
}

bool strbuf_append(struct strbuf *strbuf, struct view view) {
    if (!strbuf_reserve(strbuf, strbuf->len + view.len)) {
        return false;
    }
    memcpy(strbuf->ptr + strbuf->len, view.ptr, view.len);
    strbuf->len += view.len;
    strbuf->ptr[strbuf->len] = 0;
    return true;
}

bool strbuf_push(struct strbuf *strbuf, char character) {
    if (!strbuf_grow_if_at_capacity(strbuf)) {
        return false;
    }
    strbuf->ptr[strbuf->len++] = character;
    strbuf->ptr[strbuf->len] = 0;
    return true;
}

bool strbuf_pop(struct strbuf *strbuf) {
    if (strbuf->len == 0) {
        return false;
    }
    strbuf->ptr[--strbuf->len] = 0;
    return true;
}

bool strbuf_insert(struct strbuf *strbuf, size_t index, char character) {
    if (index == strbuf->len) {
        return strbuf_push(strbuf, character);
    }
    if (!strbuf_grow_if_at_capacity(strbuf)) {
        return false;
    }
    memmove(
        strbuf->ptr + index + 1,
        strbuf->ptr + index,
        strbuf->len - index + 1);
    strbuf->ptr[index] = character;
    ++strbuf->len;
    return true;
}

bool strbuf_erase(struct strbuf *strbuf, size_t index) {
    if (index >= strbuf->len) {
        return false;
    }
    if (index == strbuf->len - 1) {
        return strbuf_pop(strbuf);
    }
    memmove(
        strbuf->ptr + index,
        strbuf->ptr + index + 1,
        strbuf->len - index);
    --strbuf->len;
    return true;
}

bool strbuf_vformat(struct strbuf *strbuf, size_t max, const char* restrict fmt, va_list args) {
    if (max == 0) {
        return false;
    }
    if (!strbuf_reserve(strbuf, strbuf->len + max)) {
        return false;
    }
    int result = vsnprintf(strbuf->ptr + strbuf->len, max, fmt, args);
    if (result < 0) {
        strbuf->ptr[strbuf->len] = 0;
        return false;
    }
    strbuf->len += strlen(strbuf->ptr + strbuf->len);
    return true;
}

bool strbuf_format(struct strbuf *strbuf, size_t max, const char* restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bool result = strbuf_vformat(strbuf, max, fmt, args);
    va_end(args);
    return result;
}
