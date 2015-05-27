#include <cstring>
#include <cmath>
#include "hipass.h"
#include "sample.h"

using Sample::resample;

class HiPass::Ctx
{
public:
    Ctx(int nc, int srate, int hz)
	: m_nc(nc), m_once(false)
    {
	if (nc >= 8)
	    throw "two many audio channels";
	double RC = 1 / (2 * M_PI * hz);
	m_a = RC / (RC + 1.0 / srate);
	memset(m_x, 0, sizeof(m_x));
	memset(m_y, 0, sizeof(m_y));
    }
private:
    size_t m_nc;
    float m_a;
    float m_x[8];
    float m_y[8];
    bool m_once;

    template<typename T>
    void hipass1(T *data, size_t n, size_t j)
    {
	const T *end = data + n;
	if (!m_once && data < end) {
	    m_x[j] = m_y[j] = *data++;
	    m_once = true;
	}
	float x0 = m_x[j];
	float y0 = m_y[j];
	while (data < end) {
	    float x1 = *data;
	    float y1 = m_a * (y0 + x1 - x0);
	    *data = resample(y1, data, data);
	    data++;
	    x0 = x1;
	    y0 = y1;
	}
	m_x[j] = x0;
	m_y[j] = y0;
    }

    template<typename T, typename RetT>
    void hipass1(const T *data, size_t n, size_t j, RetT *out, size_t ostep)
    {
	const T *end = data + n;
	if (!m_once && data < end) {
	    m_x[j] = m_y[j] = *data++;
	    m_once = true;
	}
	float x0 = m_x[j];
	float y0 = m_y[j];
	while (data < end) {
	    float x1 = *data;
	    float y1 = m_a * (y0 + x1 - x0);
	    *out = resample(y1, data, out);
	    data++;
	    out += ostep;
	    x0 = x1;
	    y0 = y1;
	}
	m_x[j] = x0;
	m_y[j] = y0;
    }

    template<typename T>
    void hipass2(T *data, size_t n)
    {
	const T *end = data + n * 2;
	if (!m_once && data < end) {
	    m_x[0] = m_y[0] = *data++;
	    m_x[1] = m_y[1] = *data++;
	    m_once = true;
	}
	float x0a = m_x[0];
	float x0b = m_x[1];
	float y0a = m_y[0];
	float y0b = m_y[1];
	while (data < end) {
	    float x1a = data[0];
	    float x1b = data[1];
	    float y1a = m_a * (y0a + x1a - x0a);
	    float y1b = m_a * (y0b + x1b - x0b);
	    *data = resample(y1a, data, data);
	    data++;
	    *data = resample(y1b, data, data);
	    data++;
	    x0a = x1a;
	    x0b = x1b;
	    y0a = y1a;
	    y0b = y1b;
	}
	m_x[0] = x0a;
	m_x[1] = x0b;
	m_y[0] = y0a;
	m_y[1] = y0b;
    }

    template<typename T, typename RetT>
    void hipass2(const T *data, size_t n, RetT *out)
    {
	const T *end = data + n * 2;
	if (!m_once && data < end) {
	    m_x[0] = m_y[0] = *data++;
	    m_x[1] = m_y[1] = *data++;
	    m_once = true;
	}
	float x0a = m_x[0];
	float x0b = m_x[1];
	float y0a = m_y[0];
	float y0b = m_y[1];
	while (data < end) {
	    float x1a = data[0];
	    float x1b = data[1];
	    float y1a = m_a * (y0a + x1a - x0a);
	    float y1b = m_a * (y0b + x1b - x0b);
	    *out = resample(y1a, data, out);
	    out++;
	    *out = resample(y1b, data, out);
	    out++;
	    data += 2;
	    x0a = x1a;
	    x0b = x1b;
	    y0a = y1a;
	    y0b = y1b;
	}
	m_x[0] = x0a;
	m_x[1] = x0b;
	m_y[0] = y0a;
	m_y[1] = y0b;
    }

