
#include <math.h>

#include "fastfft.h"
#include "DataList.h"

static ADataList stwiddlelist;
static ADataList dtwiddlelist;
static ADataList itwiddlelist;
static ADataList bitrevlist;

static void __delete_stwiddle(uptr_t item, void *context)
{
    UNUSED(context);

    if (item) {
        delete[] (scomplex *)item;
    }
}

static void __delete_dtwiddle(uptr_t item, void *context)
{
    UNUSED(context);

    if (item) {
        delete[] (dcomplex *)item;
    }
}

static void __delete_itwiddle(uptr_t item, void *context)
{
    UNUSED(context);

    if (item) {
        delete[] (icomplex *)item;
    }
}

static void __delete_bitrev(uptr_t item, void *context)
{
    UNUSED(context);

    if (item) {
        delete[] (uint_t *)item;
    }
}

static const scomplex *getstwiddle(uint_t n)
{
    stwiddlelist.SetDestructor(&__delete_stwiddle);

    if (!stwiddlelist[n]) {
        scomplex *p;
        uint_t n2 = 1 << n;

        if ((p = new scomplex[n2]) != NULL) {
            uint_t i;

            for (i = 0; i < n2; i++) {
                p[i] = comp_single(comp_exp(-(double)i / (double)n2 * 2.0 * M_PI));
            }

            stwiddlelist.Replace(n, p);
        }
    }

    return (const scomplex *)stwiddlelist[n];
}

static const dcomplex *getdtwiddle(uint_t n)
{
    dtwiddlelist.SetDestructor(&__delete_dtwiddle);

    if (!dtwiddlelist[n]) {
        dcomplex *p;
        uint_t n2 = 1 << n;

        if ((p = new dcomplex[n2]) != NULL) {
            uint_t i;

            for (i = 0; i < n2; i++) {
                p[i] = comp_exp(-(double)i / (double)n2 * 2.0 * M_PI);
            }

            dtwiddlelist.Replace(n, p);
        }
    }

    return (const dcomplex *)dtwiddlelist[n];
}

static const icomplex *getitwiddle(uint_t n)
{
    itwiddlelist.SetDestructor(&__delete_itwiddle);

    if (!itwiddlelist[n]) {
        icomplex *p;
        uint_t n2 = 1 << n;

        if ((p = new icomplex[n2]) != NULL) {
            uint_t i;

            for (i = 0; i < n2; i++) {
                dcomplex d = comp_exp(-(double)i / (double)n2 * 2.0 * M_PI);
                p[i].re = (sint16_t)LIMIT(32768.0 * d.re, -32768.0, 32767.0);
                p[i].im = (sint16_t)LIMIT(32768.0 * d.im, -32768.0, 32767.0);
            }

            itwiddlelist.Replace(n, p);
        }
    }

    return (const icomplex *)itwiddlelist[n];
}

static const uint_t *getbitrev(uint_t n)
{
    bitrevlist.SetDestructor(&__delete_bitrev);

    if (!bitrevlist[n]) {
        uint_t *p, n2 = 1 << n;

        if ((p = new uint_t[n2]) != NULL) {
            uint_t i, j;

            for (i = 0; i < n2; i++) {
                uint_t val = 0;

                for (j = 0; j < n; j++) {
                    if (i & (1 << j)) val |= 1 << (n - 1 - j);
                }

                p[i] = val;
            }

            bitrevlist.Replace(n, p);
        }
    }

    return (const uint_t *)bitrevlist[n];
}

void preparefft(uint_t n)
{
    (void)getstwiddle(n);
    (void)getdtwiddle(n);
    (void)getitwiddle(n);
    (void)getbitrev(n);
}

void fft_1ststage(const scomplex *in, scomplex *out, uint_t n)
{
    const uint_t * const bitrev = getbitrev(n);
    const uint_t *brp, n2 = 1 << n;
    const float mul = 1.f / (float)n2;
    scomplex a, b, *outp1, *outp2, *outp1e1;

    // first stage requires no twiddling
    // but we can carry out the bit-reversing inline
    brp   = bitrev;

    outp1 = out;
    outp2 = outp1 + 1;
    outp1e1 = outp1 + n2;   // set end pointer for outp1
    do {
        comp_mul(a, in[*brp], mul); brp++;
        comp_mul(b, in[*brp], mul); brp++;

        comp_add_sub(*outp1, *outp2, a, b);
        outp1 += 2; outp2 += 2;
    }
    while (outp1 < outp1e1);
}

