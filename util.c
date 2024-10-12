#include "util.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

void die(const char *restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

size_t digit_count(size_t n) {
    size_t digits = 0;
    do { n /= 10; ++digits; } while (n != 0);
    return digits;
}

size_t saturating_sub(size_t a, size_t b) {
    return a < b ? 0 : a - b;
}

#define KEY_PREDICATE(category) bool is_ ## category(int key) { return key >= 0 && key <= UCHAR_MAX && is ## category(key); }
KEY_PREDICATE(digit)
KEY_PREDICATE(print)
KEY_PREDICATE(space)
KEY_PREDICATE(alpha)
KEY_PREDICATE(alnum)
KEY_PREDICATE(lower)
KEY_PREDICATE(upper)
#undef KEY_PREDICATE
