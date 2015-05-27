#ifndef HIPASS_H
#define HIPASS_H

#include <cstddef>

class HiPass
{
public:
    HiPass(int nc, int srate, int hz);
    ~HiPass();
    void hipass(short *data, size_t n);
    void hipass(const short *data, size_t n, short *out);
    void hipass(const short *data, size_t n, float *out);
private:
    class Ctx;
    Ctx *m_ctx;
};

#endif
