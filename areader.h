#ifndef AREADER_H
#define AREADER_H
#include <stddef.h>
class AReader
{
public:
    AReader(const char *fname);
    ~AReader();
    int readsome();
    void loop();
    int rate();
    int channels();
protected:
    virtual void process(short *data, size_t n) = 0; // s16
    virtual void process(float *data, size_t n) = 0; // flt
    short *convert(float *data, size_t n); // flt -> s16
private:
    class Ctx;
    Ctx *m_ctx;
};
#endif