void fft_1ststage_swap(const scomplex *in, scomplex *out, uint_t n)
{
    const uint_t * const bitrev = getbitrev(n);
    const uint_t *brp, n2 = 1 << n;
    scomplex a, b, *outp1, *outp2, *outp1e1;

    // first stage requires no twiddling
    // but we can carry out the bit-reversing inline
    brp   = bitrev;

    outp1 = out;
    outp2 = outp1 + 1;
    outp1e1 = outp1 + n2;   // set end pointer for outp1
    do {
        a = comp_swap(in[*brp]); brp++;
        b = comp_swap(in[*brp]); brp++;

        comp_add_sub(*outp1, *outp2, a, b);
        outp1 += 2; outp2 += 2;
    }
    while (outp1 < outp1e1);
}

void fft_2ndstage(const scomplex *in, scomplex *out, uint_t n)
{
    const uint_t n2 = 1 << n;
    scomplex b, *outp1, *outp2, *outp1e1;

    UNUSED(in);

    // second stage requires 0 and 180 degress twiddles
    outp1 = out;
    outp2 = outp1 + 2;
    outp1e1 = outp1 + n2;   // set end pointer for outp1
    do {
        comp_add_sub(*outp1, *outp2);
        outp1++; outp2++;

        b = comp_rotate90(*outp2);

        comp_add_sub(*outp1, *outp2, *outp1, b);

        outp1 += 3;
        outp2 += 3;
    }
    while (outp1 < outp1e1);
}

void fft_finalstages(const scomplex *in, scomplex *out, uint_t n)
{
    const scomplex * const twiddle = getstwiddle(n);
    const scomplex *wp;
    const uint_t n2 = 1 << n;
    scomplex c, *outp1, *outp2, *outp1e1, *outp1e2;
    uint_t i, count2, step;

    UNUSED(in);

    count2 = 4;

    // calculate step through sine table
    step   = 1 << (n - 3);

    // third and subsequent stages require full twiddling
    for (i = 2; i < n; i++) {
        outp1 = out;
        outp2 = outp1 + count2;
        outp1e2 = outp1 + n2;   // set end pointer for outp1 (outer loop)

        do {
            wp = twiddle + step;

            outp1e1 = outp1 + count2;   // set end pointer for outp1 (inner loop)

            comp_add_sub(*outp1, *outp2);
            outp1++; outp2++;

            do {
                comp_mul(c, *outp2, *wp);

                comp_add_sub(*outp1, *outp2, *outp1, c);
                outp1++; outp2++;

                wp += step;
            }
            while (outp1 < outp1e1);

            outp1 += count2;
            outp2 += count2;
        }
        while (outp1 < outp1e2);

        count2 <<= 1;
        step   >>= 1;
    }
}

void fft(const scomplex *in, scomplex *out, uint_t n)
{
    fft_1ststage(in, out, n);

    fft_2ndstage(in, out, n);

    fft_finalstages(in, out, n);
}

void ifft(const scomplex *in, scomplex *out, uint_t n)
{
    const uint_t n2 = 1 << n;
    uint_t i;

    fft_1ststage_swap(in, out, n);

    fft_2ndstage(in, out, n);

    fft_finalstages(in, out, n);

    for (i = 0; i < n2; i++) {
        out[i] = comp_swap(out[i]);
    }
}

/*----------------------------------------------------------------------------------------------------*/

void fft_1ststage(const dcomplex *in, dcomplex *out, uint_t n)
{
    const uint_t * const bitrev = getbitrev(n);
    const uint_t *brp, n2 = 1 << n;
    const double mul = 1.0 / (double)n2;
    dcomplex a, b, *outp1, *outp2, *outp1e1;

    // first stage requires no twiddling
    // but we can carry out the bit-reversing inline
    brp   = bitrev;

    outp1 = out;
    outp2 = outp1 + 1;
    outp1e1 = outp1 + n2;   // set end pointer for outp1
    do {
        comp_mul(a, in[*brp], mul); brp++;
        comp_mul(b, in[*brp], mul); brp++;

        comp_add_sub(*outp1, *outp2, a, b);
        outp1 += 2; outp2 += 2;
    }
    while (outp1 < outp1e1);
}

