
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __LINUX__
#include <sys/time.h>
#include <unistd.h>
#endif

#ifdef __MACH__
#include <mach/mach_time.h>
#endif

#include "misc.h"

/* end of includes */

#ifndef M_PI
const double M_PI = acos(0.0) * 2.0;
#endif

double dBToGain(double val)
{
	return pow(10.0, (val * .05));
}

double GainTodB(double val, double mindB)
{
	double l = mindB;

	if (val > 0.0) l = 20.0 * log10(val);

	l = MAX(l, mindB);

	return l;
}

sint32_t muldivs(sint32_t a, sint32_t b, sint32_t c)
{
	return (sint32_t)(((sint64_t)a * (sint64_t)b) / c);
}

uint32_t muldivu(uint32_t a, uint32_t b, uint32_t c)
{
	return (uint32_t)(((uint64_t)a * (uint64_t)b) / c);
}

sint32_t muldivsr(sint32_t a, sint32_t b, sint32_t c)
{
	sint64_t ires = (sint64_t)a * (sint64_t)b;
	sint32_t bias = abs(c) >> 1;
	ires += (ires < 0) ? -bias : bias;
	return (sint32_t)(ires / c);
}

sint32_t fmuldivsr(double a, sint32_t b, sint32_t c)
{
	double ires = a * (double)b;
	ires += (ires < 0.0) ? -.5 : .5;
	return (sint32_t)(ires / (double)c);
}

uint32_t muldivur(uint32_t a,uint32_t b,uint32_t c)
{
	return (uint32_t)((((uint64_t)a * (uint64_t)b) + (c >> 1)) / c);
}

sint32_t addms(sint32_t a, sint32_t b, sint32_t l)
{
	return ((l - a) >= b) ? a + b : l;
}

uint32_t addmu(uint32_t a, uint32_t b, uint32_t l)
{
	return ((l - a) >= b) ? a + b : l;
}

sint32_t subzs(sint32_t a, sint32_t b)
{
	return (a >= b) ? a - b : 0;
}

uint32_t subzu(uint32_t a, uint32_t b)
{
	return (a >= b) ? a - b : 0;
}

uint8_t muldivb(int a, int b)
{
	int c = (a * b) / 255;
	return (uint8_t)LIMIT(c, 0, 255);
}

#ifndef __GNUC__
sint64_t llabs(sint64_t val)
{
	return (val >= 0) ? val : -val;
}
#endif

double InterpRead(double *pBuffer, uint32_t Length, double pos)
{
	double a  = pos - floor(pos), b = 1.0 - a;
	uint32_t  p1 = (uint32_t)pos, p2 = p1 + 1;

	assert((Length & (Length - 1)) == 0);

	Length--;
	p1 &= Length;
	p2 &= Length;

	return b * pBuffer[p1] + a * pBuffer[p2];
}

float InterpRead(float *pBuffer, uint32_t Length, float pos)
{
	float a  = pos - floorf(pos), b = 1.0f - a;
	uint32_t p1 = (uint32_t)pos, p2 = p1 + 1;

	assert((Length & (Length - 1)) == 0);

	Length--;
	p1 &= Length;
	p2 &= Length;

	return b * pBuffer[p1] + a * pBuffer[p2];
}

double InterpSum(double *pBuffer, uint32_t Length, double pos1, double pos2)
{
	double a = 1.0 - (pos1 - floor(pos1)), b = pos2 - floor(pos2);
	double sum = 0.0;
	uint32_t  p1 = (uint32_t)pos1, p2 = (uint32_t)pos2;

	assert((Length & (Length - 1)) == 0);

	Length--;

	if (p2 == p1) {
		double c = pBuffer[p1 & Length], d = pBuffer[(p1 + 1) & Length];
		double e = a * c + (1.0 - a) * d;
		double f = b * d + (1.0 - b) * c;

		sum = (e + f) / (b - a);
	}
	else {
		sum += a * (pBuffer[ p1      & Length] * a + pBuffer[(p1 + 1) & Length] * (2.0 - a));
		sum += b * (pBuffer[(p2 + 1) & Length] * b + pBuffer[ p2      & Length] * (2.0 - b));

		p1++;
		while (p1 != p2) {
			sum += pBuffer[p1 & Length] + pBuffer[(p1 + 1) & Length];
			p1++;
		}
	}

	return .5 * sum / (pos2 - pos1);
}

