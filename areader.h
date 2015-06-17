#ifndef AREADER_H
#define AREADER_H
#include <stddef.h>
#include <limits.h>
class AReader
{
public:
    AReader(const char *fname);
    ~AReader();

    static const unsigned AEOF = UINT_MAX;
    unsigned read1();
    void process1();

    void seek(unsigned csec);
    unsigned getpos();
    void loop();
    int rate();
    int channels();
protected:
    virtual void process(short  *data, size_t n) = 0; // s16
    virtual void process(short **data, size_t n) = 0; // s16p
    virtual void process(float  *data, size_t n) = 0; // flt
    virtual void process(float **data, size_t n) = 0; // fltp
private:
    class Ctx;
    Ctx *m_ctx;
};
#endif
