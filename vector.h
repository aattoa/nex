#ifndef NEX_VECTOR_H
#define NEX_VECTOR_H

#include <stdbool.h>
#include <stddef.h>

struct vector {
    void *ptr;
    size_t len;
    size_t cap;
    size_t elem_size;
    void (*destroy_elem)(void*);
};

struct vector vector_new(size_t elem_size, void (*destroy_elem)(void*));

void vector_clear(struct vector *vector);

void vector_free(struct vector *vector);

void *vector_at(struct vector *vector, size_t index);

void *vector_at_unchecked(struct vector *vector, size_t index);

bool vector_reserve(struct vector *vector, size_t capacity);

bool vector_push(struct vector *vector, const void *bytes);

bool vector_pop(struct vector *vector);

bool vector_insert(struct vector *vector, size_t index, const void *bytes);

bool vector_erase(struct vector *vector, size_t index);

#endif // NEX_VECTOR_H
