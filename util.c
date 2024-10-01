#include "util.h"
#include <string.h>

size_t max_uz(size_t a, size_t b) {
    return a > b ? a : b;
}

size_t min_uz(size_t a, size_t b) {
    return a < b ? a : b;
}

bool streq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}
