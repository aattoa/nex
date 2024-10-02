#ifndef NEX_EDITOR_H
#define NEX_EDITOR_H

#include "filebuf.h"
#include "buffer.h"

struct editor {
    struct vector filebufs; // Element: `struct filebuf`
    size_t focus; // Zero means no focus
};

struct editor editor_new(void);

void editor_free(struct editor *editor);

struct filebuf *editor_current_filebuf(struct editor *editor);

enum filebuf_status editor_add_filebuf(struct editor *editor, const char *path);

enum filebuf_status editor_read_current_filebuf(struct editor *editor);

enum filebuf_status editor_write_current_filebuf(struct editor *editor);

#endif // NEX_EDITOR_H
