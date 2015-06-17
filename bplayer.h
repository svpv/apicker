/*
 * Background player
 */

#ifndef BPLAYER_H
#define BPLAYER_H

#include <gtkmm.h>
#include "aplayer.h"

class BPlayer : public APlayer
{
public:
    BPlayer(const char *fname, Glib::RefPtr<Gtk::Adjustment> &aj);
    ~BPlayer();
    void play_bg();
    void play_bg(unsigned start, unsigned end);
    void stop_bg();
private:
    class Ctx;
    Ctx *m_ctx;
};

#endif
