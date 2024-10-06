#include "util.h"
#include "editor.h"
#include "terminal.h"

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

static void terminal_start(void) {
    terminal_enter_raw_mode();
    terminal_print("%s%s", TERMINAL_ENTER_ALTERNATE_SCREEN, TERMINAL_CLEAR);
}

static void terminal_stop(void) {
    terminal_restore_previous_mode();
    terminal_print("%s", TERMINAL_LEAVE_ALTERNATE_SCREEN);
}

static void handle_cmdline(struct editor *editor) {
    terminal_set_cursor((struct termpos) { .x = 0, .y = 0 });
    terminal_print("%s:%s\nstatus: %s", TERMINAL_CLEAR, stror(editor->cmdline.ptr, ""), stror(editor->message.ptr, "none"));
    terminal_set_cursor((struct termpos) { .x = editor->cmdline_cursor + 2, .y = 1 });
    fflush(stdout);
    editor_handle_key_cmdline(editor, terminal_read_input());
}

static void handle_editline(struct editor *editor) {
    if (editor->editline == NULL) {
        die("null editline");
    }
    terminal_set_cursor((struct termpos) { .x = 0, .y = 0 });
    terminal_print("Editing line %zu\n%s>%s", editor->line_index + 1, TERMINAL_CLEAR_LINE, stror(editor->editline->ptr, ""));
    terminal_set_cursor((struct termpos) { .x = editor->editline_cursor + 2, .y = 2 });
    fflush(stdout);
    editor_handle_key_editline(editor, terminal_read_input());
}

int main(int argc, const char **argv) {
    struct editor editor = editor_new();
    process_arguments(&editor, argv + 1, argv + argc);

    // Enable full output buffering.
    setvbuf(stdout, NULL, _IOFBF, BUFSIZ);

    terminal_start();
    atexit(terminal_stop);

    while (editor.mode != editor_mode_quit) {
        if (editor.mode == editor_mode_cmdline) {
            handle_cmdline(&editor);
        }
        else if (editor.mode == editor_mode_editline) {
            handle_editline(&editor);
        }
        else {
            die("unhandled mode");
        }
    }

    editor_free(&editor);
}