void fft_1ststage_swap(const dcomplex *in, dcomplex *out, uint_t n)
{
    const uint_t * const bitrev = getbitrev(n);
    const uint_t *brp, n2 = 1 << n;
    dcomplex a, b, *outp1, *outp2, *outp1e1;

    // first stage requires no twiddling
    // but we can carry out the bit-reversing inline
    brp   = bitrev;

    outp1 = out;
    outp2 = outp1 + 1;
    outp1e1 = outp1 + n2;   // set end pointer for outp1
    do {
        a = comp_swap(in[*brp]); brp++;
        b = comp_swap(in[*brp]); brp++;

        comp_add_sub(*outp1, *outp2, a, b);
        outp1 += 2; outp2 += 2;
    }
    while (outp1 < outp1e1);
}

void fft_2ndstage(const dcomplex *in, dcomplex *out, uint_t n)
{
    const uint_t n2 = 1 << n;
    dcomplex b, *outp1, *outp2, *outp1e1;

    UNUSED(in);

    // second stage requires 0 and 180 degress twiddles
    outp1 = out;
    outp2 = outp1 + 2;
    outp1e1 = outp1 + n2;   // set end pointer for outp1
    do {
        comp_add_sub(*outp1, *outp2);
        outp1++; outp2++;

        b = comp_rotate90(*outp2);

        comp_add_sub(*outp1, *outp2, *outp1, b);
        outp1++; outp2++;

        outp1 += 2;
        outp2 += 2;
    }
    while (outp1 < outp1e1);
}

void fft_finalstages(const dcomplex *in, dcomplex *out, uint_t n)
{
    const dcomplex * const twiddle = getdtwiddle(n);
    const dcomplex *wp;
    const uint_t n2 = 1 << n;
    dcomplex c, *outp1, *outp2, *outp1e1, *outp1e2;
    uint_t i, count2, step;

    UNUSED(in);

    count2 = 4;

    // calculate step through sine table
    step   = 1 << (n - 3);

    // third and subsequent stages require full twiddling
    for (i = 2; i < n; i++) {
        outp1 = out;
        outp2 = outp1 + count2;
        outp1e2 = outp1 + n2;   // set end pointer for outp1 (outer loop)

        do {
            wp = twiddle + step;

            outp1e1 = outp1 + count2;   // set end pointer for outp1 (inner loop)

            comp_add_sub(*outp1, *outp2);
            outp1++; outp2++;

            do {
                comp_mul(c, *outp2, *wp);

                comp_add_sub(*outp1, *outp2, *outp1, c);
                outp1++; outp2++;

                wp += step;
            }
            while (outp1 < outp1e1);

            outp1 += count2;
            outp2 += count2;
        }
        while (outp1 < outp1e2);

        count2 <<= 1;
        step   >>= 1;
    }
}

void fft(const dcomplex *in, dcomplex *out, uint_t n)
{
    fft_1ststage(in, out, n);

    fft_2ndstage(in, out, n);

    fft_finalstages(in, out, n);
}

void ifft(const dcomplex *in, dcomplex *out, uint_t n)
{
    const uint_t n2 = 1 << n;
    uint_t i;

    fft_1ststage_swap(in, out, n);

    fft_2ndstage(in, out, n);

    fft_finalstages(in, out, n);

    for (i = 0; i < n2; i++) {
        out[i] = comp_swap(out[i]);
    }
}

/*----------------------------------------------------------------------------------------------------*/

void fft_1ststage(const icomplex *in, icomplex *out, uint_t n)
{
    const uint_t * const bitrev = getbitrev(n);
    const uint_t *brp, n2 = 1 << n;
    icomplex a, b, *outp1, *outp2, *outp1e1;

    // first stage requires no twiddling
    // but we can carry out the bit-reversing inline
    brp   = bitrev;

    outp1 = out;
    outp2 = outp1 + 1;
    outp1e1 = outp1 + n2;   // set end pointer for outp1
    do {
        a.re = in[*brp].re >> 1; a.im = in[*brp].im >> 1; brp++;
        b.re = in[*brp].re >> 1; b.im = in[*brp].im >> 1; brp++;

        outp1->re = a.re + b.re;
        outp1->im = a.im + b.im; outp1 += 2;
        outp2->re = a.re - b.re;
        outp2->im = a.im - b.im; outp2 += 2;
    }
    while (outp1 < outp1e1);
}