float InterpSum(float *pBuffer, uint32_t Length, float  pos1, float  pos2)
{
	float a = 1.0f - (pos1 - floorf(pos1)), b = pos2 - floorf(pos2);
	float sum = 0.0f;
	uint32_t p1 = (uint32_t)pos1, p2 = (uint32_t)pos2;

	assert((Length & (Length - 1)) == 0);

	Length--;

	if (p2 == p1) {
		float c = pBuffer[p1 & Length], d = pBuffer[(p1 + 1) & Length];
		float e = a * c + (1.0f - a) * d;
		float f = b * d + (1.0f - b) * c;

		sum = e + f;
	}
	else {
		sum += a * (pBuffer[ p1      & Length] * a + pBuffer[(p1 + 1) & Length] * (2.0f - a));
		sum += b * (pBuffer[(p2 + 1) & Length] * b + pBuffer[ p2      & Length] * (2.0f - b));

		p1++;
		while (p1 != p2) {
			sum += pBuffer[p1 & Length] + pBuffer[(p1 + 1) & Length];
			p1++;
		}
	}

	return .5f * sum / (pos2 - pos1);
}

void InterpReadStereo(double *pBufferL, double *pBufferR, uint32_t Length, double pos, double& left, double& right)
{
	double a  = pos - floor(pos), b = 1.0 - a;
	uint32_t  p1 = (uint32_t)pos, p2 = p1 + 1;

	assert((Length & (Length - 1)) == 0);

	Length--;
	p1 &= Length;
	p2 &= Length;

	left  = b * pBufferL[p1] + a * pBufferL[p2];
	right = b * pBufferR[p1] + a * pBufferR[p2];
}

void InterpReadStereo(float *pBufferL, float *pBufferR, uint32_t Length, float pos, float& left, float& right)
{
	float a  = pos - floorf(pos), b = 1.0f - a;
	uint32_t p1 = (uint32_t)pos, p2 = p1 + 1;

	assert((Length & (Length - 1)) == 0);

	Length--;
	p1 &= Length;
	p2 &= Length;

	left  = b * pBufferL[p1] + a * pBufferL[p2];
	right = b * pBufferR[p1] + a * pBufferR[p2];
}

#ifndef MachineIsBigEndian
bool MachineIsBigEndian()
{
	static const uint16_t __EndianID = 0x0001;
	static const bool     BigEndian  = (*(uint8_t *)&__EndianID == 0);
	return BigEndian;
}
#endif

bool ByteSwapNeeded(bool be)
{
	static bool _be = MachineIsBigEndian();
	return (be != _be);
}

uint16_t SwapBytes(uint16_t data)
{
	uint16_t data1   = data;
	uint8_t  *pData  = (uint8_t *)&data;
	uint8_t  *pData1 = (uint8_t *)&data1;

	pData1[0] = pData[1];
	pData1[1] = pData[0];

	return data1;
}
uint32_t SwapBytes(uint32_t data)
{
	uint32_t data1   = data;
	uint8_t  *pData  = (uint8_t *)&data;
	uint8_t  *pData1 = (uint8_t *)&data1;

	pData1[0] =	pData[3];
	pData1[1] =	pData[2];
	pData1[2] =	pData[1];
	pData1[3] =	pData[0];

	return data1;
}
uint64_t SwapBytes(uint64_t data)
{
	uint64_t data1   = data;
	uint8_t  *pData  = (uint8_t *)&data;
	uint8_t  *pData1 = (uint8_t *)&data1;

	pData1[0] =	pData[7];
	pData1[1] =	pData[6];
	pData1[2] =	pData[5];
	pData1[3] =	pData[4];
	pData1[4] =	pData[3];
	pData1[5] =	pData[2];
	pData1[6] =	pData[1];
	pData1[7] =	pData[0];

	return data1;
}
float SwapBytes(float data)
{
	float   data1   = data;
	uint8_t *pData  = (uint8_t *)&data;
	uint8_t *pData1 = (uint8_t *)&data1;

	pData1[0] =	pData[3];
	pData1[1] =	pData[2];
	pData1[2] =	pData[1];
	pData1[3] =	pData[0];

	return data1;
}
double SwapBytes(double data)
{
	double  data1   = data;
	uint8_t *pData  = (uint8_t *)&data;
	uint8_t *pData1 = (uint8_t *)&data1;

	pData1[0] =	pData[7];
	pData1[1] =	pData[6];
	pData1[2] =	pData[5];
	pData1[3] =	pData[4];
	pData1[4] =	pData[3];
	pData1[5] =	pData[2];
	pData1[6] =	pData[1];
	pData1[7] =	pData[0];

	return data1;
}

