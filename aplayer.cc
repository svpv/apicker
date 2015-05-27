#include <iostream>
#include <cmath>
#include <ao/ao.h>
#include "aplayer.h"
#include "hipass.h"

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
private:
    int m_driver;
    ao_sample_format m_fmt;
    ao_device *m_dev;
    void *m_buf;
    HiPass m_hipass;
};

APlayer::Ctx::Ctx(int channels, int rate)
    : m_buf(NULL), m_hipass(channels, rate, 200)
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
    m_hipass.hipass(data, n);
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

int main(int argc, char *argv[])
{
    try {
	APlayer player(argv[1]);
	player.loop();
    }
    catch (const std::exception &e) {
	std::cerr << e.what() << std::endl;
	return 1;
    }
    catch (const char *s) {
	std::cerr << s << std::endl;
	return 1;
    }
    return 0;
}
