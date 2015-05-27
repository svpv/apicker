#include <cstddef>
#include <cstring>
#include <cmath>
class HiPass
{
public:
    HiPass(int channels, int srate, int hz)
	: m_nc(channels), m_once(false)
    {
	double RC = 1 / (2 * M_PI * hz);
	m_a = RC / (RC + 1.0 / srate);
	memset(m_x, 0, sizeof(m_x));
	memset(m_y, 0, sizeof(m_y));
    }
    void hipass(short *data, size_t n)
    {
	if (!m_once && n > 0) {
	    float *x = m_x;
	    float *y = m_y;
	    short *end1 = data + m_nc;
	    do {
		*x++ = *y++ = *data++;
	    } while (data < end1);
	    m_once = true;
	}
	short *end = data + n * m_nc;
	while (data < end) {
	    float *x = m_x;
	    float *y = m_y;
	    short *end1 = data + m_nc;
	    do {
		float x0 = *x;
		float y0 = *y;
		float x1 = *data;
		float y1 = m_a * (y0 + x1 - x0);
		*data++ = rintf(y1);
		*x++ = x1;
		*y++ = y1;
	    } while (data < end1);
	}
    }
private:
    int m_nc;
    float m_a;
    float m_x[8];
    float m_y[8];
    bool m_once;
};
