
#include <math.h>

#include "fastcomplex.h"

dcomplex comp_exp(double w, double r)
{
    dcomplex res = {r * cos(w), r * sin(w)};
    return res;
}

dcomplex comp_div(const dcomplex& obj1, const dcomplex& obj2)
{
    dcomplex res;
    double mag;

    res.re = obj1.re * obj2.re + obj1.im * obj2.im;
    res.im = obj1.re * obj2.im - obj1.im * obj2.re;
    mag    = res.re * res.re + res.im * res.im;
    if (mag != 0.0) {
        res.re /= mag;
        res.im /= mag;
    }

    return res;
}

double comp_mod(const dcomplex& obj1)
{
    return sqrt(obj1.re * obj1.re + obj1.im * obj1.im);
}

double comp_arg(const dcomplex& obj1)
{
    return atan2(obj1.im, obj1.re);
}

/*----------------------------------------------------------------------------------------------------*/

scomplex comp_exp(float w, float r)
{
    scomplex res = {r * cosf(w), r * sinf(w)};
    return res;
}

scomplex comp_div(const scomplex& obj1, const scomplex& obj2)
{
    scomplex res;
    float mag;

    res.re = obj1.re * obj2.re + obj1.im * obj2.im;
    res.im = obj1.re * obj2.im - obj1.im * obj2.re;
    mag    = res.re * res.re + res.im * res.im;
    if (mag != 0.0) {
        res.re /= mag;
        res.im /= mag;
    }

    return res;
}

float comp_mod(const scomplex& obj1)
{
    return sqrtf(obj1.re * obj1.re + obj1.im * obj1.im);
}

float comp_arg(const scomplex& obj1)
{
    return atan2f(obj1.im, obj1.re);
}
