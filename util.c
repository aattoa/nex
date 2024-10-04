#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void die(const char *restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

size_t max_uz(size_t a, size_t b) {
    return a > b ? a : b;
}

size_t min_uz(size_t a, size_t b) {
    return a < b ? a : b;
}

bool streq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

const char *stror(const char *a, const char *b) {
    return a ? a : b ? b : "(null)";
}
