#ifndef APLAYER_H
#define APLAYER_H

#include "areader.h"

class APlayer: public AReader
{
public:
    APlayer(const char *fname);
    ~APlayer();
protected:
    void process(short  *data, size_t n) override;
    void process(short **data, size_t n) override;
    void process(float  *data, size_t n) override;
    void process(float **data, size_t n) override;
private:
    class Ctx;
    Ctx *m_ctx;
};

#endif
