#include "view.h"
#include <string.h>

struct view view_from(const char *string) {
    return (struct view) { .len = strlen(string), .ptr = string };
}

bool view_deep_equal(struct view lhs, struct view rhs) {
    return lhs.len == rhs.len && (lhs.ptr == rhs.ptr || memcmp(lhs.ptr, rhs.ptr, lhs.len) == 0);
}

bool view_starts_with(struct view view, struct view prefix) {
    return view.len >= prefix.len && memcmp(view.ptr, prefix.ptr, prefix.len) == 0;
}

bool view_ends_with(struct view view, struct view suffix) {
    return view.len >= suffix.len && memcmp(view.ptr + view.len - suffix.len, suffix.ptr, suffix.len) == 0;
}

bool view_extract_prefix(struct view *view, size_t length, char *buffer) {
    if (view->len >= length) {
        memcpy(buffer, view->ptr, length);
        view_remove_prefix_unchecked(view, length);
        return true;
    }
    return false;
}

bool view_extract_suffix(struct view *view, size_t length, char *buffer) {
    if (view->len >= length) {
        memcpy(buffer, view->ptr + view->len - length, length);
        view_remove_suffix_unchecked(view, length);
        return true;
    }
    return false;
}

bool view_remove_prefix_n(struct view *view, size_t n) {
    if (view->len >= n) {
        view_remove_prefix_unchecked(view, n);
        return true;
    }
    return false;
}

bool view_remove_prefix(struct view *view, struct view prefix) {
    if (view_starts_with(*view, prefix)) {
        view_remove_prefix_unchecked(view, prefix.len);
        return true;
    }
    return false;
}

bool view_remove_suffix_n(struct view *view, size_t n) {
    if (view->len >= n) {
        view_remove_suffix_unchecked(view, n);
        return true;
    }
    return false;
}

bool view_remove_suffix(struct view *view, struct view suffix) {
    if (view_ends_with(*view, suffix)) {
        view_remove_suffix_unchecked(view, suffix.len);
        return true;
    }
    return false;
}

void view_remove_prefix_unchecked(struct view *view, size_t length) {
    view->len -= length;
    view->ptr += length;
}

void view_remove_suffix_unchecked(struct view *view, size_t length) {
    view->len -= length;
}

static bool is_space(char character) {
    return character == ' ' || character == '\t';
}

void view_trim_whitespace(struct view *view) {
    while (view->len != 0 && is_space(view->ptr[0])) {
        view_remove_prefix_unchecked(view, 1);
    }
    while (view->len != 0 && is_space(view->ptr[view->len - 1])) {
        view_remove_suffix_unchecked(view, 1);
    }
}
