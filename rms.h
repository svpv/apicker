#include <cmath>
#include <vector>
#include "areader.h"
class RMS: public AReader
{
public:
    RMS(const char *fname)
	: AReader(fname), m_sum(0), m_nsum(0)
    {
	m_nmax = rate() / 100; // 10 msec
    }
private:
    template<typename T>
    inline double chsum(T  *data, size_t i, size_t nc)
    {
	double s = data[i * nc];
	for (size_t j = 1; j < nc; j++)
	    s += data[i * nc + j];
	return s;
    }
    template<typename T>
    inline double chsum(T **data, size_t i, size_t nc)
    {
	double s = data[0][i];
	for (size_t j = 1; j < nc; j++)
	    s += data[j][i];
	return s;
    }
    inline double avg(short  *data, size_t i, size_t nc) { return chsum(data, i, nc) / (nc * 32767); }
    inline double avg(short **data, size_t i, size_t nc) { return chsum(data, i, nc) / (nc * 32767); }
    inline double avg(float  *data, size_t i, size_t nc) { return chsum(data, i, nc) / nc; }
    inline double avg(float **data, size_t i, size_t nc) { return chsum(data, i, nc) / nc; }

    double m_sum;
    size_t m_nsum;
    size_t m_nmax;
    std::vector<double> m_rms;

    inline void acc(double avg)
    {
	m_sum += avg * avg;
	m_nsum++;
	if (m_nsum < m_nmax)
	    return;

	double rms = sqrt(m_sum / m_nmax);
	m_rms.push_back(rms);
	m_sum = 0;
	m_nsum = 0;
    }

    template<typename T>
    void do_process(T data, size_t n, size_t nc)
    {
	for (size_t i = 0; i < n; i++)
	    acc(avg(data, i, nc));
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
	    throw "unsupported number of channels";
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
