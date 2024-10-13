#include "util.h"
#include "editor.h"
#include "terminal.h"

#include <stdio.h>
#include <stdlib.h>

static void process_arguments(struct editor *editor, const char **begin, const char **end) {
    bool handle_flags = true;
    for (const char **arg = begin; arg != end; ++arg) {
        if (handle_flags && **arg == '-') {
            if (streq(*arg, "--")) {
                handle_flags = false;
            }
            else if (streq(*arg, "-h") || streq(*arg, "--help")) {
                puts("Usage: nex [options] [file ...]"
                    "\nOptions:"
                    "\n  -h, --help     \tDisplay help information"
                    "\n  -v, --version  \tDisplay version information"
                    "\n      --visual   \tStart in visual mode"
                    "\n      --dims WxH \tUse width=W and height=H");
                exit(EXIT_SUCCESS);
            }
            else if (streq(*arg, "-v") || streq(*arg, "--version")) {
                puts("nex 0.0.1");
                exit(EXIT_SUCCESS);
            }
            else if (streq(*arg, "--visual")) {
                editor->mode = editor_mode_vi;
            }
            else if (streq(*arg, "--dims")) {
                if (++arg == end) {
                    die("missing dims argument\n");
                }
                size_t w, h;
                if (sscanf(*arg, "%zux%zu", &w, &h) != 2 || w > UINT16_MAX || h > UINT16_MAX) {
                    die("invalid dims argument\n");
                }
                editor->size = (struct termsize) { .width = w, .height = h };
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

static void handle_cmdline(struct editor *editor) {
    terminal_print(
        "%s%s:%s\nstatus: %s",
        TERMINAL_RESET_CURSOR,
        TERMINAL_CLEAR,
        stror(editor->cmdline.ptr, ""),
        stror(editor->message.ptr, "none"));
    terminal_set_cursor((struct termpos) { .x = editor->cmdline_state.cursor + 2, .y = 1 });
    terminal_flush();
    editor_handle_key_cmdline(editor, terminal_read_input());
}

static void handle_editline(struct editor *editor) {
    if (editor->editline == NULL) {
        die("null editline\n");
    }
    struct view line = view_subview(strbuf_view(*editor->editline), editor->editline_state.leftmost_column, editor->size.width);
    terminal_print(
        "%s%sEditing line %zu\n%.*s\ncursor: %zu, leftmost: %zu, count: %zu, status: %s",
        TERMINAL_RESET_CURSOR,
        TERMINAL_CLEAR,
        editor->line_index + 1,
        (int)line.len,
        stror(line.ptr, ""),
        editor->editline_state.cursor + 1,
        editor->editline_state.leftmost_column + 1,
        editor->editline_state.count,
        stror(editor->message.ptr, "none"));
    terminal_set_cursor((struct termpos) { .x = editor->editline_state.cursor + 1 - editor->editline_state.leftmost_column, .y = 2 });
    terminal_flush();
    editor_handle_key_editline(editor, terminal_read_input());
}

static void handle_vi(struct editor *editor) {
    struct filebuf *filebuf = editor_current_filebuf(editor);
    if (filebuf == NULL) {
        die("null visual filebuf\n");
    }
    size_t number_width = editor->settings.number ? digit_count(editor->vi_state.frame.top + editor->size.height - 1) : 0;
    size_t top = editor->vi_state.frame.top;
    size_t bottom = min_uz(filebuf->lines.len, top + editor->size.height) - 1;
    assert(top < bottom);
    terminal_print("%s", TERMINAL_RESET_CURSOR);
    for (size_t i = top; i != bottom; ++i) {
        struct strbuf *strbuf = vector_at(&filebuf->lines, i);
        struct view line = view_subview(strbuf_view(*strbuf), editor->vi_state.frame.left, editor->size.width);
        terminal_print("%s", TERMINAL_CLEAR_LINE);
        if (editor->settings.number) {
            terminal_print("%*zu ", (int)number_width, i + 1);
        }
        size_t display_line_width = min_uz(line.len, sat_sub_uz(editor->size.width, number_width + 1));
        terminal_print("%.*s\n", (int)display_line_width, stror(line.ptr, ""));
    }
    for (size_t i = bottom + 1; i < top + editor->size.height; ++i) {
        terminal_print("%s~\n", TERMINAL_CLEAR_LINE);
    }
    terminal_print("%s", TERMINAL_CLEAR_LINE);
    if (editor->settings.showmode) {
        terminal_print("-- %s -- ", vi_mode_describe(editor->vi_state.mode));
    }
    if (editor->settings.ruler) {
        terminal_print("%zu,%zu ", editor->vi_state.cursor.y + 1, editor->vi_state.cursor.x + 1);
    }
    terminal_print(
        "count: %zu, reg: %i, status: %s",
        editor->vi_state.count,
        (int)editor->vi_state.regname,
        stror(editor->message.ptr, "none"));
    terminal_set_cursor((struct termpos) {
        .x = editor->vi_state.cursor.x + 1 - editor->vi_state.frame.left + number_width + (editor->settings.number ? 1 : 0),
        .y = editor->vi_state.cursor.y + 1 - editor->vi_state.frame.top,
    });
    terminal_flush();
    editor_handle_key_vi(editor, terminal_read_input());
}

static void terminal_start(void) {
    terminal_enter_raw_mode();
    terminal_print("%s%s", TERMINAL_ENTER_ALTERNATE_SCREEN, TERMINAL_CLEAR);
}

static void terminal_stop(void) {
    terminal_restore_previous_mode();
    terminal_print("%s", TERMINAL_LEAVE_ALTERNATE_SCREEN);
}

int main(int argc, const char **argv) {
    struct editor editor = editor_new(terminal_get_size());
    process_arguments(&editor, argv + 1, argv + argc);

    // Enable full output buffering.
    setvbuf(stdout, NULL, _IOFBF, BUFSIZ);

    terminal_start();
    atexit(terminal_stop);

    editor.settings.sidescrolloff = 40;
    editor.settings.scrolloff = 10;

    while (editor.mode != editor_mode_quit) {
        if (editor.mode == editor_mode_cmdline) {
            handle_cmdline(&editor);
        }
        else if (editor.mode == editor_mode_editline) {
            handle_editline(&editor);
            editor_cursor_scroll(
                &editor.editline_state.leftmost_column,
                editor.size.width,
                editor.editline_state.cursor,
                editor.settings.sidescrolloff);
        }
        else if (editor.mode == editor_mode_vi) {
            handle_vi(&editor);
            editor_cursor_scroll(
                &editor.vi_state.frame.left,
                editor.size.width,
                editor.vi_state.cursor.x,
                editor.settings.sidescrolloff);
            editor_cursor_scroll(
                &editor.vi_state.frame.top,
                editor.size.height - 1, // -1 for status bar
                editor.vi_state.cursor.y,
                editor.settings.scrolloff);
        }
        else {
            die("unhandled mode\n");
        }
    }

    editor_free(&editor);
}
