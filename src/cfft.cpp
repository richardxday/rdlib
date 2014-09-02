
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfft.h"

/* end of includes */

AFFT::AFFT(int iPoints) : Twiddle(NULL),
						  Bitrev(NULL),
						  Points(0),
						  PointsDiv4(0),
						  UsageCount(0)
{
	int len = (3 * iPoints) / 2;
	int i, j, n1, n2;
	
	Points     = iPoints;
	PointsDiv4 = Points / 4;

	Twiddle    = new double[len];
	Bitrev     = new int[Points];

	if (Twiddle && Bitrev) {
		for (i = 0; i < len; i++) {
			Twiddle[i] = sin((double)i / (double)Points * 2.0 * M_PI);
		}
		
		int bits = LOGN(Points, 2);
		for (i = 0; i < Points; i++) {
			n1 = i;
			n2 = 0;
			for (j = 0; j < bits; j++) {
				n2 = (n2 << 1) | (n1 & 1);
				n1 >>= 1;
			}
			Bitrev[i] = n2;
		}
	}
}

AFFT::~AFFT()
{
	if (Bitrev)  delete[] Bitrev;
	if (Twiddle) delete[] Twiddle;
}

float *AFFT::Create() const
{
	float *data = NULL;

	if ((data = new float[Points * 2]) != NULL) {
		memset(data, 0, Points * 2 * sizeof(*data));
	}

	return data;
}

double *AFFT::CreateDouble() const
{
	double *data = NULL;

	if ((data = new double[Points * 2]) != NULL) {
		memset(data, 0, Points * 2 * sizeof(*data));
	}

	return data;
}

void AFFT::fft(float *real, float *imag, float Scale) const
{
	double wr, wi, wrk, wik;
	int    i, j, k, pr, p1, p2;
	int    num = Points / 2, count = 1;
	int    stages = LOGN(Points, 2);

	do {
		p1 = pr = 0;
		j = count;

		do {
			wr = Twiddle[pr + PointsDiv4];
			wi = Twiddle[pr];

			p2 = p1 + num;
			k  = num;

			do {
				wrk = wr * real[p2] + wi * imag[p2];
				wik = wr * imag[p2] - wi * real[p2];
				real[p2] = (float)(real[p1] - wrk);
				imag[p2] = (float)(imag[p1] - wik);
				real[p1] = (float)(real[p1] + wrk);
				imag[p1] = (float)(imag[p1] + wik);

				p1++; p2++; k--;
			} while (k > 0);

			p1 += num;
			pr  = Bitrev[Bitrev[pr] + 2];
			j--;
		} while (j > 0);

		num >>= 1; count <<= 1; stages--;
	} while (stages > 0);

	float temp;
	for (i = 0; i < Points; i++) {
		k = Bitrev[i];
		if (k > i) {
			temp    = real[i];
			real[i] = real[k];
			real[k] = temp;
			temp    = imag[i];
			imag[i] = imag[k];
			imag[k] = temp;
		}
		
		real[i] *= Scale;
		imag[i] *= Scale;
	}
}

void AFFT::fft(double *real, double *imag, double Scale) const
{
	double wr, wi, wrk, wik;
	int    i, j, k, pr, p1, p2;
	int    num = Points / 2, count = 1;
	int    stages = LOGN(Points, 2);

	do {
		p1 = pr = 0;
		j = count;

		do {
			wr = Twiddle[pr + PointsDiv4];
			wi = Twiddle[pr];

			p2 = p1 + num;
			k  = num;

			do {
				wrk = wr * real[p2] + wi * imag[p2];
				wik = wr * imag[p2] - wi * real[p2];
				real[p2] = real[p1] - wrk;
				imag[p2] = imag[p1] - wik;
				real[p1] = real[p1] + wrk;
				imag[p1] = imag[p1] + wik;

				p1++; p2++; k--;
			} while (k > 0);

			p1 += num;
			pr  = Bitrev[Bitrev[pr] + 2];
			j--;
		} while (j > 0);

		num >>= 1; count <<= 1; stages--;
	} while (stages > 0);

	double temp;
	for (i = 0; i < Points; i++) {
		k = Bitrev[i];
		if (k > i) {
			temp    = real[i];
			real[i] = real[k];
			real[k] = temp;
			temp    = imag[i];
			imag[i] = imag[k];
			imag[k] = temp;
		}
		
		real[i] *= Scale;
		imag[i] *= Scale;
	}
}

