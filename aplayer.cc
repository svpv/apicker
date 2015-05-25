#include <iostream>
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
private:
    int m_driver;
    ao_sample_format m_fmt;
    ao_device *m_dev;
};

APlayer::Ctx::Ctx(int channels, int rate)
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
}

void APlayer::Ctx::play(short *data, size_t n)
{
    size_t c = m_fmt.channels;
    ao_play(m_dev, (char *) data, n * 2 * c);
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

void APlayer::process(short *data, size_t n)
{
    m_ctx->play(data, n);
}

void APlayer::process(float *data, size_t n)
{
    m_ctx->play(convert(data, n), n);
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
