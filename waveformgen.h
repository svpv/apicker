/*
 * Generate waveform file.
 */
#include <cstdio>
#include <cassert>
#include <algorithm>
#include "rms.h"
#include "hipass.h"

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
	double q999 = s[s.size() * 999 / 1000];

	for (double v : r) {
	    if (v > q999)
		v = q999;
	    int c = v / q999 * 255 + 0.5;
	    assert(c >= 0);
	    assert(c <= 255);
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