void AFFT::Magnitude(float *data, bool Full, bool Phase) const
{
	float *real = GetReal(data), *imag = GetImag(data);
	float re, im, ph = 0.0;
	int   i, len = Full ? Points : Points / 2;

	for (i = 0; i < len; i++) {
		re = real[i];
		im = imag[i];
    
		if (Phase) {
			if (re == 0.0) ph = (float)((im < 0.0) ? (1.5 * M_PI) : (.5 * M_PI));
			else {
				ph = atanf(im / re);
				if (re < 0.0) ph += (float)M_PI;
				if (ph < 0.0) ph += (float)(2.0 * M_PI);
			}
		}
		real[i] = sqrtf(re * re + im * im);
		imag[i] = ph;
	}
}

void AFFT::InverseMagnitude(float *data) const
{
	float *real = GetReal(data), *imag = GetImag(data);
	float rad, ph;
	int   i;

	for (i = 0; i < Points; i++) {
		rad = real[i];
		ph  = imag[i];
		real[i] = rad * cosf(ph);
		imag[i] = rad * sinf(ph);
	}
}

void AFFT::MultiplyFFTs(float *data, const float *data1, const float *data2) const
{
	float       *real  = GetReal(data),  *imag  = GetImag(data);
	const float *real1 = GetReal(data1), *imag1 = GetImag(data1);
	const float *real2 = GetReal(data2), *imag2 = GetImag(data2);
	float       re, im;
	int         i;

	for (i = 0; i < Points; i++) {
		re = real1[i] * real2[i] - imag1[i] * imag2[i];
		im = real1[i] * imag2[i] + real1[i] * imag2[i];
		real[i] = re;
		imag[i] = im;
	}
}

void AFFT::Magnitude(double *data, bool Full, bool Phase) const
{
	double *real = GetReal(data), *imag = GetImag(data);
	double re, im, ph = 0.0;
	int   i, len = Full ? Points : Points / 2;

	for (i = 0; i < len; i++) {
		re = real[i];
		im = imag[i];
    
		if (Phase) {
			if (re == 0.0) ph = (double)((im < 0.0) ? (1.5 * M_PI) : (.5 * M_PI));
			else {
				ph = atan(im / re);
				if (re < 0.0) ph += (double)M_PI;
				if (ph < 0.0) ph += (double)(2.0 * M_PI);
			}
		}
		real[i] = sqrt(re * re + im * im);
		imag[i] = ph;
	}
}

void AFFT::InverseMagnitude(double *data) const
{
	double *real = GetReal(data), *imag = GetImag(data);
	double rad, ph;
	int   i;

	for (i = 0; i < Points; i++) {
		rad = real[i];
		ph  = imag[i];
		real[i] = rad * cos(ph);
		imag[i] = rad * sin(ph);
	}
}

void AFFT::MultiplyFFTs(double *data, const double *data1, const double *data2) const
{
	double       *real  = GetReal(data),  *imag  = GetImag(data);
	const double *real1 = GetReal(data1), *imag1 = GetImag(data1);
	const double *real2 = GetReal(data2), *imag2 = GetImag(data2);
	double       re, im;
	int          i;

	for (i = 0; i < Points; i++) {
		re = real1[i] * real2[i] - imag1[i] * imag2[i];
		im = real1[i] * imag2[i] + real1[i] * imag2[i];
		real[i] = re;
		imag[i] = im;
	}
}

