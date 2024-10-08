#ifndef NEX_UTIL_H
#define NEX_UTIL_H

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#ifdef __GNUC__
#define NEX_GNU_ATTRIBUTE(...) __attribute__((__VA_ARGS__))
#else
#define NEX_GNU_ATTRIBUTE(...)
#endif

#define NEX_PURE      NEX_GNU_ATTRIBUTE(pure)
#define NEX_CONST     NEX_GNU_ATTRIBUTE(const)
#define NEX_NONNULL   NEX_GNU_ATTRIBUTE(nonnull)
#define NEX_NODISCARD NEX_GNU_ATTRIBUTE(warn_unused_result)
#define NEX_INLINE    inline NEX_GNU_ATTRIBUTE(always_inline)

NEX_GNU_ATTRIBUTE(noreturn, format(printf, 1, 2))
void die(const char *restrict fmt, ...);

NEX_INLINE NEX_CONST
size_t max_uz(size_t a, size_t b) {
    return a > b ? a : b;
}

NEX_INLINE NEX_CONST
size_t min_uz(size_t a, size_t b) {
    return a < b ? a : b;
}

NEX_INLINE NEX_PURE NEX_NONNULL
bool streq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

NEX_INLINE NEX_CONST
const char *stror(const char *a, const char *b) {
    return a ? a : b ? b : "(null)";
}

bool is_digit(int key) NEX_NODISCARD;
bool is_print(int key) NEX_NODISCARD;
bool is_space(int key) NEX_NODISCARD;
bool is_alpha(int key) NEX_NODISCARD;
bool is_alnum(int key) NEX_NODISCARD;
bool is_lower(int key) NEX_NODISCARD;
bool is_upper(int key) NEX_NODISCARD;

#endif // NEX_UTIL_H
