#ifndef __FAST_COMPLEX__
#define __FAST_COMPLEX__

#include "types.h"

typedef struct {
    float re, im;
} scomplex;

typedef struct {
    double re, im;
} dcomplex;

typedef struct {
    sint16_t re, im;
} icomplex;

extern dcomplex comp_exp(double w, double r = 1.0);
inline dcomplex comp_value(double re = 0.0, double im = 0.0) {dcomplex res = {re, im}; return res;}
inline dcomplex comp_swap(const dcomplex& obj1) {dcomplex res = {obj1.im,  obj1.re}; return res;}
inline dcomplex comp_conj(const dcomplex& obj1) {dcomplex res = {obj1.re, -obj1.im}; return res;}
inline dcomplex comp_rotate90(const dcomplex& obj1) {dcomplex res = {obj1.im, -obj1.re}; return res;}
inline dcomplex comp_add(const dcomplex& obj1, const dcomplex& obj2) {dcomplex res = {obj1.re + obj2.re, obj1.im + obj2.im}; return res;}
inline dcomplex comp_sub(const dcomplex& obj1, const dcomplex& obj2) {dcomplex res = {obj1.re - obj2.re, obj1.im - obj2.im}; return res;}
inline dcomplex comp_mul(const dcomplex& obj1, const dcomplex& obj2) {dcomplex res = {obj1.re * obj2.re - obj1.im * obj2.im, obj1.re * obj2.im + obj1.im * obj2.re}; return res;}
extern dcomplex comp_div(const dcomplex& obj1, const dcomplex& obj2);

inline dcomplex comp_mul(const dcomplex& obj1, double val) {dcomplex res = {obj1.re * val, obj1.im * val}; return res;}
inline dcomplex comp_div(const dcomplex& obj1, double val) {dcomplex res = {obj1.re / val, obj1.im / val}; return res;}

inline void     comp_add(dcomplex& res, const dcomplex& obj1, const dcomplex& obj2) {res.re = obj1.re + obj2.re; res.im = obj1.im + obj2.im;}
inline void     comp_sub(dcomplex& res, const dcomplex& obj1, const dcomplex& obj2) {res.re = obj1.re - obj2.re; res.im = obj1.im - obj2.im;}
inline void     comp_mul(dcomplex& res, const dcomplex& obj1, const dcomplex& obj2) {double re2 = obj1.re * obj2.re - obj1.im * obj2.im; res.im = obj1.re * obj2.im + obj1.im * obj2.re; res.re = re2;}
inline void     comp_div(dcomplex& res, const dcomplex& obj1, const dcomplex& obj2) {res = comp_div(obj1, obj2);}

inline void     comp_mul(dcomplex& res, const dcomplex& obj1, double val) {res.re = obj1.re * val; res.im = obj1.im * val;}
inline void     comp_div(dcomplex& res, const dcomplex& obj1, double val) {res.re = obj1.re / val; res.im = obj1.im / val;}

inline void     comp_add_sub(dcomplex& obj1, dcomplex& obj2) {dcomplex res1 = {obj1.re + obj2.re, obj1.im + obj2.im}; obj2.re = obj1.re - obj2.re; obj2.im = obj1.im - obj2.im; obj1 = res1;}
inline void     comp_add_sub(dcomplex& dst1, dcomplex& dst2, const dcomplex& obj1, const dcomplex& obj2) {dcomplex res1 = {obj1.re + obj2.re, obj1.im + obj2.im}; dst2.re = obj1.re - obj2.re; dst2.im = obj1.im - obj2.im; dst1 = res1;}

extern double   comp_mod(const dcomplex& obj1);
extern double   comp_arg(const dcomplex& obj1);
inline double   comp_dot(const dcomplex& obj1, const dcomplex& obj2) {return obj1.re * obj2.re + obj1.im * obj2.im;}

/*----------------------------------------------------------------------------------------------------*/

extern scomplex comp_exp(float w, float r = 1.f);
inline scomplex comp_single(const dcomplex& obj1) {scomplex res = {(float)obj1.re, (float)obj1.im}; return res;}
inline scomplex comp_value(float re = 0.0, float im = 0.0) {scomplex res = {re, im}; return res;}
inline scomplex comp_swap(const scomplex& obj1) {scomplex res = {obj1.im,  obj1.re}; return res;}
inline scomplex comp_conj(const scomplex& obj1) {scomplex res = {obj1.re, -obj1.im}; return res;}
inline scomplex comp_rotate90(const scomplex& obj1) {scomplex res = {obj1.im, -obj1.re}; return res;}
inline scomplex comp_add(const scomplex& obj1, const scomplex& obj2) {scomplex res = {obj1.re + obj2.re, obj1.im + obj2.im}; return res;}
inline scomplex comp_sub(const scomplex& obj1, const scomplex& obj2) {scomplex res = {obj1.re - obj2.re, obj1.im - obj2.im}; return res;}
inline scomplex comp_mul(const scomplex& obj1, const scomplex& obj2) {scomplex res = {obj1.re * obj2.re - obj1.im * obj2.im, obj1.re * obj2.im + obj1.im * obj2.re}; return res;}

extern scomplex comp_div(const scomplex& obj1, const scomplex& obj2);
inline scomplex comp_mul(const scomplex& obj1, float val) {scomplex res = {obj1.re * val, obj1.im * val}; return res;}
inline scomplex comp_div(const scomplex& obj1, float val) {scomplex res = {obj1.re / val, obj1.im / val}; return res;}

extern float    comp_mod(const scomplex& obj1);
extern float    comp_arg(const scomplex& obj1);
inline float    comp_dot(const scomplex& obj1, const scomplex& obj2) {return obj1.re * obj2.re + obj1.im * obj2.im;}

inline void     comp_add(scomplex& res, const scomplex& obj1, const scomplex& obj2) {res.re = obj1.re + obj2.re; res.im = obj1.im + obj2.im;}
inline void     comp_sub(scomplex& res, const scomplex& obj1, const scomplex& obj2) {res.re = obj1.re - obj2.re; res.im = obj1.im - obj2.im;}
inline void     comp_mul(scomplex& res, const scomplex& obj1, const scomplex& obj2) {float re2 = obj1.re * obj2.re - obj1.im * obj2.im; res.im = obj1.re * obj2.im + obj1.im * obj2.re; res.re = re2;}
inline void     comp_div(scomplex& res, const scomplex& obj1, const scomplex& obj2) {res = comp_div(obj1, obj2);}

inline void     comp_mul(scomplex& res, const scomplex& obj1, float val) {res.re = obj1.re * val; res.im = obj1.im * val;}
inline void     comp_div(scomplex& res, const scomplex& obj1, float val) {res.re = obj1.re / val; res.im = obj1.im / val;}

inline void     comp_add_sub(scomplex& obj1, scomplex& obj2) {scomplex res1 = {obj1.re + obj2.re, obj1.im + obj2.im}; obj2.re = obj1.re - obj2.re; obj2.im = obj1.im - obj2.im; obj1 = res1;}
inline void     comp_add_sub(scomplex& dst1, scomplex& dst2, const scomplex& obj1, const scomplex& obj2) {scomplex res1 = {obj1.re + obj2.re, obj1.im + obj2.im}; dst2.re = obj1.re - obj2.re; dst2.im = obj1.im - obj2.im; dst1 = res1;}

#endif
