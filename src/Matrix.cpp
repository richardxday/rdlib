
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Matrix.h"

/* end of includes */

#define loc(x,y,n) ((x) + (y) * (n))

void VectorClear(double *vec, uint_t n)
{
	memset(vec, 0, n * sizeof(*vec));
}

void VectorAdd(double *dst, const double *vec1, const double *vec2, uint_t n)
{
	uint_t i;
	for (i = 0; i < n; i++) dst[i] = vec1[i] + vec2[i];
}

void VectorDiv(double *dst, const double *vec1, const double *vec2, uint_t n)
{
	uint_t i;
	for (i = 0; i < n; i++) dst[i] = vec1[i] - vec2[i];
}

void VectorMul(double *dst, const double *vec1, const double *vec2, uint_t n)
{
	uint_t i;
	for (i = 0; i < n; i++) dst[i] = vec1[i] * vec2[i];
}

void VectorMul(double *dst, const double *vec, double val, uint_t n)
{
	if      (val == 0.0) memset(dst, 0, n * sizeof(*dst));
	else if (val == 1.0) memcpy(dst, vec, n * sizeof(*dst));
	else {
		uint_t i;
		for (i = 0; i < n; i++) dst[i] = vec[i] * val;
	}
}

void VectorDiv(double *dst, const double *vec, double val, uint_t n)
{
	VectorMul(dst, vec, 1.0 / val, n);
}

double VectorMod(const double *vec, uint_t n)
{
	double mod = 0.0;
	uint_t i;
	for (i = 0; i < n; i++) mod += vec[i] * vec[i];
	return sqrt(mod);
}

double DotProduct(const double *vec1, const double *vec2, uint_t n)
{
	double val = 0.0;
	uint_t i;
	for (i = 0; i < n; i++) val += vec1[i] * vec2[i];
	return val;
}

void MatrixIdentity(double *mat, uint_t n)
{
	memset(mat, 0, n * n * sizeof(*mat));
}

void MatrixCopy(double *dst, const double *src, uint_t n)
{
	memcpy(dst, src, n * n * sizeof(*dst));
}

void MatrixMul(double *dst, const double *mat1, const double *mat2, uint_t n)
{
	double tmp[64];
	uint_t i, j, k;

	assert(n < 8);

	for (j = 0; j < n; j++) {
		for (i = 0; i < n; i++) {
			tmp[loc(i, j, n)] = 0.0;
			for (k = 0; k < n; k++) {
				tmp[loc(i, j, n)] += mat1[loc(i, k, n)] * mat2[loc(k, j, n)];
			}
		}
	}
	memcpy(dst, tmp, n * n * sizeof(*dst));
}

void MatrixMul(double *dst, const double *mat, double val, uint_t n)
{
	VectorMul(dst, mat, val, n * n);
}

void MatrixDiv(double *dst, const double *mat, double val, uint_t n)
{
	VectorMul(dst, mat, 1.0 / val, n * n);
}

void MatrixRotate(double *dst, const double *mat, uint_t index1, uint_t index2, double angle, uint_t n)
{
	if (angle != 0.0) {
		double tmp[64];

		assert(n < 8);

		MatrixIdentity(tmp, n);

		angle *= M_PI / 180.0;
		double si = sin(angle), co = cos(angle);
		tmp[loc(index1, index1, n)] = si; tmp[loc(index2, index1, n)] =  co;
		tmp[loc(index1, index2, n)] = co; tmp[loc(index2, index2, n)] = -si;

		MatrixMul(dst, mat, tmp, n);
	}
	else MatrixCopy(dst, mat, n);
}

void MatrixInverse(double *dst, const double *mat, uint_t n)
{
	double tmp[64];
	
	assert(n < 8);

	UNUSED(mat);

	MatrixIdentity(tmp, n);

	UNUSED(dst);
}

void MatrixTranspose(double *dst, const double *mat, uint_t n)
{
	uint_t i, j;

	if (mat == dst) {
		for (j = 1; j < n; j++) {
			for (i = 0; i < j; i++) {
				uint_t loc1 = loc(i, j, n);
				uint_t loc2 = loc(j, i, n);
				double a = mat[loc1];
				double b = mat[loc2];
				dst[loc1] = b;
				dst[loc2] = a;
			}
		}
	}
	else {
		for (j = 0; j < n; j++) {
			for (i = 0; i < n; i++) {
				uint_t loc1 = loc(i, j, n);
				uint_t loc2 = loc(j, i, n);
				double a = mat[loc1];
				double b = mat[loc2];
				dst[loc1] = b;
				dst[loc2] = a;
			}
		}
	}
}

double& MatrixElement(double *mat, uint_t i, uint_t j, uint_t n)
{
	return mat[loc(i, j, n)];
}

double MatrixElement(const double *mat, uint_t i, uint_t j, uint_t n)
{
	return mat[loc(i, j, n)];
}

void MatrixVecMul(double *dst, const double *mat, const double *vec, uint_t n)
{
	uint_t i, j;

	for (j = 0; j < n; j++) {
		dst[j] = 0.0;
		for (i = 0; i < n; i++) {
			dst[j] += mat[loc(i, j, n)] * vec[i];
		}
	}
}
