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

void view_trim_whitespace(struct view *view) {
    while (view->len != 0 && is_space(view->ptr[0])) {
        view_remove_prefix_unchecked(view, 1);
    }
    while (view->len != 0 && is_space(view->ptr[view->len - 1])) {
        view_remove_suffix_unchecked(view, 1);
    }
}

struct view view_subview(struct view view, size_t offset, size_t length) {
    view_remove_prefix_unchecked(&view, min_uz(view.len, offset));
    view.len = min_uz(view.len, length);
    return view;
}

size_t view_find(struct view view, char byte) {
    for (size_t i = 0; i != view.len; ++i) {
        if (view.ptr[i] == byte) {
            return i;
        }
    }
    return SIZE_MAX;
}

bool view_split(struct view view, struct view *restrict left, struct view *restrict right, size_t offset) {
    if (view.len < offset) {
        return false;
    }
    *left = *right = view;
    view_remove_suffix_unchecked(left, view.len - offset);
    view_remove_prefix_unchecked(right, offset);
    return true;
}

bool view_split_char(struct view view, struct view *restrict left, struct view *restrict right, char byte) {
    size_t offset = view_find(view, byte);
    if (offset == SIZE_MAX) {
        return false;
    }
    view_split(view, left, right, offset);
    view_remove_prefix_unchecked(right, 1);
    return true;
}
