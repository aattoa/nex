#ifndef NEX_KEYCODES_H
#define NEX_KEYCODES_H

enum {
    NEX_KEY_NONE      = 0,
    NEX_KEY_ENTER     = 13,
    NEX_KEY_ESCAPE    = 27,
    NEX_KEY_BACKSPACE = 127,

    NEX_KEY_UP = 300, // Arbitrary starting point above 256
    NEX_KEY_DOWN,
    NEX_KEY_LEFT,
    NEX_KEY_RIGHT,
};

#define NEX_KEY_CONTROL(character) ((character) - 96)
#define NEX_KEY_SHIFT(character)   ((character) - 32)

#endif // NEX_KEYCODES_H