#define SWAP(n1,n) temp = p[n1]; p[n1] = p[n1+n]; p[n1+n] = temp
#define SWAP_BLOCK_PROCESSING 0

static void __swap_bytes_2(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0,  1);
		p += 2; n--;
	}
	while (n) {
		SWAP( 0,  1);
		SWAP( 2,  1);
		SWAP( 4,  1);
		SWAP( 6,  1);
		p += 8; n -= 4;
	}
#else
	while (n) {
		SWAP( 0,  1);
		p += 2; n--;
	}
#endif
}
static void __swap_bytes_3(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0,  2);
		p += 3; n--;
	}
	while (n) {
		SWAP( 0,  2);
		SWAP( 3,  2);
		SWAP( 6,  2);
		SWAP( 9,  2);
		p += 12; n -= 4;
	}
#else
	while (n) {
		SWAP( 0,  2);
		p += 3; n--;
	}
#endif
}
static void __swap_bytes_4(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0,  3); SWAP( 1,  1);
		p += 4; n--;
	}
	while (n) {
		SWAP( 0,  3); SWAP( 1,  1);
		SWAP( 4,  3); SWAP( 5,  1);
		SWAP( 8,  3); SWAP( 9,  1);
		SWAP(12,  3); SWAP(13,  1);
		p += 16; n -= 4;
	}
#else
	while (n) {
		SWAP( 0,  3); SWAP( 1,  1);
		p += 4; n--;
	}
#endif
}
static void __swap_bytes_5(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0,  4); SWAP( 1,  2);
		p += 5; n--;
	}
	while (n) {
		SWAP( 0,  4); SWAP( 1,  2);
		SWAP( 5,  4); SWAP( 6,  2);
		SWAP(10,  4); SWAP(11,  2);
		SWAP(15,  4); SWAP(16,  2);
		p += 20; n -= 4;
	}
#else
	while (n) {
		SWAP( 0,  4); SWAP( 1,  2);
		p += 5; n--;
	}
#endif
}
static void __swap_bytes_6(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0,  5); SWAP( 1,  3); SWAP( 2,  1);
		p += 6; n--;
	}
	while (n) {
		SWAP( 0,  5); SWAP( 1,  3); SWAP( 2,  1);
		SWAP( 6,  5); SWAP( 7,  3); SWAP( 8,  1);
		SWAP(12,  5); SWAP(13,  3); SWAP(14,  1);
		SWAP(18,  5); SWAP(19,  3); SWAP(20,  1);
		p += 24; n -= 4;
	}
#else
	while (n) {
		SWAP( 0,  5); SWAP( 1,  3); SWAP( 2,  1);
		p += 6; n--;
	}
#endif
}
static void __swap_bytes_7(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0,  6); SWAP( 1,  4); SWAP( 2,  2);
		p += 7; n--;
	}
	while (n) {
		SWAP( 0,  6); SWAP( 1,  4); SWAP( 2,  2);
		SWAP( 7,  6); SWAP( 8,  4); SWAP( 9,  2);
		SWAP(14,  6); SWAP(15,  4); SWAP(16,  2);
		SWAP(21,  6); SWAP(22,  4); SWAP(23,  2);
		p += 28; n -= 4;
	}
#else
	while (n) {
		SWAP( 0,  6); SWAP( 1,  4); SWAP( 2,  2);
		p += 7; n--;
	}
