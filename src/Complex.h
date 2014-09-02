#ifndef __COMPLEX__
#define __COMPLEX__

#include "misc.h"

class Complex {
public:
	Complex(double Re = 0.0, double Im = 0.0) {re = Re; im = Im;}
	Complex(const Complex& val) {re = val.re; im = val.im;}
  
	void is(Complex& val) {re = val.re; im = val.im;}
	double Re() const {return re;}
	double Im() const {return im;}
  
	Complex& operator = (double Re) {re = Re; im = 0.0; return *this;}
	Complex& operator = (const Complex& a) {re = a.re; im = a.im; return *this;}

	friend Complex operator + (const Complex& a, double Re) {return Complex(a.re + Re, a.im);}
	friend Complex operator + (double Re, const Complex a) {return Complex(a.re + Re, a.im);}
	friend Complex operator + (const Complex& a, const Complex& b) {return Complex(a.re + b.re, a.im + b.im);}
	friend Complex operator - (const Complex& a, double Re) {return Complex(a.re - Re, a.im);}
	friend Complex operator - (double Re, const Complex& a) {return Complex(Re - a.re,-a.im);}
	friend Complex operator - (const Complex& a, const Complex& b) {return Complex(a.re - b.re, a.im - b.im);}
	Complex operator - () {return Complex(-re,-im);}
	friend Complex operator * (const Complex& a, double Re) {return Complex(a.re * Re, a.im * Re);}
	friend Complex operator * (double Re, const Complex& a) {return Complex(a.re * Re, a.im * Re);}
	friend Complex operator * (const Complex& a, const Complex& b) {return Complex(a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re);}
	friend Complex operator / (const Complex& a, double Re) {return Complex(a.re / Re, a.im / Re);}
	friend Complex operator / (const Complex& a, const Complex& b) {
		Complex a1(a.re * b.re + a.im * b.im, a.im * b.re - a.re * b.im);
		double c = b.re * b.re + b.im * b.im;

		if (c != 0.0) {
			a1.re /= c;
			a1.im /= c;
		}

		return a1;
	}
	friend int operator == (const Complex& a, const Complex& b)    {return  ((a.re == b.re) && (a.im == b.im));}
	friend int operator != (const Complex& a, const Complex& b) {return !((a.re == b.re) && (a.im == b.im));}
	friend int operator <  (const Complex& a, const Complex& b)    {return  ((a.re <  b.re) && (a.im <  b.im));}
	friend int operator <= (const Complex& a, const Complex& b)    {return  ((a.re <= b.re) && (a.im <= b.im));}
	friend int operator >  (const Complex& a, const Complex& b)    {return  ((a.re >  b.re) && (a.im >  b.im));}
	friend int operator >= (const Complex& a, const Complex& b)    {return  ((a.re >= b.re) && (a.im >= b.im));}

	Complex  operator++(int) {Complex res = *this; re++; return res;}
	Complex  operator--(int) {Complex res = *this; re++; return res;}
	Complex& operator++() {re++; return *this;}
	Complex& operator--() {re--; return *this;}

	Complex& operator += (double Re) {re += Re; return *this;}
	Complex& operator += (const Complex& a) {re += a.re; im += a.im; return *this;}
	Complex& operator -= (double Re) {re -= Re; return *this;}
	Complex& operator -= (const Complex& a) {re -= a.re; im -= a.im; return *this;}
	Complex& operator *= (double Re) {re *= Re; im *= Re; return *this;}
	Complex& operator *= (const Complex& a) {double re1 = re * a.re - im * a.im; im = re * a.im + im * a.re; re = re1; return *this;}
	Complex& operator /= (double Re) {re /= Re; im /= Re; return *this;}
	Complex& operator /= (const Complex& a) {
		Complex a1(re * a.re + im * a.im, im * a.re - re * a.im);
		double c = a.re * a.re + a.im * a.im;
		if (c != 0.0) {
			re = a1.re / c;
			im = a1.im / c;
		}
		return *this;
	}

	double mod() const {return sqrt(re * re + im * im);}
	double arg() const {return atan2(im, re);}
	void exp(double w) {re = cos(w); im = sin(w);}
	void exp(double f, double fs) {double w = f / fs * 2.0 * pi; re = cos(w); im = sin(w);}

	double re, im;
};

class FComplex {
public:
	FComplex(float Re = 0.0, float Im = 0.0) {re = Re; im = Im;}
	FComplex(const FComplex& val) {re = val.re; im = val.im;}
  
