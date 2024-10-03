#include "util.h"
#include "terminal.h"

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <termios.h>
#include <unistd.h>

static void require(bool condition, const char *message) {
    if (!condition) {
        die("%s: %s", message, strerror(errno));
    }
}

static struct termios previous_term;

void terminal_enter_raw_mode(void) {
    require(tcgetattr(STDIN_FILENO, &previous_term) != -1, "tcgetattr");
    struct termios term = previous_term;
    term.c_iflag &= ~(ICRNL | IXON);
    term.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    require(tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) != -1, "tcsetattr");
}

void terminal_restore_previous_mode(void) {
    require(tcsetattr(STDIN_FILENO, TCSAFLUSH, &previous_term) != -1, "tcsetattr");
}

void terminal_set_cursor(struct termpos position) {
    require(printf("\033[%i;%iH", (int)position.y, (int)position.x) >= 0, "printf");
}

void terminal_print(const char *restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    require(vprintf(fmt, args) >= 0, "vprintf");
    va_end(args);
}