#endif
}
static void __swap_bytes_8(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0,  7); SWAP( 1,  5); SWAP( 2,  3); SWAP( 3,  1);
		p += 8; n--;
	}
	while (n) {
		SWAP( 0,  7); SWAP( 1,  5); SWAP( 2,  3); SWAP( 3,  1);
		SWAP( 8,  7); SWAP( 9,  5); SWAP(10,  3); SWAP(11,  1);
		SWAP(16,  7); SWAP(17,  5); SWAP(18,  3); SWAP(19,  1);
		SWAP(24,  7); SWAP(25,  5); SWAP(26,  3); SWAP(27,  1);
		p += 32; n -= 4;
	}
#else
	while (n) {
		SWAP( 0,  7); SWAP( 1,  5); SWAP( 2,  3); SWAP( 3,  1);
		p += 8; n--;
	}
#endif
}
static void __swap_bytes_9(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0,  8); SWAP( 1,  6); SWAP( 2,  4); SWAP( 3,  2);
		p += 9; n--;
	}
	while (n) {
		SWAP( 0,  8); SWAP( 1,  6); SWAP( 2,  4); SWAP( 3,  2);
		SWAP( 9,  8); SWAP(10,  6); SWAP(11,  4); SWAP(12,  2);
		SWAP(18,  8); SWAP(19,  6); SWAP(20,  4); SWAP(21,  2);
		SWAP(27,  8); SWAP(28,  6); SWAP(29,  4); SWAP(30,  2);
		p += 36; n -= 4;
	}
#else
	while (n) {
		SWAP( 0,  8); SWAP( 1,  6); SWAP( 2,  4); SWAP( 3,  2);
		p += 9; n--;
	}
#endif
}
static void __swap_bytes_10(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0,  9); SWAP( 1,  7); SWAP( 2,  5); SWAP( 3,  3); SWAP( 4,  1);
		p += 10; n--;
	}
	while (n) {
		SWAP( 0,  9); SWAP( 1,  7); SWAP( 2,  5); SWAP( 3,  3); SWAP( 4,  1);
		SWAP(10,  9); SWAP(11,  7); SWAP(12,  5); SWAP(13,  3); SWAP(14,  1);
		SWAP(20,  9); SWAP(21,  7); SWAP(22,  5); SWAP(23,  3); SWAP(24,  1);
		SWAP(30,  9); SWAP(31,  7); SWAP(32,  5); SWAP(33,  3); SWAP(34,  1);
		p += 40; n -= 4;
	}
#else
	while (n) {
		SWAP( 0,  9); SWAP( 1,  7); SWAP( 2,  5); SWAP( 3,  3); SWAP( 4,  1);
		p += 10; n--;
	}
#endif
}
static void __swap_bytes_11(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0, 10); SWAP( 1,  8); SWAP( 2,  6); SWAP( 3,  4); SWAP( 4,  2);
		p += 11; n--;
	}
	while (n) {
		SWAP( 0, 10); SWAP( 1,  8); SWAP( 2,  6); SWAP( 3,  4); SWAP( 4,  2);
		SWAP(11, 10); SWAP(12,  8); SWAP(13,  6); SWAP(14,  4); SWAP(15,  2);
		SWAP(22, 10); SWAP(23,  8); SWAP(24,  6); SWAP(25,  4); SWAP(26,  2);
		SWAP(33, 10); SWAP(34,  8); SWAP(35,  6); SWAP(36,  4); SWAP(37,  2);
		p += 44; n -= 4;
	}
#else
	while (n) {
		SWAP( 0, 10); SWAP( 1,  8); SWAP( 2,  6); SWAP( 3,  4); SWAP( 4,  2);
		p += 11; n--;
	}
#endif
}
static void __swap_bytes_12(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0, 11); SWAP( 1,  9); SWAP( 2,  7); SWAP( 3,  5); SWAP( 4,  3); SWAP( 5,  1);
		p += 12; n--;
	}
	while (n) {
		SWAP( 0, 11); SWAP( 1,  9); SWAP( 2,  7); SWAP( 3,  5); SWAP( 4,  3); SWAP( 5,  1);
		SWAP(12, 11); SWAP(13,  9); SWAP(14,  7); SWAP(15,  5); SWAP(16,  3); SWAP(17,  1);
		SWAP(24, 11); SWAP(25,  9); SWAP(26,  7); SWAP(27,  5); SWAP(28,  3); SWAP(29,  1);
		SWAP(36, 11); SWAP(37,  9); SWAP(38,  7); SWAP(39,  5); SWAP(40,  3); SWAP(41,  1);
		p += 48; n -= 4;
	}