void AFFT::InitState(FFT_STATE& state, float *real, float *imag, float Scale) const
{
	memset(&state, 0, sizeof(state));

	state.fft    = this;
	
	state.real   = real;
	state.imag   = imag;
	state.Scale  = Scale;
	
	state.wr     = Twiddle[PointsDiv4];
	state.wi     = Twiddle[0];

	state.Index1 = 0;
	state.Count1 = 1;
	
	state.Index2 = 0;
	state.Count2 = Points / 2;

	state.Pos    = 0;
	state.BRPos  = 0;

	state.Butterflies  = LOGN(Points, 2) * (Points / 2);
	state.ReorderCount = Points;
}

void AFFT::Process(FFT_STATE& state, int nOperations)
{
	double wrk, wik;
	int    n, p1 = state.Pos, p2 = p1 + state.Count2;

	assert(state.fft == this);

	n = MIN(nOperations, state.Butterflies);
	nOperations       -= n;
	state.Butterflies -= n;

	while (n > 0) {
		assert((p1 >= 0) && (p1 < Points));
		assert((p2 >= 0) && (p2 < Points));

		wrk = state.wr * state.real[p2] + state.wi * state.imag[p2];
		wik = state.wr * state.imag[p2] - state.wi * state.real[p2];
		state.real[p2] = (float)(state.real[p1] - wrk);
		state.imag[p2] = (float)(state.imag[p1] - wik);
		state.real[p1] = (float)(state.real[p1] + wrk);
		state.imag[p1] = (float)(state.imag[p1] + wik);
		
		p1++; p2++; state.Index2++; n--;

		if (state.Index2 == state.Count2) {
			state.Index2 = 0;

			p1 += state.Count2;
			p2 += state.Count2;

			state.Index1++;
			if (state.Index1 == state.Count1) {
				state.Index1 = 0;

				state.Count1 <<= 1;
				state.Count2 >>= 1;

				state.BRPos = 0;
				p1 = 0;
				p2 = state.Count2;
			}
			else state.BRPos = Bitrev[Bitrev[state.BRPos] + 2];

			state.wr = Twiddle[state.BRPos + PointsDiv4];
			state.wi = Twiddle[state.BRPos];
		}
	}

	n = MIN(nOperations, state.ReorderCount);
	nOperations        -= n;
	state.ReorderCount -= n;

	float temp;
	while (n > 0) {
		p2 = Bitrev[p1]; 

		assert((p1 >= 0) && (p1 < Points));
		assert((p2 >= 0) && (p2 < Points));

		if (p2 > p1) {
			temp           = state.real[p1];
			state.real[p1] = state.real[p2];
			state.real[p2] = temp;
			temp           = state.imag[p1];
			state.imag[p1] = state.imag[p2];
			state.imag[p2] = temp;
		}

		state.real[p1] *= state.Scale;
		state.imag[p1] *= state.Scale;

		p1++; n--;
	}

	state.Pos = p1;
}

