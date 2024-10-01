#include "buffer.h"
#include <stdlib.h>
#include <string.h>

struct buffer buffer_new(void) {
    return (struct buffer) { .len = 0, .ptr = NULL };
}

void buffer_free(struct buffer *buffer) {
    if (buffer->ptr) {
        free(buffer->ptr);
        buffer->len = 0;
        buffer->ptr = NULL;
    }
}

void buffer_destroy(void *buffer) {
    buffer_free((struct buffer*)buffer);
}

bool buffer_allocate(struct buffer *buffer, size_t len) {
    if (len == 0) {
        *buffer = buffer_new();
        return true;
    }
    void *ptr = malloc(len);
    if (ptr == NULL) {
        return false;
    }
    buffer->len = len;
    buffer->ptr = ptr;
    return true;
}

bool buffer_copy(struct buffer *destination, struct view source) {
    bool ok = buffer_allocate(destination, source.len);
    if (ok) {
        memcpy(destination->ptr, source.ptr, source.len);
    }
    return ok;
}

struct view buffer_view(struct buffer *buffer) {
    return (struct view) { .len = buffer->len, .ptr = buffer->ptr };
}
