
#ifndef __MATRIX__
#define __MATRIX__

#include "misc.h"

extern void VectorClear(double *vec, uint_t n);
extern void VectorAdd(double *dst, const double *vec1, const double *vec2, uint_t n);
extern void VectorSub(double *dst, const double *vec1, const double *vec2, uint_t n);
extern void VectorMul(double *dst, const double *vec1, const double *vec2, uint_t n);
extern void VectorMul(double *dst, const double *vec, double val, uint_t n);
extern void VectorDiv(double *dst, const double *vec, double val, uint_t n);
extern double VectorMod(const double *vec, uint_t n);
extern double DotProduct(const double *vec1, const double *vec2, uint_t n);

extern void MatrixIdentity(double *mat, uint_t n);
extern void MatrixCopy(double *dst, const double *src, uint_t n);
extern void MatrixMul(double *dst, const double *mat1, const double *mat2, uint_t n);
extern void MatrixMul(double *dst, const double *mat, double val, uint_t n);
extern void MatrixDiv(double *dst, const double *mat, double val, uint_t n);
extern void MatrixRotate(double *dst, const double *mat, uint_t index1, uint_t index2, double angle, uint_t n);
extern void MatrixInverse(double *dst, const double *mat, uint_t n);
extern void MatrixTranspose(double *dst, const double *mat, uint_t n);

extern double& MatrixElement(double *mat, uint_t i, uint_t j, uint_t n);
extern double  MatrixElement(const double *mat, uint_t i, uint_t j, uint_t n);

extern void MatrixVecMul(double *dst, const double *mat, const double *vec, uint_t n);

#endif
