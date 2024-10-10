#include "registers.h"

struct registers registers_new(void) {
    struct registers registers;
    for (size_t i = 0; i != registers_count; ++i) {
        registers.array[i] = strbuf_new();
    }
    return registers;
}

void registers_free(struct registers *registers) {
    for (size_t i = 0; i != registers_count; ++i) {
        strbuf_free(&registers->array[i]);
    }
}

struct strbuf *register_get(struct registers *registers, char name) {
    if ('0' <= name && name <= '9') return &registers->array[name - '0'];
    if ('a' <= name && name <= 'z') return &registers->array[10 + (name - 'a')];
    if ('A' <= name && name <= 'Z') return &registers->array[10 + 26 + (name - 'A')];
    if (name == REGISTER_UNNAMED)   return &registers->array[10 + 26 + 26];
    return NULL;
}
