#ifndef HIPASS_H
#define HIPASS_H

#include <cstddef>

class HiPass
{
public:
    HiPass(int nc, int srate, int hz);
    ~HiPass();
    // inplace
    void hipass(short  *data, size_t n);
    void hipass(float  *data, size_t n);
    void hipass(short **data, size_t n);
    void hipass(float **data, size_t n);
    // interleaved
    void hipass(const short *data, size_t n, short *out);
    void hipass(const short *data, size_t n, float *out);
    void hipass(const float *data, size_t n, short *out);
    void hipass(const float *data, size_t n, float *out);
    // planar
    void hipass(const short *const *data, size_t n, short *out);
    void hipass(const short *const *data, size_t n, float *out);
    void hipass(const float *const *data, size_t n, short *out);
    void hipass(const float *const *data, size_t n, float *out);
private:
    class Ctx;
    Ctx *m_ctx;
};

#endif