	void is(FComplex& val) {re = val.re; im = val.im;}
	float Re() const {return re;}
	float Im() const {return im;}
  
	FComplex& operator = (float Re) {re = Re; im = 0.0; return *this;}
	FComplex& operator = (const FComplex& a) {re = a.re; im = a.im; return *this;}

	friend FComplex operator + (const FComplex& a, float Re) {return FComplex(a.re + Re, a.im);}
	friend FComplex operator + (float Re, const FComplex a) {return FComplex(a.re + Re, a.im);}
	friend FComplex operator + (const FComplex& a, const FComplex& b) {return FComplex(a.re + b.re, a.im + b.im);}
	friend FComplex operator - (const FComplex& a, float Re) {return FComplex(a.re - Re, a.im);}
	friend FComplex operator - (float Re, const FComplex& a) {return FComplex(Re - a.re,-a.im);}
	friend FComplex operator - (const FComplex& a, const FComplex& b) {return FComplex(a.re - b.re, a.im - b.im);}
	FComplex operator - () {return FComplex(-re,-im);}
	friend FComplex operator * (const FComplex& a, float Re) {return FComplex(a.re * Re, a.im * Re);}
	friend FComplex operator * (float Re, const FComplex& a) {return FComplex(a.re * Re, a.im * Re);}
	friend FComplex operator * (const FComplex& a, const FComplex& b) {return FComplex(a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re);}
	friend FComplex operator / (const FComplex& a, float Re) {return FComplex(a.re / Re, a.im / Re);}
	friend FComplex operator / (const FComplex& a, const FComplex& b) {
		FComplex a1(a.re * b.re + a.im * b.im, a.im * b.re - a.re * b.im);
		float c = b.re * b.re + b.im * b.im;

		if (c != 0.0) {
			a1.re /= c;
			a1.im /= c;
		}

		return a1;
	}
	friend int operator == (const FComplex& a, const FComplex& b)    {return  ((a.re == b.re) && (a.im == b.im));}
	friend int operator != (const FComplex& a, const FComplex& b) {return !((a.re == b.re) && (a.im == b.im));}
	friend int operator <  (const FComplex& a, const FComplex& b)    {return  ((a.re <  b.re) && (a.im <  b.im));}
	friend int operator <= (const FComplex& a, const FComplex& b)    {return  ((a.re <= b.re) && (a.im <= b.im));}
	friend int operator >  (const FComplex& a, const FComplex& b)    {return  ((a.re >  b.re) && (a.im >  b.im));}
	friend int operator >= (const FComplex& a, const FComplex& b)    {return  ((a.re >= b.re) && (a.im >= b.im));}

	FComplex  operator++(int) {FComplex res = *this; re++; return res;}
	FComplex  operator--(int) {FComplex res = *this; re++; return res;}
	FComplex& operator++() {re++; return *this;}
	FComplex& operator--() {re--; return *this;}

	FComplex& operator += (float Re) {re += Re; return *this;}
	FComplex& operator += (const FComplex& a) {re += a.re; im += a.im; return *this;}
	FComplex& operator -= (float Re) {re -= Re; return *this;}
	FComplex& operator -= (const FComplex& a) {re -= a.re; im -= a.im; return *this;}
	FComplex& operator *= (float Re) {re *= Re; im *= Re; return *this;}
	FComplex& operator *= (const FComplex& a) {float re1 = re * a.re - im * a.im; im = re * a.im + im * a.re; re = re1; return *this;}
	FComplex& operator /= (float Re) {re /= Re; im /= Re; return *this;}
	FComplex& operator /= (const FComplex& a) {
		FComplex a1(re * a.re + im * a.im, im * a.re - re * a.im);
		float c = a.re * a.re + a.im * a.im;
		if (c != 0.0) {
			re = a1.re / c;
			im = a1.im / c;
		}
		return *this;
	}

	float mod() const {return sqrtf(re * re + im * im);}
	float arg() const {
		float val;
		if (re == 0.0) val = (float)((im < 0.0) ? (1.5 * pi) : (.5 * pi));
		else {
			val = atanf(im / re);
			if (re < 0.0)
				val += (float)pi;
			if (val < 0.0)
				val += (float)(2.0 * pi);
		}  
		return val;
	}
	void exp(float w) {re = cosf(w); im = sinf(w);}
	void exp(float f, float fs) {float w = f / fs * 2.0f * (float)pi; re = cosf(w); im = sinf(w);}

	float re, im;
};

#endif