#else
	while (n) {
		SWAP( 0, 11); SWAP( 1,  9); SWAP( 2,  7); SWAP( 3,  5); SWAP( 4,  3); SWAP( 5,  1);
		p += 12; n--;
	}
#endif
}
static void __swap_bytes_13(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0, 12); SWAP( 1, 10); SWAP( 2,  8); SWAP( 3,  6); SWAP( 4,  4); SWAP( 5,  2);
		p += 13; n--;
	}
	while (n) {
		SWAP( 0, 12); SWAP( 1, 10); SWAP( 2,  8); SWAP( 3,  6); SWAP( 4,  4); SWAP( 5,  2);
		SWAP(13, 12); SWAP(14, 10); SWAP(15,  8); SWAP(16,  6); SWAP(17,  4); SWAP(18,  2);
		SWAP(26, 12); SWAP(27, 10); SWAP(28,  8); SWAP(29,  6); SWAP(30,  4); SWAP(31,  2);
		SWAP(39, 12); SWAP(40, 10); SWAP(41,  8); SWAP(42,  6); SWAP(43,  4); SWAP(44,  2);
		p += 52; n -= 4;
	}
#else
	while (n) {
		SWAP( 0, 12); SWAP( 1, 10); SWAP( 2,  8); SWAP( 3,  6); SWAP( 4,  4); SWAP( 5,  2);
		p += 13; n--;
	}
#endif
}
static void __swap_bytes_14(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0, 13); SWAP( 1, 11); SWAP( 2,  9); SWAP( 3,  7); SWAP( 4,  5); SWAP( 5,  3); SWAP( 6,  1);
		p += 14; n--;
	}
	while (n) {
		SWAP( 0, 13); SWAP( 1, 11); SWAP( 2,  9); SWAP( 3,  7); SWAP( 4,  5); SWAP( 5,  3); SWAP( 6,  1);
		SWAP(14, 13); SWAP(15, 11); SWAP(16,  9); SWAP(17,  7); SWAP(18,  5); SWAP(19,  3); SWAP(20,  1);
		SWAP(28, 13); SWAP(29, 11); SWAP(30,  9); SWAP(31,  7); SWAP(32,  5); SWAP(33,  3); SWAP(34,  1);
		SWAP(42, 13); SWAP(43, 11); SWAP(44,  9); SWAP(45,  7); SWAP(46,  5); SWAP(47,  3); SWAP(48,  1);
		p += 56; n -= 4;
	}
#else
	while (n) {
		SWAP( 0, 13); SWAP( 1, 11); SWAP( 2,  9); SWAP( 3,  7); SWAP( 4,  5); SWAP( 5,  3); SWAP( 6,  1);
		p += 14; n--;
	}
#endif
}
static void __swap_bytes_15(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0, 14); SWAP( 1, 12); SWAP( 2, 10); SWAP( 3,  8); SWAP( 4,  6); SWAP( 5,  4); SWAP( 6,  2);
		p += 15; n--;
	}
	while (n) {
		SWAP( 0, 14); SWAP( 1, 12); SWAP( 2, 10); SWAP( 3,  8); SWAP( 4,  6); SWAP( 5,  4); SWAP( 6,  2);
		SWAP(15, 14); SWAP(16, 12); SWAP(17, 10); SWAP(18,  8); SWAP(19,  6); SWAP(20,  4); SWAP(21,  2);
		SWAP(30, 14); SWAP(31, 12); SWAP(32, 10); SWAP(33,  8); SWAP(34,  6); SWAP(35,  4); SWAP(36,  2);
		SWAP(45, 14); SWAP(46, 12); SWAP(47, 10); SWAP(48,  8); SWAP(49,  6); SWAP(50,  4); SWAP(51,  2);
		p += 60; n -= 4;
	}
