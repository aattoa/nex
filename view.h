#ifndef NEX_VIEW_H
#define NEX_VIEW_H

#include "util.h"
#include <stdbool.h>
#include <stddef.h>

struct view {
    size_t len;
    const char *ptr;
};

struct view view_from(const char *string) NEX_PURE NEX_NONNULL;

bool view_deep_equal(struct view lhs, struct view rhs) NEX_PURE;

bool view_starts_with(struct view view, struct view prefix) NEX_PURE;

bool view_ends_with(struct view view, struct view suffix) NEX_PURE;

bool view_extract_prefix(struct view *view, size_t length, char *buffer) NEX_NODISCARD;

bool view_extract_suffix(struct view *view, size_t length, char *buffer) NEX_NODISCARD;

bool view_remove_prefix_n(struct view *view, size_t n) NEX_NONNULL;

bool view_remove_prefix(struct view *view, struct view prefix) NEX_NONNULL;

bool view_remove_suffix_n(struct view *view, size_t n) NEX_NONNULL;

bool view_remove_suffix(struct view *view, struct view suffix) NEX_NONNULL;

void view_remove_prefix_unchecked(struct view *view, size_t length) NEX_NONNULL;

void view_remove_suffix_unchecked(struct view *view, size_t length) NEX_NONNULL;

void view_trim_whitespace(struct view *view) NEX_NONNULL;

struct view view_subview(struct view view, size_t offset, size_t length) NEX_CONST;

#endif // NEX_VIEW_H
