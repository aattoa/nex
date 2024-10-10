#include "settings.h"

struct nex_settings nex_settings_new(void) {
    return (struct nex_settings) {
        .scrolloff = 0,
        .sidescrolloff = 0,
        .showmode = true,
        .ruler = true,
    };
}