#else
	while (n) {
		SWAP( 0, 14); SWAP( 1, 12); SWAP( 2, 10); SWAP( 3,  8); SWAP( 4,  6); SWAP( 5,  4); SWAP( 6,  2);
		p += 15; n--;
	}
#endif
}
static void __swap_bytes_16(uint8_t *p, uint_t n)
{
	uint8_t temp;
#if SWAP_BLOCK_PROCESSING
	while (n & 3) {
		SWAP( 0, 15); SWAP( 1, 13); SWAP( 2, 11); SWAP( 3,  9); SWAP( 4,  7); SWAP( 5,  5); SWAP( 6,  3); SWAP( 7,  1);
		p += 16; n--;
	}
	while (n) {
		SWAP( 0, 15); SWAP( 1, 13); SWAP( 2, 11); SWAP( 3,  9); SWAP( 4,  7); SWAP( 5,  5); SWAP( 6,  3); SWAP( 7,  1);
		SWAP(16, 15); SWAP(17, 13); SWAP(18, 11); SWAP(19,  9); SWAP(20,  7); SWAP(21,  5); SWAP(22,  3); SWAP(23,  1);
		SWAP(32, 15); SWAP(33, 13); SWAP(34, 11); SWAP(35,  9); SWAP(36,  7); SWAP(37,  5); SWAP(38,  3); SWAP(39,  1);
		SWAP(48, 15); SWAP(49, 13); SWAP(50, 11); SWAP(51,  9); SWAP(52,  7); SWAP(53,  5); SWAP(54,  3); SWAP(55,  1);
		p += 64; n -= 4;
	}
#else
	while (n) {
		SWAP( 0, 15); SWAP( 1, 13); SWAP( 2, 11); SWAP( 3,  9); SWAP( 4,  7); SWAP( 5,  5); SWAP( 6,  3); SWAP( 7,  1);
		p += 16; n--;
	}
#endif
}
typedef void (*SWAPFUNC)(uint8_t *p, uint_t n);
static const SWAPFUNC SwapFuncs[MAX_SWAP_BYTES + 1] = {
	NULL, NULL, &__swap_bytes_2, &__swap_bytes_3, &__swap_bytes_4, &__swap_bytes_5, &__swap_bytes_6, &__swap_bytes_7, &__swap_bytes_8, &__swap_bytes_9, &__swap_bytes_10, &__swap_bytes_11, &__swap_bytes_12, &__swap_bytes_13, &__swap_bytes_14, &__swap_bytes_15, &__swap_bytes_16,
};

void SwapBytes(void *buf, uint_t nbytes)
{
	assert(nbytes < NUMBEROF(SwapFuncs));
	if (SwapFuncs[nbytes]) (*SwapFuncs[nbytes])((uint8_t *)buf, 1);
}
void SwapBytes(void *buf, uint_t itemsize, uint_t nitems)
{
	assert(itemsize < NUMBEROF(SwapFuncs));
	if (SwapFuncs[itemsize]) (*SwapFuncs[itemsize])((uint8_t *)buf, nitems);
}

void *Allocate(void *pData, size_t nItems, size_t& MaxItems, size_t ItemSize, size_t Inc)
{
	if ((pData == NULL) || (nItems > MaxItems)) {
		size_t NewMaxItems = MAX(nItems, MaxItems) + Inc - 1;
		void   *pNewData;

		NewMaxItems -= NewMaxItems % Inc;
		NewMaxItems  = MAX(NewMaxItems, Inc);

		if ((pNewData = (void *)new uint8_t[NewMaxItems * ItemSize]) != NULL) {
			memset(pNewData, 0, NewMaxItems * ItemSize);

			if (pData) {
				memcpy(pNewData, pData, MaxItems * ItemSize);
				delete[] (uint8_t *)pData;
			}

			pData    = pNewData;
			MaxItems = NewMaxItems;
		}
		else {
			debug("Failed to allocate %lu items of %lu bytes\n", (ulong_t)NewMaxItems, (ulong_t)ItemSize);

			if (pData) {
				delete[] (uint8_t *)pData;
				pData  = NULL;
			}

			MaxItems = 0;
		}
	}

	return pData;
}

static bool debug_enabled = true;

