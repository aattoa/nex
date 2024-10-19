#ifndef NEX_VECTOR_H
#define NEX_VECTOR_H

#include "util.h"

struct vector {
    void *ptr;
    size_t len;
    size_t cap;
    size_t elem_size;
    void (*destroy_elem)(void*);
};

struct vector vector_new(size_t elem_size, void (*destroy_elem)(void*)) NEX_CONST;

void vector_clear(struct vector *vector) NEX_NONNULL;

void vector_free(struct vector *vector) NEX_NONNULL;

void *vector_front(struct vector *vector) NEX_NONNULL NEX_NODISCARD;

void *vector_back(struct vector *vector) NEX_NONNULL NEX_NODISCARD;

void *vector_at(struct vector *vector, size_t index) NEX_NONNULL NEX_NODISCARD;

void *vector_at_unchecked(struct vector *vector, size_t index) NEX_NONNULL NEX_NODISCARD;

bool vector_reserve(struct vector *vector, size_t capacity) NEX_NONNULL;

bool vector_push(struct vector *vector, const void *bytes) NEX_NONNULL;

bool vector_pop(struct vector *vector) NEX_NONNULL;

bool vector_pop_n(struct vector *vector, size_t n) NEX_NONNULL;

bool vector_insert(struct vector *vector, size_t index, const void *bytes) NEX_NONNULL;

bool vector_erase(struct vector *vector, size_t index) NEX_NONNULL;

bool vector_erase_n(struct vector *vector, size_t index, size_t n) NEX_NONNULL;

#endif // NEX_VECTOR_H
