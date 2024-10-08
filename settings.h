#ifndef NEX_SETTINGS_H
#define NEX_SETTINGS_H

#include "util.h"

struct nex_settings {
    size_t scrolloff;
    size_t sidescrolloff;
};

struct nex_settings nex_settings_new(void) NEX_CONST;

#endif // NEX_SETTINGS_H
