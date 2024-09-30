#ifndef NEX_BUFFER_H
#define NEX_BUFFER_H

#include "view.h"
#include <stddef.h>

struct buffer {
    size_t len;
    char *ptr;
};

struct buffer buffer_new(void);

// Returns `true` on success.
bool buffer_allocate(struct buffer *buffer, size_t len);

// Returns `true` on success.
bool buffer_copy(struct buffer *destination, struct view *source);

struct view buffer_view(struct buffer *buffer);

void buffer_free(struct buffer *buffer);

#endif // NEX_BUFFER_H
