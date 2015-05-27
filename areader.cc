#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include "areader.h"

class LibAV
{
public:
    LibAV()
    {
	av_register_all();
	avcodec_register_all();
    }
};

static LibAV libav;

class AReader::Ctx
{
public:
    Ctx(const char *filename);
    ~Ctx();
    size_t read(void **datap);
    enum AVSampleFormat fmt()
    {
	return m_coctx->sample_fmt;
    }
    int channels()
    {
	return m_coctx->channels;
    }
    int rate()
    {
	return m_coctx->sample_rate;
    }
protected:
    AVFormatContext *m_format;
    AVCodecContext *m_coctx;
    AVCodec *m_codec;
    AVPacket m_packet;
    AVFrame *m_frame;
    unsigned m_stream;
    short *m_buf1;
    float *m_buf2;
};

AReader::Ctx::Ctx(const char *fname)
    : m_buf1(NULL), m_buf2(NULL)
{
    if (fname == NULL)
	throw "null filename";

    m_format = NULL;
    if (avformat_open_input(&m_format, fname, NULL, NULL) < 0)
	throw "cannot open media file";

    if (avformat_find_stream_info(m_format, NULL) < 0)
	throw "cannot find stream info";

    for (m_stream = 0; m_stream < m_format->nb_streams; m_stream++)
	if (m_format->streams[m_stream]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
	    break;
    if (m_stream == m_format->nb_streams)
	throw "cannot find audio stream";

    m_coctx = m_format->streams[m_stream]->codec;
    m_codec = avcodec_find_decoder(m_coctx->codec_id);
    if (m_codec == NULL)
	throw "cannot find audio decoder";

    if (avcodec_open2(m_coctx, m_codec, NULL) < 0)
	throw "cannot open audio decoder";

    av_init_packet(&m_packet);
    m_frame = av_frame_alloc();
}

size_t AReader::Ctx::read(void **datap)
{
    while (av_read_frame(m_format, &m_packet) >= 0)
    {
	if (m_packet.stream_index != (int) m_stream)
	    continue;
	int finished = 0;
	int len = avcodec_decode_audio4(m_coctx, m_frame, &finished, &m_packet);
	if (len < 0)
	    throw "cannot decode audio";
	if (finished) {
	    *datap = m_frame->extended_data;
	    return m_frame->nb_samples;
	}
    }
    return 0;
}

AReader::Ctx::~Ctx()
{
    free(m_buf1);
    free(m_buf2);
}

AReader::AReader(const char *fname)
{
    m_ctx = new Ctx(fname);
}

AReader::~AReader()
{
    delete m_ctx;
}

int AReader::channels()
{
    return m_ctx->channels();
}

int AReader::rate()
{
    return m_ctx->rate();
}

int AReader::readsome()
{
    void *data;
    size_t n = m_ctx->read(&data);
    if (n == 0)
	return 100;
    switch (m_ctx->fmt()) {
    case AV_SAMPLE_FMT_S16:
	process(*(short **) data, n);
	break;
    case AV_SAMPLE_FMT_S16P:
	process( (short **) data, n);
	break;
    case AV_SAMPLE_FMT_FLT:
	process(*(float **) data, n);
	break;
    case AV_SAMPLE_FMT_FLTP:
	process( (float **) data, n);
	break;
    default:
	throw "unsupported audio sample format";
    }
    return 1;
}

#include <stdio.h>
#include <unistd.h>

void AReader::loop()
{
    int percent;
    int tty = isatty(2);
    const char *sep = "";
    do {
	percent = readsome();
	if (tty) {
	    fprintf(stderr, "%s%d%%", sep, percent);
	    sep = "\r";
	}
	else if ((percent % 10) == 0) {
	    fprintf(stderr, "%s%d%%", sep, percent);
	    sep = " ";
	}
    }
    while (percent < 100);
    putc('\n', stderr);
}