    template<typename T>
    void hipassn(T *data, size_t n)
    {
	const T *end = data + n * m_nc;
	if (!m_once && data < end) {
	    float *x = m_x;
	    float *y = m_y;
	    const T *end1 = data + m_nc;
	    do {
		*x++ = *y++ = *data++;
	    } while (data < end1);
	    m_once = true;
	}
	while (data < end) {
	    float *x = m_x;
	    float *y = m_y;
	    const T *end1 = data + m_nc;
	    do {
		float x0 = *x;
		float y0 = *y;
		float x1 = *data;
		float y1 = m_a * (y0 + x1 - x0);
		*data = resample(y1, data, data);
		data++;
		*x++ = x1;
		*y++ = y1;
	    } while (data < end1);
	}
    }

    template<typename T, typename RetT>
    void hipassn(const T *data, size_t n, RetT *out)
    {
	const T *end = data + n * m_nc;
	if (!m_once && data < end) {
	    float *x = m_x;
	    float *y = m_y;
	    const T *end1 = data + m_nc;
	    do {
		*x++ = *y++ = *data++;
	    } while (data < end1);
	    m_once = true;
	}
	while (data < end) {
	    float *x = m_x;
	    float *y = m_y;
	    const T *end1 = data + m_nc;
	    do {
		float x0 = *x;
		float y0 = *y;
		float x1 = *data;
		float y1 = m_a * (y0 + x1 - x0);
		*out = resample(y1, data, out);
		data++;
		out++;
		*x++ = x1;
		*y++ = y1;
	    } while (data < end1);
	}
    }
public:
    template<typename T>
    void hipass(T *data, size_t n)
    {
	switch (m_nc)
	{
	case 0:
	    // cannot happen, fall through
	case 1:
	    hipass1(data, n, 0);
	    break;
	case 2:
	    hipass2(data, n);
	    break;
	default:
	    hipassn(data, n);
	}
    }

    template<typename T, typename RetT>
    void hipass(const T *data, size_t n, RetT *out)
    {
	switch (m_nc)
	{
	case 0:
	    // cannot happen, fall through
	case 1:
	    hipass1(data, n, 0, out, 1);
	    break;
	case 2:
	    hipass2(data, n, out);
	    break;
	default:
	    hipassn(data, n, out);
	}
    }

    template<typename T>
    void hipass(T **data, size_t n)
    {
	for (size_t j = 0; j < m_nc; j++)
	    hipass1(data[j], n, j);
    }

    template<typename T, typename RetT>
    void hipass(const T *const *data, size_t n, RetT *out)
    {
	for (size_t j = 0; j < m_nc; j++)
	    hipass1(data[j], n, j, out, 1);
    }
};

HiPass::HiPass(int nc, int srate, int hz)
{
    m_ctx = new Ctx(nc, srate, hz);
}

HiPass::~HiPass()
{
    delete m_ctx;
}

// inplace
void HiPass::hipass(short  *data, size_t n) { m_ctx->hipass(data, n); }
void HiPass::hipass(float  *data, size_t n) { m_ctx->hipass(data, n); }
void HiPass::hipass(short **data, size_t n) { m_ctx->hipass(data, n); }
void HiPass::hipass(float **data, size_t n) { m_ctx->hipass(data, n); }
// interleaved
void HiPass::hipass(const short *data, size_t n, short *out) { m_ctx->hipass(data, n, out); }
void HiPass::hipass(const short *data, size_t n, float *out) { m_ctx->hipass(data, n, out); }
void HiPass::hipass(const float *data, size_t n, short *out) { m_ctx->hipass(data, n, out); }
void HiPass::hipass(const float *data, size_t n, float *out) { m_ctx->hipass(data, n, out); }
// planar
void HiPass::hipass(const short *const *data, size_t n, short *out) { m_ctx->hipass(data, n, out); }
void HiPass::hipass(const short *const *data, size_t n, float *out) { m_ctx->hipass(data, n, out); }
void HiPass::hipass(const float *const *data, size_t n, short *out) { m_ctx->hipass(data, n, out); }
void HiPass::hipass(const float *const *data, size_t n, float *out) { m_ctx->hipass(data, n, out); }
