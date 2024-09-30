#include "filebuf.h"
#include <stdio.h>

int main(void) {
    struct filebuf file;
    enum filebuf_status status = filebuf_read(&file, "test.txt");
    if (status == filebuf_ok) {
        printf("line count: %zu, capacity: %zu\n", file.line_count, file.line_capacity);
        for (size_t i = 0; i != file.line_count; ++i) {
            printf("line %zu: '", i + 1);
            fwrite(file.lines[i].ptr, 1, file.lines[i].len, stdout);
            printf("'\n");
        }
        filebuf_free(&file);
    }
    else {
        fprintf(stderr, "filebuf_read failed: %s\n", filebuf_status_describe(status));
    }
}
