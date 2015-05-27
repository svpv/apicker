#ifndef SAMPLE_H
#define SAMPLE_H

#include <cmath>

namespace Sample
{
    template<class T>
    static inline
    T resample(T sample, const T *ArgRange, const T *RetRange)
    {
	(void) ArgRange; (void) RetRange;
	return sample;
    }

    static inline
    float resample(float sample, const short *ArgRange, const short *RetRange)
    {
	(void) ArgRange; (void) RetRange;
	return rintf(sample);
    }

    static inline
    float resample(float sample, const short *ArgRange, const float *RetRange)
    {
	(void) ArgRange; (void) RetRange;
	return sample / 32767.0f;
    }

    static inline
    float resample(float sample, const float *ArgRange, const short *RetRange)
    {
	(void) ArgRange; (void) RetRange;
	return rint(sample * 32767.0f);
    }
}

#endif
