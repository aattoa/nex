#ifndef NEX_UTIL_H
#define NEX_UTIL_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

size_t max_uz(size_t a, size_t b);
size_t min_uz(size_t a, size_t b);

bool streq(const char *a, const char *b);

#endif // NEX_UTIL_H
