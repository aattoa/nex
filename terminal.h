#ifndef NEX_TERMINAL_H
#define NEX_TERMINAL_H

#include "util.h"

struct termpos {
    uint16_t x, y;
};

struct termsize {
    uint16_t width, height;
};

int terminal_read_input(void) NEX_NODISCARD;

struct termsize terminal_get_size(void) NEX_NODISCARD;

void terminal_enter_raw_mode(void);

void terminal_restore_previous_mode(void);

void terminal_set_cursor(struct termpos position);

void terminal_flush(void);

NEX_GNU_ATTRIBUTE(format(printf, 1, 2))
void terminal_print(const char *restrict fmt, ...);

#define TERMINAL_CLEAR                  "\033[2J"
#define TERMINAL_CLEAR_LINE             "\033[2K"
#define TERMINAL_SHOW_CURSOR            "\033[?25h"
#define TERMINAL_HIDE_CURSOR            "\033[?25l"
#define TERMINAL_ENTER_ALTERNATE_SCREEN "\033[?1049h"
#define TERMINAL_LEAVE_ALTERNATE_SCREEN "\033[?1049l"
#define TERMINAL_RESET_CURSOR           "\033[H"

#endif // NEX_TERMINAL_H
