#ifndef WAVEFORM_H
#define WAVEFORM_H

/*
 * Simple waveform: audio level each 10 ms.
 */

#include <stddef.h>

class Waveform
{
public:
    Waveform(const char *fname);
    ~Waveform();
    size_t m_n;
    unsigned char *m_v;
};

#endif
