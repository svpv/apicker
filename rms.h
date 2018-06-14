#include <cmath>
#include <vector>
#include "areader.h"
class RMS: public AReader
{
public:
    RMS(const char *fname)
	: AReader(fname), m_sumsq(0), m_nsum(0)
    {
	m_nmax = rate() / 100; // 10 msec
    }
private:
    template<typename T>
    inline double sumsq(T  *data, size_t i, size_t nc)
    {
	double s = 0;
	for (size_t j = 0; j < nc; j++)
	    s += data[i * nc + j] *
		 data[i * nc + j] ;
	return s;
    }
    template<typename T>
    inline double sumsq(T **data, size_t i, size_t nc)
    {
	double s = 0;
	for (size_t j = 0; j < nc; j++)
	    s += data[j][i] *
		 data[j][i] ;
	return s;
    }
    double range(short  *data) { return 32767; }
    double range(short **data) { return 32767; }
    double range(float  *data) { return 1; }
    double range(float **data) { return 1; }

    double m_sumsq;
    size_t m_nsum;
    size_t m_nmax;
    std::vector<double> m_rms;

    template<typename T>
    void do_process(T data, size_t n, size_t nc)
    {
	for (size_t i = 0; i < n; i++) {
	    m_sumsq += sumsq(data, i, nc);
	    m_nsum++;
	    if (m_nsum < m_nmax)
		continue;
	    double rms = sqrt(m_sumsq / (m_nsum * range(data) * range(data)));
	    m_rms.push_back(rms);
	    m_sumsq = 0;
	    m_nsum = 0;
	}
    }

    template<typename T>
    void do_process(T data, size_t n)
    {
	int nc = channels();
	switch (nc) {
	case 1:
	    do_process(data, n, 1);
	    break;
	case 2:
	    do_process(data, n, 2);
	    break;
	default:
	    do_process(data, n, nc);
	}
    }
protected:
    void process(short  *data, size_t n) override { do_process(data, n); }
    void process(short **data, size_t n) override { do_process(data, n); }
    void process(float  *data, size_t n) override { do_process(data, n); }
    void process(float **data, size_t n) override { do_process(data, n); }
public:
    std::vector<double> &rms()
    {
	return m_rms;
    }
};
