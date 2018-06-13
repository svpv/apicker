#include <stdint.h>
#include <assert.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
}
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
    void seek(unsigned csec);
    unsigned read1()
    {
	m_read_n = read(&m_read_data);
	if (m_read_n == 0)
	    m_pos = AEOF;
	return m_pos;
    }
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
public:
    unsigned m_pos;
    void *m_read_data;
    size_t m_read_n;
};

AReader::Ctx::Ctx(const char *fname)
    : m_buf1(NULL), m_buf2(NULL), m_pos(0)
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
    int rc = avcodec_receive_frame(m_coctx, m_frame);
    while (rc == AVERROR(EAGAIN)) {
	int xx = av_read_frame(m_format, &m_packet);
	if (xx == AVERROR_EOF)
	    xx = avcodec_send_packet(m_coctx, NULL);
	else if (xx < 0)
	    throw "cannot decode audio";
	else if (m_packet.stream_index != (int) m_stream) {
	    av_packet_unref(&m_packet);
	    continue;
	}
	else {
	    xx = avcodec_send_packet(m_coctx, &m_packet);
	    av_packet_unref(&m_packet);
	}
	if (xx == AVERROR_EOF)
	    return 0;
	if (xx < 0)
	    throw "cannot decode audio";
	rc = avcodec_receive_frame(m_coctx, m_frame);
    }
    if (rc == AVERROR_EOF)
	return 0;
    if (rc < 0) {
	if (m_codec->id == AV_CODEC_ID_MP3)
	    return 0; /* probably id3sync issues at eof */
	throw "cannot decode audio";
    }
    m_pos = av_rescale_q(m_frame->pts,
	    m_format->streams[m_stream]->time_base, (AVRational){1,100});
    *datap = m_frame->extended_data;
    assert(m_frame->nb_samples > 0);
    return m_frame->nb_samples;
}

void AReader::Ctx::seek(unsigned csec)
{
    int64_t pos = av_rescale_q(csec, (AVRational){1,100},
	    m_format->streams[m_stream]->time_base);
    if (av_seek_frame(m_format, m_stream, pos, 0) < 0)
	throw "cannot seek audo stream";
    m_pos = csec;
}

AReader::Ctx::~Ctx()
{
    av_frame_free(&m_frame);
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

unsigned AReader::read1()
{
    return m_ctx->read1();
}

void AReader::process1()
{
    switch (m_ctx->fmt()) {
    case AV_SAMPLE_FMT_S16:
	process(*(short **) m_ctx->m_read_data, m_ctx->m_read_n);
	break;
    case AV_SAMPLE_FMT_S16P:
	process( (short **) m_ctx->m_read_data, m_ctx->m_read_n);
	break;
    case AV_SAMPLE_FMT_FLT:
	process(*(float **) m_ctx->m_read_data, m_ctx->m_read_n);
	break;
    case AV_SAMPLE_FMT_FLTP:
	process( (float **) m_ctx->m_read_data, m_ctx->m_read_n);
	break;
    default:
	throw "unsupported audio sample format";
    }
}

void AReader::seek(unsigned csec)
{
    m_ctx->seek(csec);
}

unsigned AReader::getpos()
{
    return m_ctx->m_pos;
}

#include <stdio.h>
#include <unistd.h>

void AReader::loop()
{
    unsigned pos;
    while ((pos = read1()) != AEOF)
	process1();
}
