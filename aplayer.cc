#include <iostream>
#include <cmath>
#include <cstring>
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
    unsigned m_csec;
private:
    int m_driver;
    ao_sample_format m_fmt;
    ao_device *m_dev;
    void *m_buf;
};

APlayer::Ctx::Ctx(int channels, int rate)
    : m_buf(NULL)
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
    ap->seek(ap->m_ctx->m_csec);
    int percent;
    do
	percent = ap->readsome();
    while (percent < 100 && !ap->m_ctx->m_stop);
    return NULL;
}

void start_bg_thread(APlayer *ap)
{
    pthread_t bg_thread;
    if (pthread_create(&bg_thread, NULL, bg_play_routine, ap) != 0)
	throw "cannot create player thread";
}

void APlayer::play_bg(unsigned csec)
{
    m_ctx->m_stop = false;
    m_ctx->m_csec = csec;
    start_bg_thread(this);
}

void APlayer::stop_bg()
{
    m_ctx->m_stop = true;
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
