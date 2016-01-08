#include <atomic>
#include <thread>
#include "bplayer.h"

class BPlayer::Ctx
{
public: 
    Ctx(Glib::RefPtr<Gtk::Adjustment> &aj, BPlayer &bp) :
	m_aj(aj), m_playing_requested(false), m_child_exiting(false),
	m_signalling(false), m_endpos(AEOF)
    {
	m_aj->signal_value_changed().connect(
		[this, &bp]() { change_pos(bp); });
    }

    void change_pos(BPlayer &bp)
    {
	if (m_playing_requested && !m_signalling) {
	    if (m_endpos != AEOF)
		bp.stop_bg(); // stop if playing short passage
	    else
		bp.play_bg(); // othrwise, rewind and continue
	}
    }

    Glib::RefPtr<Gtk::Adjustment> m_aj;
    std::thread m_thread;
    volatile std::atomic<bool> m_playing_requested;
    volatile std::atomic<bool> m_child_exiting;
    bool m_signalling;
    unsigned m_endpos;
    sigc::connection m_tick;
    sigc::slot<void> m_reset;
};

BPlayer::BPlayer(const char *fname, Glib::RefPtr<Gtk::Adjustment> &aj)
    : APlayer(fname)
{
    m_ctx = new BPlayer::Ctx(aj, *this);
}

BPlayer::~BPlayer()
{
    stop_bg();
    delete m_ctx;
}

void BPlayer::play_bg(unsigned begin, unsigned end, sigc::slot<void> reset)
{
    stop_bg();

    bool passage = !(begin == AEOF && end == 0);
    unsigned pos = m_ctx->m_aj->get_value();
    if (passage)
	pos = begin;

    try {
	seek(pos);
    }
    catch (...) {
	fprintf(stderr, "cannot seek\n");
	return;
    }

    m_ctx->m_playing_requested = true;
    m_ctx->m_child_exiting = false;
    m_ctx->m_endpos = passage ? end : AEOF;
    m_ctx->m_reset = reset;

    auto bg_loop = [this]
    {
	while (1) {
	    unsigned pos = read1();
	    if (pos >= m_ctx->m_endpos)
		break;
	    if (!m_ctx->m_playing_requested)
		break;
	    process1();
	    if (!m_ctx->m_playing_requested)
		break;
	}
	m_ctx->m_child_exiting = true;
    };

    m_ctx->m_thread = std::thread(bg_loop);

    auto on_tick = [this]() -> bool
    {
	m_ctx->m_signalling = true;
	m_ctx->m_aj->set_value(getpos());
	m_ctx->m_signalling = false;
	if (m_ctx->m_child_exiting)
	    stop_bg();
	return true;
    };

    if (m_ctx->m_tick)
	m_ctx->m_tick.unblock();
    else
	m_ctx->m_tick = Glib::signal_timeout().connect(on_tick, 10);
}

void BPlayer::play_bg()
{
    play_bg(AEOF, 0);
}

void BPlayer::stop_bg()
{
    if (!m_ctx->m_playing_requested)
	return;
    m_ctx->m_playing_requested = false;
    m_ctx->m_thread.join();
    m_ctx->m_tick.block();
    m_ctx->m_reset();
}
