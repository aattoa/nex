#ifndef NEX_KEYCODES_H
#define NEX_KEYCODES_H

enum {
    NEX_KEY_NONE = 300, // Arbitrary starting point above 256
    NEX_KEY_UP,
    NEX_KEY_DOWN,
    NEX_KEY_LEFT,
    NEX_KEY_RIGHT,
    NEX_KEY_ENTER,
    NEX_KEY_BACKSPACE,
    NEX_KEY_ESCAPE,
};

#define NEX_KEY_CONTROL(character) ((character) - 96)
#define NEX_KEY_SHIFT(character) ((character) - 32)

#endif // NEX_KEYCODES_H