void debug(const char *fmt, ...)
{
	if (debug_enabled) {
		va_list ap;

		va_start(ap, fmt);
#ifdef _WIN32
		char *buf = NULL;
		vasprintf(&buf, fmt, ap);
		if (buf) {
			OutputDebugString(buf);
			free(buf);
		}
#else
		vprintf(fmt, ap);
#endif
		va_end(ap);
	}
}

void enabledebug(bool enabled)
{
	debug_enabled = enabled;
}

#if defined(_WIN32) && !defined(__MINGW32__)
int vasprintf(char **buf, const char *fmt, va_list ap)
{
	int l = -1;

	if ((buf[0] = (char *)calloc(32768, sizeof(**buf))) != NULL) {
		l = vsprintf(buf[0], fmt, ap);
	}

	return l;
}

static HINSTANCE hInstance = 0;

void SetInstance(HINSTANCE h)
{
	::hInstance = h;
}

HINSTANCE GetInstance()
{
	return ::hInstance;
}

uint32_t Exec(const char *cmd, uint32_t timeout)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD retcode = 0;

	const char *cmdproc = getenv("ComSpec");
	if (!cmdproc) cmdproc = "cmd.exe";

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));
	
	// Start the child process
	char cmdline[1024];
	sprintf(cmdline, "%s /c if 1==1 %s", cmdproc, cmd);
	//debug("Exec: %s\n", cmdline);
	if(!CreateProcess(NULL,				// No module name (use command line)
					  cmdline,			// Command line
					  NULL,				// Process handle not inheritable
					  NULL,				// Thread handle not inheritable
					  true,				// Set handle inheritance to false
					  0,				// No creation flags
					  NULL,				// Use parent's environment block
					  NULL,				// Use parent's starting directory 
					  &si,				// Pointer to STARTUPINFO structure
					  &pi)) {			// Pointer to PROCESS_INFORMATION structure
		//debug("Exec: failed to create process, code %lu\n", GetLastError());
		return ~0;
	}
	
	// Wait until child process exits
	WaitForSingleObject(pi.hProcess, timeout);
	
	// Close process and thread handles
	GetExitCodeProcess(pi.hProcess, &retcode);
			
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return retcode;
}
#endif

#ifdef __LINUX__
uint32_t GetTickCount()
{
#ifdef __MACH__
	static mach_timebase_info_data_t timebase;
	static bool inited = false;

	if (!inited) {
		mach_timebase_info(&timebase);
		inited = true;
	}

	uint64_t tick = mach_absolute_time();
	tick = (tick * timebase.numer) / timebase.denom;

	return (uint32_t)(tick / 1000000);
#else
	struct timespec timespec;

#ifdef ANDROID
	clock_gettime(CLOCK_MONOTONIC_HR, &timespec);
#elif defined(__CYGWIN__)
	clock_gettime(CLOCK_MONOTONIC, &timespec);
#else
	clock_gettime(CLOCK_MONOTONIC_RAW, &timespec);
#endif

	return timespec.tv_sec * 1000 + (timespec.tv_nsec / 1000000);
#endif
}

void Sleep(uint32_t ms)
{
	usleep(ms * 1000);
}
#endif

/*--------------------------------------------------------------------------------*/
/** Multiply 64-bit unsigned integer by 32-bit fraction without overflow
 */
/*--------------------------------------------------------------------------------*/
uint64_t muldivu(uint64_t val, uint32_t mul, uint32_t div)
{
  // split val into 32-bit parts for multiplying
  uint32_t parts[] = {(uint32_t)val, (uint32_t)(val >> 32)};

  // result (3 x 32 bits)
  uint32_t res[NUMBEROF(parts) + 1];    // maximum number of 32-bit parts required for product

  uint64_t carry = 0;                   // accumulator
  uint_t i;

  // first, multiply up by multiplier
  for (i = 0; i < NUMBEROF(parts); i++)
  {
    carry += mul * (uint64_t)parts[i];  // 64-bit multiply of 32-bit parts
    res[i] = (uint32_t)carry;           // save lower 32-bits of result
    carry >>= 32;                       // and shift down to carry to next stage
  }
  // save final value
  res[i] = (uint32_t)carry;

  // now divide by divider
  carry = 0;
  for (i = NUMBEROF(res); i > 0; /* i pre-decremented below */)
  {
    carry <<= 32;                           // shift carry up
    carry  += res[--i];                     // add value
    res[i]  = (uint32_t)(carry / div);      // store result of divide
    carry  %= div;                          // save remainder
  }

  return res[0] + ((uint64_t)res[1] << 32);
}

