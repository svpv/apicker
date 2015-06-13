#include "cursor.h"

static const char *hand_open[] = {
    "20 20 3 1",
    "  c none",
    ". c white",
    "* c black",
    "                    ",
    "                    ",
    "         **         ",
    "     ** *..***      ",
    "    *..**..*..*     ",
    "    *..**..*..* *   ",
    "     *..*..*..**.*  ",
    "     *..*..*..*..*  ",
    "   ** *.......*..*  ",
    "  *..**..........*  ",
    "  *...*.........*   ",
    "   *............*   ",
    "    *...........*   ",
    "    *..........*    ",
    "     *.........*    ",
    "      *.......*     ",
    "       *......*     ",
    "       *......*     ",
    "                    ",
    "                    ",
};

static const char *hand_closed[] = {
    "20 20 3 1",
    "  c none",
    ". c white",
    "* c black",
    "                    ",
    "                    ",
    "                    ",
    "                    ",
    "                    ",
    "         **         ",
    "       **..***      ",
    "      *.......**    ",
    "      *.........*   ",
    "    ***.........*   ",
    "   *..*.........*   ",
    "   *............*   ",
    "    *...........*   ",
    "    *..........*    ",
    "     *.........*    ",
    "      *.......*     ",
    "       *......*     ",
    "       *......*     ",
    "                    ",
    "                    "
};

static const char **images[] = {
    hand_open,
    hand_closed,
    NULL,
};

GdkCursor *cursor_get(enum CursorType t)
{
    GdkPixbuf *p = gdk_pixbuf_new_from_xpm_data(images[t]);
    GdkCursor *c = gdk_cursor_new_from_pixbuf(gdk_display_get_default(), p, 10, 10);
    g_object_unref(p);
    return c;
}
