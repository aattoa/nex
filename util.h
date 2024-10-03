#ifndef NEX_UTIL_H
#define NEX_UTIL_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __GNUC__
#define NEX_GNU_ATTRIBUTE(...) __attribute__((__VA_ARGS__))
#else
#define NEX_GNU_ATTRIBUTE(...)
#endif

NEX_GNU_ATTRIBUTE(noreturn, format(printf, 1, 2))
void die(const char *restrict fmt, ...);

NEX_GNU_ATTRIBUTE(const)
size_t max_uz(size_t a, size_t b);

NEX_GNU_ATTRIBUTE(const)
size_t min_uz(size_t a, size_t b);

NEX_GNU_ATTRIBUTE(pure)
bool streq(const char *a, const char *b);

char *nex_strdup(const char *str);

#endif // NEX_UTIL_H