/*--------------------------------------------------------------------------------*/
/** Return machine time on in nanoseconds
 */
/*--------------------------------------------------------------------------------*/
uint64_t GetNanosecondTicks()
{
#ifdef _WIN32
  static uint32_t div = 0;
  LARGE_INTEGER time;

  // find divider if not known
  if (!div)
  {
    LARGE_INTEGER freq;

    // calculate divisor to get from ns from performance counter
    QueryPerformanceFrequency(&freq);

    // assume divider is 32-bits or less
    div = (uint32_t)freq.QuadPart;
  }

  QueryPerformanceCounter(&time);

  // multiply time by 1e9 (s->ns) then divide by divisor
  return muldivu((uint64_t)time.QuadPart, 1000000000, div);
#elif __MACH__
  static mach_timebase_info_data_t timebase;
  static bool inited = false;

  if (!inited)
  {
    mach_timebase_info(&timebase);
    inited = true;
  }

  // scale by timebase.numer / timebase.denom which results in nano-seconds
  return muldivu(mach_absolute_time(), timebase.numer, timebase.denom);
#else
  struct timespec timespec;

#ifdef ANDROID
  clock_gettime(CLOCK_MONOTONIC_HR, &timespec);
#elif defined(__CYGWIN__)
  clock_gettime(CLOCK_MONOTONIC, &timespec);
#else
  clock_gettime(CLOCK_MONOTONIC_RAW, &timespec);
#endif

  return (uint64_t)timespec.tv_sec * 1000000000ULL + (uint64_t)timespec.tv_nsec;
#endif
}

/*--------------------------------------------------------------------------------*/
/** Fix denormalize value to prevent execution penalties
 */
/*--------------------------------------------------------------------------------*/
double fix_denormalized(double val)
{
	volatile double res;

	// adding 1.0e-308 and subtracting 1.0-308 results in 0 if value is too small 
	res  = val;
	res += 1.0e-308;
	res -= 1.0e-308;

	return res;
}

bool Interpolate(double& value, double target, double coeff, double limit)
{
	bool updated = false;

	if (value != target) {
		value += (target - value) * coeff;
		value  = fix_denormalized(value);
		if (fabs(target - value) <= limit) value = target;
		updated = true;
	}

	return updated;
}

bool InterpolateInteger(double& value, double target, double coeff, double limit)
{
	int val = (int)value;

	::Interpolate(value, target, coeff, limit);

	return ((int)value != val);
}

void LimitAngle(double& angle)
{
	while (angle >= 180.0) angle -= 360.0;
	while (angle < -180.0) angle += 360.0;
}

bool InterpolateAngle(double& value, double target, double coeff, double limit)
{
	double diff = target - value;

	LimitAngle(diff);

	value += diff * coeff;
	value  = fix_denormalized(value);
	if ((fabs(diff) <= limit) || (fabs(target - value) <= limit)) value = target;

	return (diff != 0.0);
}

bool Interpolate2ndOrder(double& value, double& velocity, double target, double coeff, double speed, double limit)
{
	bool updated = false;

	if (speed == 0.0) {
		updated |= Interpolate(value, target, coeff, limit);
	}
	else if (value != target) {
		value   += velocity * speed;
		updated |= Interpolate(velocity, target - value, coeff, limit);
		updated |= Interpolate(value, target, .1, limit);
	}

	return updated;
}

bool Interpolate2ndOrderAngle(double& value, double& velocity, double target, double coeff, double speed, double limit)
{
	bool updated = false;

	if (speed == 0.0) {
		updated |= InterpolateAngle(value, target, coeff, limit);
	}
	else if (value != target) {
		value   += velocity * speed;
		updated |= InterpolateAngle(velocity, target - value, coeff, limit);
		updated |= InterpolateAngle(value, target, .1, limit);
	}

	return updated;
}

