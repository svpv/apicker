#include <iostream>
#include <cmath>
#include <cstring>
#include <pthread.h>
#include <ao/ao.h>
#include "aplayer.h"

class LibAO
{
public:
    LibAO()
    {
	ao_initialize();
    }
    ~LibAO()
    {
	ao_shutdown();
    }
};

static LibAO libao;

class APlayer::Ctx
{
public: 
    Ctx(int channels, int rate);
    ~Ctx();
    void play(short *data, size_t n);
    void *buf(size_t size)
    {
	m_buf = realloc(m_buf, size);
	return m_buf;
    }
    volatile bool m_stop;
    sigc::connection m_tick;
    Glib::RefPtr<Gtk::Adjustment> m_aj;
    pthread_t m_thread;
private:
    int m_driver;
    ao_sample_format m_fmt;
    ao_device *m_dev;
    void *m_buf;
};

APlayer::Ctx::Ctx(int channels, int rate)
    : m_thread(0), m_buf(NULL)
{
    m_fmt.bits = 16;
    m_fmt.channels = channels;
    m_fmt.rate = rate;
    m_fmt.byte_format = AO_FMT_NATIVE;
    m_fmt.matrix = 0;

    m_driver = ao_default_driver_id();
    m_dev = ao_open_live(m_driver, &m_fmt, NULL);
}

APlayer::Ctx::~Ctx()
{
    free(m_buf);
}

void APlayer::Ctx::play(short *data, size_t n)
{
    size_t nc = m_fmt.channels;
    ao_play(m_dev, (char *) data, n * 2 * nc);
}

APlayer::APlayer(const char *fname)
    : AReader(fname)
{
    m_ctx = new APlayer::Ctx(channels(), rate());
}

APlayer::~APlayer()
{
    delete m_ctx;
}

static void *bg_play_routine(void *arg)
{
    APlayer *ap = (APlayer *) arg;
    ap->seek(ap->m_ctx->m_aj->get_value());
    int percent;
    do
	percent = ap->readsome();
    while (percent < 100 && !ap->m_ctx->m_stop);
    return NULL;
}

static bool signalling_routine(APlayer *ap)
{
    ap->m_ctx->m_aj->set_value(ap->getpos());
    return true;
}

void APlayer::play_bg(Glib::RefPtr<Gtk::Adjustment> &aj)
{
    if (m_ctx->m_thread)
	stop_bg();
    m_ctx->m_stop = false;
    m_ctx->m_aj = aj;
    if (pthread_create(&m_ctx->m_thread, NULL, bg_play_routine, this) != 0)
	throw "cannot create player thread";
    if (m_ctx->m_tick)
	m_ctx->m_tick.unblock();
    else
	m_ctx->m_tick = Glib::signal_timeout().connect(
		sigc::bind(sigc::ptr_fun(signalling_routine), this), 10);
}

void APlayer::stop_bg()
{
    m_ctx->m_stop = true;
    if (pthread_join(m_ctx->m_thread, NULL) != 0)
	throw "cannot join player thread";
    m_ctx->m_thread = 0;
    m_ctx->m_tick.block();
}

static inline float f2s(float f)
{
    if (f >= 1.0f)
	return  32767;
    if (f <= -1.0f)
	return -32767;
    return rintf(f * 32767);
}

static inline void conv1(short **data, short *out, size_t i, size_t nc)
{
    for (size_t j = 0; j < nc; j++)
	out[nc * i + j] = data[j][i];
}

static inline void conv1(float **data, short *out, size_t i, size_t nc)
{
    for (size_t j = 0; j < nc; j++)
	out[nc * i + j] = f2s(data[j][i]);
}

static inline void conv1(float  *data, short *out, size_t i, size_t nc)
{
    for (size_t j = 0; j < nc; j++)
	out[nc * i + j] = f2s(data[nc * i + j]);
}

template<typename T>
static void convert(T data, short *out, size_t n, size_t nc)
{
    switch (nc) {
    case 1:
	for (size_t i = 0; i < n; i++)
	    conv1(data, out, i, 1);
	break;
    case 2:
	for (size_t i = 0; i < n; i++)
	    conv1(data, out, i, 2);
	break;
    default:
	for (size_t i = 0; i < n; i++)
	    conv1(data, out, i, nc);
    }
}

void APlayer::process(short *data, size_t n)
{
    m_ctx->play(data, n);
}

void APlayer::process(short **data, size_t n)
{
    size_t nc = channels();
    short *out = (short *) m_ctx->buf(n * 2 * nc);
    convert(data, out, n, nc);
    m_ctx->play(out, n);
}

void APlayer::process(float *data, size_t n)
{
    size_t nc = channels();
    short *out = (short *) m_ctx->buf(n * 2 * nc);
    convert(data, out, n, nc);
    m_ctx->play(out, n);
}

void APlayer::process(float **data, size_t n)
{
    size_t nc = channels();
    short *out = (short *) m_ctx->buf(n * 2 * nc);
    convert(data, out, n, nc);
    m_ctx->play(out, n);
}
