#include "dfts.h"

void dft(double *in, double complex *out, uint samples, uint sampleRate) {
    for (uint f = 0; f < samples; f++) {
        out[f] = 0;
        for (uint sample = 0; sample < samples; sample++) {
            double t = (double) sample / sampleRate / 2;
            double scaler = (double) 1 / samples;
            out[f] += in[sample % samples] * cexp(-2 * I * M_PI * f * t) * scaler;
        }
    }
}

/* Everything else here was not my own, but derived from others' work */
/* e.g. Wikipedia */
void _fft(double complex *in, double complex *out, int n, int step) {
    if (n == 1) {
        out[0] = in[0];
        return;
    }

    const uint half_n = (uint) n / 2;

    _fft(in       , out         , half_n, step * 2);
    _fft(in + step, out + half_n, half_n, step * 2);

    for (uint i = 0; i < half_n; i++) {
        double complex v = cexpl(-2 * M_PI * I * i / n) * out[i + half_n]; 
        double complex e = out[i];
        out[i]          = e + v;
        out[i + half_n] = e - v;
    }
}

void fft(double complex *in_buf, double complex *out_buf, int n) {
    _fft(in_buf, out_buf, n, 1);

#if UNMIRROR_FFT
    for (uint i = 0; i < n / 2; i++) {
        out_buf[i] += out_buf[n - 1 - i];
    }
    for (uint i = n / 2; i < n; i++) {
        out_buf[i] = 0;
    }
#endif

    // Convert out to magnitudes
    // for (uint i = 0; i < n; i++) {
    //     out_buf[i] = cabs(out_buf[i]);
    // }
}

