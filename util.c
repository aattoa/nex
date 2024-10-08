#include "util.h"
#include <stdio.h>
#include <stdlib.h>

void die(const char *restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}
