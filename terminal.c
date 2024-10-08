#include "util.h"
#include "keycodes.h"
#include "terminal.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <poll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static void require(bool condition, const char *message) {
    if (!condition) {
        die("%s: %s", message, strerror(errno));
    }
}

static bool terminal_poll(void) {
    struct pollfd pollfd = { .fd = STDIN_FILENO, .events = POLLIN };
    int result = poll(&pollfd, 1, 10 /*ms*/);
    require(result != -1, "poll");
    return result != 0; // Zero indicates timeout
}

static bool terminal_read_byte(char *byte) {
    int result = read(STDIN_FILENO, byte, 1);
    require(result != -1, "read");
    return result != 0;
}

static int terminal_extract_control_sequence(void) {
    char byte;
    if (!terminal_read_byte(&byte)) {
        return NEX_KEY_NONE;
    }
    switch (byte) {
    case 'A': return NEX_KEY_UP;
    case 'B': return NEX_KEY_DOWN;
    case 'C': return NEX_KEY_RIGHT;
    case 'D': return NEX_KEY_LEFT;
    default: return byte;
    }
}

static char previous_false_csi_byte = '[';

int terminal_read_input(void) {
    char byte;
    if (previous_false_csi_byte != '[') {
        byte = previous_false_csi_byte;
        previous_false_csi_byte = '[';
    }
    else if (!terminal_read_byte(&byte)) {
        return NEX_KEY_NONE;
    }
    if (byte == 033) {
        if (terminal_poll()) {
            char csi;
            if (terminal_read_byte(&csi)) {
                if (csi == '[') {
                    return terminal_extract_control_sequence();
                }
                previous_false_csi_byte = csi;
            }
        }
        return NEX_KEY_ESCAPE;
    }
    else if (byte == 13) {
        return NEX_KEY_ENTER;
    }
    else if (byte == 127) {
        return NEX_KEY_BACKSPACE;
    }
    return byte;
}

struct termsize terminal_get_size(void) {
    struct winsize winsize;
    require(ioctl(STDIN_FILENO, TIOCGWINSZ, &winsize) != -1, "ioctl");
    return (struct termsize) { .width = winsize.ws_col, .height = winsize.ws_row };
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
