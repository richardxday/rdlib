
#include <stdio.h>

#include "PointSizeRect.h"

/* end of includes */

APoint& APoint::SetPoint(const ASize& sz)
{
	x = sz.w, y = sz.h;
	return *this;
}

APoint& APoint::operator = (const ASize& sz)
{
	x = sz.w, y = sz.h;
	return *this;
}

/*------------------------------------------------------------------------------------------*/

AColour::AColour(const AColour& col, int scale)
{
	if (scale != 0xff) {
		r = muldivb(col.r, scale);
		g = muldivb(col.g, scale);
		b = muldivb(col.b, scale);
		a = muldivb(col.a, scale);
	}
	else {
		r = col.r;
		g = col.g;
		b = col.b;
		a = col.a;
	}
}

AColour::AColour(const char *pText)
{
	FromText(pText);
}

AColour& AColour::SetColour(const AColour& col, int scale)
{
	if (scale != 0xff) {
		r = muldivb(col.r, scale);
		g = muldivb(col.g, scale);
		b = muldivb(col.b, scale);
		a = muldivb(col.a, scale);
	}
	else {
		r = col.r;
		g = col.g;
		b = col.b;
		a = col.a;
	}

	return *this;
}

AColour& AColour::Maximize()
{
	int col;
	
	col = MAX(r, g);
	col = MAX(col, b);
	col = MAX(col, a);

	if (col) {
		int scale = 255 * 255 / col;
		r = muldivb(r, scale);
		g = muldivb(g, scale);
		b = muldivb(b, scale);
		a = muldivb(a, scale);
	}

	return *this;
}

AColour& AColour::Mix(const AColour& col1, const AColour& col2, int scale1, int scale2)
{
	int r1 = col1.r * scale1 + col2.r * scale2;
	int g1 = col1.g * scale1 + col2.g * scale2;
	int b1 = col1.b * scale1 + col2.b * scale2;
	int a1 = col1.a * scale1 + col2.a * scale2;

	r1 = MAX(r1, 0);
	r1 = MIN(r1, 255);
	r  = (uint8_t)r1;

	g1 = MAX(g1, 0);
	g1 = MIN(g1, 255);
	g  = (uint8_t)g1;

	b1 = MAX(b1, 0);
	b1 = MIN(b1, 255);
	b  = (uint8_t)b1;

	a1 = MAX(a1, 0);
	a1 = MIN(a1, 255);
	a  = (uint8_t)a1;

	return *this;
}
