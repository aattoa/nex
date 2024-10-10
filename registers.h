#ifndef NEX_REGISTER_H
#define NEX_REGISTER_H

#include "util.h"
#include "strbuf.h"

#define REGISTER_UNNAMED '"'

enum {
    registers_count = 26 + 26 + 10 + 1 // Alphanumerics + unnamed.
};

struct registers {
    struct strbuf array[registers_count];
};

struct registers registers_new(void) NEX_CONST;

void registers_free(struct registers *registers) NEX_NONNULL;

struct strbuf *register_get(struct registers *registers, char name) NEX_NONNULL;

#endif // NEX_REGISTER_H
