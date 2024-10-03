#ifndef NEX_TERMINAL_H
#define NEX_TERMINAL_H

#include "util.h"
#include <stdbool.h>
#include <stdint.h>

struct termpos {
    int16_t x, y;
};

struct termsize {
    int16_t w, h;
};

void terminal_enter_raw_mode(void);

void terminal_restore_previous_mode(void);

void terminal_set_cursor(struct termpos position);

NEX_GNU_ATTRIBUTE(format(printf, 1, 2))
void terminal_print(const char *restrict fmt, ...);

#define TERMINAL_CLEAR                  "\033[2J"
#define TERMINAL_CLEAR_LINE             "\033[2K"
#define TERMINAL_SHOW_CURSOR            "\033[?25h"
#define TERMINAL_HIDE_CURSOR            "\033[?25l"
#define TERMINAL_ENTER_ALTERNATE_SCREEN "\033[?1049h"
#define TERMINAL_LEAVE_ALTERNATE_SCREEN "\033[?1049l"

#endif // NEX_TERMINAL_H
