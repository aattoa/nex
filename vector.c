#include "vector.h"
#include <stdlib.h>
#include <string.h>

NEX_CONST static size_t vector_grow_capacity(size_t capacity) {
    return capacity == 0 ? 8 : min_uz(capacity * 2, capacity + 1028);
}

static bool vector_grow_if_at_capacity(struct vector *vector) {
    if (vector->len != vector->cap) {
        return true;
    }
    size_t new_cap = vector_grow_capacity(vector->cap);
    return new_cap > vector->cap && vector_reserve(vector, new_cap);
}

struct vector vector_new(size_t elem_size, void (*destroy_elem)(void*)) {
    return (struct vector) {
        .ptr = NULL,
        .len = 0,
        .cap = 0,
        .elem_size = elem_size,
        .destroy_elem = destroy_elem,
    };
}

void vector_clear(struct vector *vector) {
    if (vector->destroy_elem != NULL) {
        for (size_t i = 0; i != vector->len; ++i) {
            vector->destroy_elem(vector_at_unchecked(vector, i));
        }
    }
    vector->len = 0;
}

void vector_free(struct vector *vector) {
    vector_clear(vector);
    free(vector->ptr);
    vector->ptr = NULL;
    vector->cap = 0;
}

void *vector_at(struct vector *vector, size_t index) {
    return index < vector->len ? vector_at_unchecked(vector, index) : NULL;
}

void *vector_at_unchecked(struct vector *vector, size_t index) {
    assert(vector->ptr != NULL);
    assert(index < vector->len);
    return (char*)vector->ptr + index * vector->elem_size;
}

bool vector_reserve(struct vector *vector, size_t capacity) {
    if (vector->cap < capacity) {
        void *ptr = realloc(vector->ptr, capacity * vector->elem_size);
        if (ptr == NULL) {
            return false;
        }
        vector->ptr = ptr;
        vector->cap = capacity;
    }
    return true;
}

bool vector_push(struct vector *vector, const void *bytes) {
    if (!vector_grow_if_at_capacity(vector)) {
        return false;
    }
    assert(vector->len < vector->cap);
    memcpy((char*)vector->ptr + vector->len * vector->elem_size, bytes, vector->elem_size);
    ++vector->len;
    return true;
}

bool vector_pop(struct vector *vector) {
    return vector_pop_n(vector, 1);
}

bool vector_pop_n(struct vector *vector, size_t n) {
    if (vector->len < n) {
        return false;
    }
    if (vector->destroy_elem != NULL) {
        for (size_t i = vector->len - n - 1; i != vector->len; ++i) {
            vector->destroy_elem(vector_at_unchecked(vector, i));
        }
    }
    vector->len -= n;
    return true;
}

bool vector_insert(struct vector *vector, size_t index, const void *bytes) {
    if (index == vector->len) {
        return vector_push(vector, bytes);
    }
    if (!vector_grow_if_at_capacity(vector)) {
        return false;
    }
    ++vector->len;
    memmove(
        vector_at_unchecked(vector, index + 1),
        vector_at_unchecked(vector, index),
        (vector->len - index - 1) * vector->elem_size);
    memcpy(vector_at_unchecked(vector, index), bytes, vector->elem_size);
    return true;
}

bool vector_erase(struct vector *vector, size_t index) {
    return vector_erase_n(vector, index, 1);
}

bool vector_erase_n(struct vector *vector, size_t index, size_t n) {
    if (index + n >= vector->len) {
        return false;
    }
    if (index == vector->len - n) {
        return vector_pop_n(vector, n);
    }
    if (vector->destroy_elem != NULL) {
        for (size_t i = 0; i != n; ++i) {
            vector->destroy_elem(vector_at_unchecked(vector, index + i));
        }
    }
    memmove(
        vector_at_unchecked(vector, index),
        vector_at_unchecked(vector, index + n),
        (vector->len - index - n) * vector->elem_size);
    vector->len -= n;
    return true;
}
