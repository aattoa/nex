#include "util.h"
#include "editor.h"
#include "terminal.h"
#include "keycodes.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void show_usage(void) {
    puts("Usage: nex [options] [file ...]"
        "\nOptions:"
        "\n\t-h, --help    \tDisplay help information"
        "\n\t-v, --version \tDisplay version information"
        "\n\t--            \tTreat remaining arguments as filenames");
}

static void process_arguments(struct editor *editor, const char **begin, const char **end) {
    bool handle_flags = true;
    for (const char **arg = begin; arg != end; ++arg) {
        if (handle_flags && **arg == '-') {
            if (streq(*arg, "--")) {
                handle_flags = false;
            }
            else if (streq(*arg, "-h") || streq(*arg, "--help")) {
                show_usage();
            }
            else if (streq(*arg, "-v") || streq(*arg, "--version")) {
                puts("nex 0.0.1");
            }
            else {
                fprintf(stderr, "Unrecognized option '%s'\n", *arg);
            }
        }
        else {
            enum filebuf_status status = editor_add_filebuf(editor, *arg);
            if (status != filebuf_ok) {
                fprintf(stderr, "Could not edit '%s': %s\n", *arg, filebuf_status_describe(status));
            }
        }
    }
}

void terminal_start(void) {
    terminal_enter_raw_mode();
    terminal_print("%s", TERMINAL_ENTER_ALTERNATE_SCREEN TERMINAL_HIDE_CURSOR);
}

void terminal_stop(void) {
    terminal_restore_previous_mode();
    terminal_print("%s", TERMINAL_LEAVE_ALTERNATE_SCREEN TERMINAL_SHOW_CURSOR);
}

int main(int argc, const char **argv) {
    struct editor editor = editor_new();
    process_arguments(&editor, argv + 1, argv + argc);

    // Enable full output buffering.
    setvbuf(stdout, NULL, _IOFBF, BUFSIZ);

    terminal_start();
    atexit(terminal_stop);
    fflush(stdout);

    terminal_print("%s", TERMINAL_CLEAR);
    fflush(stdout);

    bool quit = false;
    while (!quit) {
        int key = terminal_read_input();
        if (key == NEX_KEY_CONTROL('c')) {
            quit = true;
        }
        terminal_set_cursor((struct termpos) { 0, 0 });
        terminal_print("%sgot %d\n", TERMINAL_CLEAR_LINE, key);
        fflush(stdout);
    }

    editor_free(&editor);
}
