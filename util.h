#ifndef NEX_UTIL_H
#define NEX_UTIL_H

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
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

NEX_INLINE NEX_PURE NEX_NONNULL
bool streq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

NEX_INLINE NEX_CONST
const char *stror(const char *a, const char *b) {
    return a ? a : b ? b : "(null)";
}

size_t digit_count(size_t n) NEX_CONST;

bool is_digit(int key) NEX_NODISCARD;
bool is_print(int key) NEX_NODISCARD;
bool is_space(int key) NEX_NODISCARD;
bool is_alpha(int key) NEX_NODISCARD;
bool is_alnum(int key) NEX_NODISCARD;
bool is_lower(int key) NEX_NODISCARD;
bool is_upper(int key) NEX_NODISCARD;

#define NEX_INT(abbrev, type, min, max) \
NEX_INLINE NEX_CONST type min_ ## abbrev(type a, type b) { return a < b ? a : b; } \
NEX_INLINE NEX_CONST type max_ ## abbrev(type a, type b) { return a > b ? a : b; } \
NEX_INLINE NEX_CONST type sat_sub_ ## abbrev(type a, type b) { return min + b > a ? min : a - b; } \
NEX_INLINE NEX_CONST type sat_add_ ## abbrev(type a, type b) { return max - b < a ? max : a + b; }

NEX_INT(i8,  int8_t,   INT8_MIN,  INT8_MAX)
NEX_INT(i16, int16_t,  INT16_MIN, INT16_MAX)
NEX_INT(i32, int32_t,  INT32_MIN, INT32_MAX)
NEX_INT(i64, int64_t,  INT64_MIN, INT64_MAX)
NEX_INT(u8,  uint8_t,  0,         UINT8_MAX)
NEX_INT(u16, uint16_t, 0,         UINT16_MAX)
NEX_INT(u32, uint32_t, 0,         UINT32_MAX)
NEX_INT(u64, uint64_t, 0,         UINT64_MAX)
NEX_INT(uz,  size_t,   0,         SIZE_MAX)

#undef NEX_INT

#endif // NEX_UTIL_H
