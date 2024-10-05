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

#define NEX_PURE      NEX_GNU_ATTRIBUTE(pure)
#define NEX_CONST     NEX_GNU_ATTRIBUTE(const)
#define NEX_NONNULL   NEX_GNU_ATTRIBUTE(nonnull)
#define NEX_NODISCARD NEX_GNU_ATTRIBUTE(warn_unused_result)

NEX_GNU_ATTRIBUTE(noreturn, format(printf, 1, 2))
void die(const char *restrict fmt, ...);

size_t max_uz(size_t a, size_t b) NEX_CONST;

size_t min_uz(size_t a, size_t b) NEX_CONST;

bool streq(const char *a, const char *b) NEX_PURE NEX_NONNULL;

const char *stror(const char *a, const char *b) NEX_CONST;

#endif // NEX_UTIL_H
