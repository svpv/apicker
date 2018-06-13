/*
 * Generate waveform file.
 */
#include <cstdio>
#include <cassert>
#include <algorithm>
#include "rms.h"
#include "hipass.h"

static inline double scale_to_dB(double scale)
{
    if (scale < 0.001)
	return -60;
    return 20 * log10(scale);
}

static inline double dB_to_scale(double dB)
{
    return pow(10, 0.05 * dB);
}

class WaveformGen: public RMS, protected HiPass
{
public:
    WaveformGen(const char *fname)
	: RMS(fname), HiPass(channels(), rate(), 100)
    {
    }
    void save(FILE *fp)
    {
	fwrite("WF1", 4, 1, fp);
	fwrite("\0\0\0", 4, 1, fp);

	std::vector<double> const& r = rms();
	std::vector<double> s = r;
	std::sort(s.begin(), s.end());
	assert(s.size() > 0);

	// Approximates ReplayGain, realistic loudness.
	double q95 = s[s.size() * 95 / 100];
	double gain_dB = -17 - scale_to_dB(q95);
	double gain_mul = dB_to_scale(gain_dB);

	// But also want to stretch to full height.
	double q999 = s[s.size() * 999 / 1000];
	double stretch_mul = q999 > 1/9 ? 1 / q999 : 9;

	for (double v : r) {
	    double dB = scale_to_dB(v * gain_mul);
	    v *= sqrt(gain_mul * stretch_mul);
	    // Mixed level: each dB, starting with -60 dB, takes one pixel.
	    // This provides much better resolution for soft sounds, since
	    // with 256 levels, the lowest linear level is only about -48 dB.
	    int c = dB + 60.3 + v * 200;
	    assert(c >= 0);
	    if (c > 255)
		c = 255;
	    putc(c, fp);
	}
	fflush(fp);
    }
    void save(const char *fname)
    {
	if (fname == NULL)
	    throw "null filename";
	FILE *fp = fopen(fname, "w");
	if (fp == NULL)
	    throw "cannot open file for writing";
	save(fp);
    }
private:
    template<typename T>
    void do_process(T data, size_t n)
    {
	hipass(data, n);
	RMS::process(data, n);
    }
protected:
    void process(short  *data, size_t n) override { do_process(data, n); }
    void process(short **data, size_t n) override { do_process(data, n); }
    void process(float  *data, size_t n) override { do_process(data, n); }
    void process(float **data, size_t n) override { do_process(data, n); }
};