#define PACK_FUNCTIONS_IMPLEMENT(type)																				\
void AFFT::Interleave(type *dst, const type *src, int offset, int n) const											\
{																													\
	const type *real = GetReal(src) + offset, *imag = GetImag(src) + offset;										\
	int   i;																										\
	for (i = 0; i < n; i++) {dst[0] = real[i]; dst[1] = imag[i]; dst += 2;}											\
}																													\
void AFFT::DeInterleave(type *dst, const type *src, int offset, int n) const										\
{																													\
	type *real = GetReal(dst) + offset, *imag = GetImag(dst) + offset;												\
	int  i;																											\
	for (i = 0; i < n; i++) {real[i] = src[0]; imag[i] = src[1]; src += 2;}											\
}																													\
void AFFT::Unpack(type *dst1, type *dst2, const type *src) const													\
{																													\
	const type *real = GetReal(src), *imag = GetImag(src);															\
	type  *real1 = GetReal(dst1), *imag1 = GetImag(dst1);															\
	type  *real2 = GetReal(dst2), *imag2 = GetImag(dst2);															\
	type  rep, rem, imp, imm;																						\
	int   i, n = Points / 2;																						\
																													\
	real1[0] = real[0];																								\
	real2[0] = imag[0];																								\
	imag1[0] = imag2[0] = 0.0;																						\
	for (i = 1; i < n ;i++) {																						\
		rep = (type)(.5 * (real[i] + real[Points - i]));															\
		rem = (type)(.5 * (real[i] - real[Points - i]));															\
		imp = (type)(.5 * (imag[i] + imag[Points - i]));															\
		imm = (type)(.5 * (imag[i] - imag[Points - i]));															\
																													\
		real1[i]          =  rep;																					\
		imag1[i]          =  imm;																					\
		real1[Points - i] =  rep;																					\
		imag1[Points - i] = -imm;																					\
																													\
		real2[i]          =  imp;																					\
		imag2[i]          = -rem;																					\
		real2[Points - i] =  imp;																					\
		imag2[Points - i] =  rem;																					\
	}																												\
}																													\
void AFFT::Pack(type *dst, const type *src1, const type *src2) const												\
{																													\
	const type *real1 = GetReal(src1), *imag1 = GetImag(src1);														\
	const type *real2 = GetReal(src2), *imag2 = GetImag(src2);														\
	type *real = GetReal(dst), *imag = GetImag(dst);																\
	type  re, im;																									\
	int   i;																										\
																													\
	for (i = 0; i < Points; i++) {																					\
		re = real1[i] - imag2[i];																					\
		im = imag1[i] + real2[i];																					\
																													\
		real[i] = re;																								\
		imag[i] = im;																								\
	}																												\
}																													\
void AFFT::UnpackAndInterleave(type *dst1, type *dst2, const type *src, int offset, int n) const					\
{																													\
	const type *real = GetReal(src), *imag = GetImag(src);															\
	type  rep, rem, imp, imm;																						\
	const int lim = Points / 2 + 1;																					\
	int   n1;																										\
																													\
	if (offset < lim) {																								\
		n1 = MIN(n, lim);																							\
		n -= n1;																									\
																													\
		if (offset == 0) {																							\
			dst1[0] = real[offset]; dst2[0] = imag[offset];															\
			dst2[1] = dst1[1] = 0.0;																				\
			dst1 += 2; dst2 += 2; offset++; n1--;																	\
		}																											\
																													\
		while (n1 > 0) {																							\
			rep = (type)(.5 * (real[offset] + real[Points - offset]));												\
			rem = (type)(.5 * (real[offset] - real[Points - offset]));												\
			imp = (type)(.5 * (imag[offset] + imag[Points - offset]));												\
			imm = (type)(.5 * (imag[offset] - imag[Points - offset]));												\
																													\
			dst1[0] =  rep;																							\
			dst1[1] =  imm;																							\
			dst2[0] =  imp;																							\
			dst2[1] = -rem;																							\
																													\
			dst1 += 2; dst2 += 2; offset++; n1--;																	\
		}																											\
	}																												\
	if (n && (offset >= lim)) {																					\
		n1 = MIN(n, Points - offset);																				\
		n -= n1;																									\
																													\
		while (n1 > 0) {																							\
			rep = (type)(.5 * (real[Points - offset] + real[offset]));												\
			rem = (type)(.5 * (real[Points - offset] - real[offset]));												\
			imp = (type)(.5 * (imag[Points - offset] + imag[offset]));												\
			imm = (type)(.5 * (imag[Points - offset] - imag[offset]));												\
																													\
			dst1[0] =  rep;																							\
			dst1[1] = -imm;																							\
			dst2[0] =  imp;																							\
			dst2[1] =  rem;																							\
																													\
			dst1 += 2; dst2 += 2; offset++; n1--;																	\
		}																											\
	}																												\
}																													\
void AFFT::DeInterleaveAndPack(type *dst, const type *src1, const type *src2, int offset, int n) const				\
{																													\
	type *real = GetReal(dst) + offset, *imag = GetImag(dst) + offset;												\
	int   i;																										\
	for (i = 0; i < n; i++) {																						\
		real[i] = src1[0] - src2[1];																				\
		imag[i] = src1[1] + src2[0];																				\
		src1 += 2; src2 += 2;																						\
	}																												\
}																													\
void AFFT::Convolve(type *dst, const type *src1, const type *src2) const											\
{																													\
	type *real = GetReal(dst), *imag = GetImag(dst);																\
	const type *real1 = GetReal(src1), *imag1 = GetImag(src1);														\
	const type *real2 = GetReal(src2), *imag2 = GetImag(src2);														\
	type  re, im;																									\
	int   i;																										\
	for (i = 0; i < Points; i++) {																					\
		re = real1[i] * real2[i] - imag1[i] * imag2[i];																\
		im = real1[i] * imag2[i] + imag1[i] * real2[i];																\
		real[i] = re;																								\
		imag[i] = im;																								\
	}																												\
}																													\
void AFFT::ConvolveAdd(type *dst, const type *src1, const type *src2) const											\
{																													\
	type *real = GetReal(dst), *imag = GetImag(dst);																\
	const type *real1 = GetReal(src1), *imag1 = GetImag(src1);														\
	const type *real2 = GetReal(src2), *imag2 = GetImag(src2);														\
	type  re, im;																									\
	int   i;																										\
	for (i = 0; i < Points; i++) {																					\
		re = real1[i] * real2[i] - imag1[i] * imag2[i];																\
		im = real1[i] * imag2[i] + imag1[i] * real2[i];																\
		real[i] += re;																								\
		imag[i] += im;																								\
	}																												\
}																													\
void AFFT::DeConvolve(type *dst, const type *src1, const type *src2) const											\
{																													\
	type *real = GetReal(dst), *imag = GetImag(dst);																\
	const type *real1 = GetReal(src1), *imag1 = GetImag(src1);														\
	const type *real2 = GetReal(src2), *imag2 = GetImag(src2);														\
	type  re, im, denom;																							\
	int   i;																										\
	for (i = 0; i < Points; i++) {																					\
		denom = real2[i] * real2[i] + imag2[i] *imag2[i];															\
		re = real1[i] * real2[i] + imag1[i] * imag2[i];																\
		im = imag1[i] * real2[i] - real1[i] * imag2[i];																\
		if (denom != 0.0) {																						\
			re /= denom;																							\
			im /= denom;																							\
		}																											\
		real[i] = re;																								\
		imag[i] = im;																								\
	}																												\
}																										

PACK_FUNCTIONS_IMPLEMENT(float);
PACK_FUNCTIONS_IMPLEMENT(double);

/*----------------------------------------------------------------------------------------------------*/

AFFTList::AFFTList()
{
}

AFFTList::~AFFTList()
{
	AFFT *fft;

	while ((fft = (AFFT *)List.Pop()) != NULL) delete fft;
}

AFFT *AFFTList::Create(int Points)
{
	AFFT *fft = NULL;
	int i;

	for (i = 0; ((fft = (AFFT *)List[i]) != NULL); i++) {
		if (fft->GetPoints() == Points) {
			fft->AddUsage();
			break;
		}
	}
	
	if (fft == NULL) {
		if ((fft = new AFFT(Points)) != NULL) {
			fft->AddUsage();
			List.Add(fft);
		}
	}

	return fft;
}

void AFFTList::Delete(AFFT *fft)
{
	if (fft->RemoveUsage()) {
		List.Remove(fft);
		
		delete fft;
	}
}

AFFTList FFTList;
