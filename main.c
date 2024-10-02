#include "editor.h"
#include "util.h"
#include <stdbool.h>
#include <stdio.h>

static void show_usage(void) {
    puts("Usage: nex [options] [file ...]\n"
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

int main(int argc, const char **argv) {
    struct editor editor = editor_new();
    process_arguments(&editor, argv + 1, argv + argc);
    printf("Editing %zu files\n", editor.filebufs.len);
    editor_free(&editor);
}