void fft_1ststage_swap(const icomplex *in, icomplex *out, uint_t n)
{
    const uint_t * const bitrev = getbitrev(n);
    const uint_t *brp, n2 = 1 << n;
    icomplex a, b, *outp1, *outp2, *outp1e1;

    // first stage requires no twiddling
    // but we can carry out the bit-reversing inline
    brp   = bitrev;

    outp1 = out;
    outp2 = outp1 + 1;
    outp1e1 = outp1 + n2;   // set end pointer for outp1
    do {
        a.re = in[*brp].im >> 1; a.im = in[*brp].re >> 1; brp++;
        b.re = in[*brp].im >> 1; b.im = in[*brp].re >> 1; brp++;

        outp1->re = a.re + b.re;
        outp1->im = a.im + b.im; outp1 += 2;
        outp2->re = a.re - b.re;
        outp2->im = a.im - b.im; outp2 += 2;
    }
    while (outp1 < outp1e1);
}

void fft_2ndstage(const icomplex *in, icomplex *out, uint_t n)
{
    const uint_t n2 = 1 << n;
    icomplex a, b, *outp1, *outp2, *outp1e1;

    UNUSED(in);

    // second stage requires 0 and 180 degress twiddles
    outp1 = out;
    outp2 = outp1 + 2;
    outp1e1 = outp1 + n2;   // set end pointer for outp1
    do {
        a.re = outp1->re >> 1; a.im = outp1->im >> 1;
        b.re = outp2->re >> 1; b.im = outp2->im >> 1;

        outp1->re = a.re + b.re;
        outp1->im = a.im + b.im; outp1++;
        outp2->re = a.re - b.re;
        outp2->im = a.im - b.im; outp2++;

        a.re = outp1->re >> 1; a.im =   outp1->im >> 1;
        b.re = outp2->re >> 1; b.im = -(outp2->im >> 1);

        outp1->re = a.re + b.re;
        outp1->im = a.im + b.im; outp1 += 3;
        outp2->re = a.re - b.re;
        outp2->im = a.im - b.im; outp2 += 3;
    }
    while (outp1 < outp1e1);
}

void fft_finalstages(const icomplex *in, icomplex *out, uint_t n)
{
    const icomplex * const twiddle = getitwiddle(n);
    const icomplex *wp;
    const uint_t n2 = 1 << n;
    icomplex a, b, c, *outp1, *outp2, *outp1e1, *outp1e2;
    sint16_t re, im, wre, wim;
    uint_t i, count2, step;

    UNUSED(in);

    count2 = 4;

    // calculate step through sine table
    step   = 1 << (n - 3);

    // third and subsequent stages require full twiddling
    for (i = 2; i < n; i++) {
        outp1 = out;
        outp2 = outp1 + count2;
        outp1e2 = outp1 + n2;   // set end pointer for outp1 (outer loop)

        do {
            a.re = outp1->re >> 1; a.im = outp1->im >> 1;
            b.re = outp2->re >> 1; b.im = outp2->im >> 1;

            wp   = twiddle + step;

            outp1e1   = outp1 + count2; // set end pointer for outp1 (inner loop)

            outp1->re = a.re + b.re;
            outp1->im = a.im + b.im; outp1++;
            outp2->re = a.re - b.re;
            outp2->im = a.im - b.im; outp2++;

            do {
                wre = wp->re;
                wim = wp->im;

                a.re = outp1->re >> 1; a.im = outp1->im >> 1;
                b.re = outp2->re >> 1; b.im = outp2->im >> 1;

                // ignore overflow in multiplies since all
                // data exists in the lower halves only
                re =  (sint32_t)b.re * (sint32_t)wre + (sint32_t)b.im * (sint32_t)wim;
                im = -(sint32_t)b.re * (sint32_t)wim + (sint32_t)b.im * (sint32_t)wre;

                c.re = (sint16_t)(re >> 15);
                c.im = (sint16_t)(im >> 15);

                outp1->re = a.re + c.re;
                outp1->im = a.im + c.im; outp1++;
                outp2->re = a.re - c.re;
                outp2->im = a.im - c.im; outp2++;

                wp += step;
            }
            while (outp1 < outp1e1);

            outp1 += count2;
            outp2 += count2;
        }
        while (outp1 < outp1e2);

        count2 <<= 1;
        step   >>= 1;
    }
}

void fft(const icomplex *in, icomplex *out, uint_t n)
{
    fft_1ststage(in, out, n);

    fft_2ndstage(in, out, n);

    fft_finalstages(in, out, n);
}

void ifft(const icomplex *in, icomplex *out, uint_t n)
{
    const uint_t n2 = 1 << n;
    uint_t i;

    fft_1ststage_swap(in, out, n);

    fft_2ndstage(in, out, n);

    fft_finalstages(in, out, n);

    for (i = 0; i < n2; i++) {
        sint16_t re = out[i].re;
        out[i].re = out[i].im;
        out[i].im = re;
    }
}
