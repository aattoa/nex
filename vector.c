#include "vector.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

static bool vector_grow_if_at_capacity(struct vector *vector) {
    if (vector->len != vector->cap) {
        return true;
    }
    size_t new_cap = vector->cap == 0 ? 8 : min_uz(vector->cap * 2, vector->cap + 1028);
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
    if (vector->len == 0) {
        return false;
    }
    --vector->len;
    if (vector->destroy_elem != NULL) {
        vector->destroy_elem(vector_at_unchecked(vector, vector->len));
    }
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
        (vector->len - 1 - index) * vector->elem_size);
    memcpy(vector_at_unchecked(vector, index), bytes, vector->elem_size);
    return true;
}

bool vector_erase(struct vector *vector, size_t index) {
    if (index >= vector->len) {
        return false;
    }
    if (index == vector->len - 1) {
        return vector_pop(vector);
    }
    if (vector->destroy_elem != NULL) {
        vector->destroy_elem(vector_at_unchecked(vector, index));
    }
    memmove(
        vector_at_unchecked(vector, index),
        vector_at_unchecked(vector, index + 1),
        (vector->len - index) * vector->elem_size);
    --vector->len;
    return true;
}
