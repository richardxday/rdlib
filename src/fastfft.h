#ifndef __FAST_FFT__
#define __FAST_FFT__

#include "fastcomplex.h"

void preparefft(uint_t n);

void fft(const scomplex *in, scomplex *out, uint_t n);
void ifft(const scomplex *in, scomplex *out, uint_t n);
void fft(const dcomplex *in, dcomplex *out, uint_t n);
void ifft(const dcomplex *in, dcomplex *out, uint_t n);
void fft(const icomplex *in, icomplex *out, uint_t n);
void ifft(const icomplex *in, icomplex *out, uint_t n);

#endif

