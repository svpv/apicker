#include <pthread.h>
#include "bplayer.h"

class BPlayer::Ctx
{
public: 
    Ctx(Glib::RefPtr<Gtk::Adjustment> &aj)
	: m_aj(aj), m_playing(false)
    {

    }
    Glib::RefPtr<Gtk::Adjustment> m_aj;
    pthread_t m_thread;
    volatile bool m_playing;
    sigc::connection m_tick;
};

BPlayer::BPlayer(const char *fname, Glib::RefPtr<Gtk::Adjustment> &aj)
    : APlayer(fname)
{
    m_ctx = new BPlayer::Ctx(aj);
}

BPlayer::~BPlayer()
{
    if (m_ctx->m_playing)
	stop_bg();
    delete m_ctx;
}

static void *bg_play_routine(void *arg)
{
    BPlayer *bp = (BPlayer *) arg;
    bp->seek(bp->m_ctx->m_aj->get_value());
    int percent;
    do
	percent = bp->readsome();
    while (percent < 100 && bp->m_ctx->m_playing);
    return NULL;
}

static bool signalling_routine(BPlayer *bp)
{
    bp->m_ctx->m_aj->set_value(bp->getpos());
    return true;
}

void BPlayer::play_bg()
{
    if (m_ctx->m_playing)
	stop_bg();
    m_ctx->m_playing = true;
    if (pthread_create(&m_ctx->m_thread, NULL, bg_play_routine, this) != 0)
	throw "cannot create player thread";
    if (m_ctx->m_tick)
	m_ctx->m_tick.unblock();
    else
	m_ctx->m_tick = Glib::signal_timeout().connect(
		sigc::bind(sigc::ptr_fun(signalling_routine), this), 10);
}

void BPlayer::stop_bg()
{
    m_ctx->m_playing = false;
    if (pthread_join(m_ctx->m_thread, NULL) != 0)
	throw "cannot join player thread";
    m_ctx->m_tick.block();
}
