#ifndef FFTS_H
#define FFTS_H

#include <math.h>
#include <complex.h>

#include "stuff.h"

// Converts to frequencies before Nyquist limit
// Very slow for high sample rates
void dft(double *in, double complex *out, uint samples, uint sampleRate);

void fft(double complex *in, double complex* out, int n);

#endif //FFTS_H
