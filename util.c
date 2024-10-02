#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void die(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

char *nex_strdup(const char *str) {
    size_t len = strlen(str) + 1;
    char *dup = malloc(len);
    if (dup != NULL) {
        memcpy(dup, str, len);
    }
    return dup;
}

bool streq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

size_t max_uz(size_t a, size_t b) {
    return a > b ? a : b;
}

size_t min_uz(size_t a, size_t b) {
    return a < b ? a : b;
}
