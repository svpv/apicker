#ifndef CURSORS_H
#define CURSORS_H

#include <gtkmm.h>

enum CursorType {
    CURSOR_HAND_OPEN,
    CURSOR_HAND_CLOSED,
    CURSOR_NUM_CURSORS,
};

GdkCursor *cursor_get(enum CursorType t);

#endif
