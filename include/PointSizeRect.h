
#ifndef __POINT_SIZE_RECT__
#define __POINT_SIZE_RECT__

#include "misc.h"
#include "strsup.h"

enum {
    JUSTIFY_FRACTION = 100,
    JUSTIFY_LEFT,
    JUSTIFY_CENTRE,
    JUSTIFY_RIGHT,
    JUSTIFY_FULLY,
    JUSTIFY_TOP = JUSTIFY_LEFT,
    JUSTIFY_BOTTOM = JUSTIFY_RIGHT,
};

class ASize;
class APoint {
public:
    APoint(int ix = 0, int iy = 0) {x = ix; y = iy;}
    APoint(const APoint& object) {x = object.x; y = object.y;}
#ifdef _WIN32
    APoint(const POINT& pt) {x = pt.x; y = pt.y;}
#endif

    APoint& SetPoint(int ix = 0, int iy = 0) {
        x = ix; y = iy;
        return *this;
    }
    APoint& SetPoint(const ASize& sz);

    APoint& operator = (const APoint& pt) {
        x = pt.x; y = pt.y;
        return *this;
    }
    APoint& operator = (const ASize& sz);

#ifdef _WIN32
    APoint& operator = (const POINT& pt) {
        x = pt.x; y = pt.y;
        return *this;
    }
#endif

    APoint& operator += (const APoint& pt) {
        x += pt.x; y += pt.y;
        return *this;
    }
    APoint& operator -= (const APoint& pt) {
        x -= pt.x; y -= pt.y;
        return *this;
    }
    APoint& operator *= (const APoint& pt) {
        x *= pt.x; y *= pt.y;
        return *this;
    }
    APoint& operator /= (const APoint& pt) {
        x /= pt.x; y /= pt.y;
        return *this;
    }
    APoint& operator *= (int n) {
        x *= n; y *= n;
        return *this;
    }
    APoint& operator /= (int n) {
        x /= n; y /= n;
        return *this;
    }

    friend APoint operator + (const APoint& pt1, const APoint& pt2) {
        APoint pt(pt1.x + pt2.x, pt1.y + pt2.y);
        return pt;
    }
    friend APoint operator - (const APoint& pt1, const APoint& pt2) {
        APoint pt(pt1.x - pt2.x, pt1.y - pt2.y);
        return pt;
    }
    friend APoint operator * (const APoint& pt1, const APoint& pt2) {
        APoint pt(pt1.x * pt2.x, pt1.y * pt2.y);
        return pt;
    }
    friend APoint operator / (const APoint& pt1, const APoint& pt2) {
        APoint pt(pt1.x / pt2.x, pt1.y / pt2.y);
        return pt;
    }
    friend APoint operator * (const APoint& pt1, int n) {
        APoint pt(pt1.x * n, pt1.y * n);
        return pt;
    }
    friend APoint operator / (const APoint& pt1, int n) {
        APoint pt(pt1.x / n, pt1.y / n);
        return pt;
    }
    friend APoint operator * (int n, const APoint& pt1) {
        APoint pt(pt1.x * n, pt1.y * n);
        return pt;
    }

#define POINT_COMPARE(op) bool operator op (const APoint& pt) const {return ((x op pt.x) && (y op pt.y));}

    POINT_COMPARE(==);
    POINT_COMPARE(<=);
    POINT_COMPARE(>=);
    POINT_COMPARE(<);
    POINT_COMPARE(>);

    bool operator != (const APoint& pt) const {return !operator == (pt);}

    friend APoint Min(const APoint& pt1, const APoint& pt2) {return APoint(MIN(pt1.x, pt2.x), MIN(pt1.y, pt2.y));}
    friend APoint Max(const APoint& pt1, const APoint& pt2) {return APoint(MAX(pt1.x, pt2.x), MAX(pt1.y, pt2.y));}

    APoint operator - () {return APoint(-x, -y);}

    APoint& Move(int x1, int y1) {
        x += x1; y += y1;
        return *this;
    }

#ifdef __STRING_SUPPORT__
    AString ToText() const {
        AString String;
        String.Format("%d %d", x, y);
        return String;
    }
    void FromText(const AString& text) {
        SetPoint((int)text.Word(0), (int)text.Word(1));
    }
#endif

#ifdef _WIN32
    operator POINT () const {
        POINT pt;
        pt.x = x; pt.y = y;
        return pt;
    }
#endif

    int x, y;
};

class ASize {
public:
    ASize(int iw = 0, int ih = 0) {w = iw; h = ih;}
    ASize(const ASize& object) {w = object.w; h = object.h;}
#ifdef _WIN32
    ASize(const SIZE& sz) {w = sz.cx; h = sz.cy;}
#endif

    ASize& SetSize(int iw = 0, int ih = 0) {w = iw; h = ih; return *this;}
    ASize& SetSize(const APoint& pt) {
        w = pt.x, h = pt.y;
        return *this;
    }

    ASize& operator = (const ASize& sz) {
        w = sz.w; h = sz.h;
        return *this;
    }

    ASize& operator = (const APoint& pt) {
        w = pt.x, h = pt.y;
        return *this;
    }

#ifdef _WIN32
    ASize& operator = (const SIZE& sz) {
        w = sz.cx; h = sz.cy;
        return *this;
    }
#endif

    ASize& operator += (const ASize& sz) {
        w += sz.w; h += sz.h;
        return *this;
    }
    ASize& operator -= (const ASize& sz) {
        w -= sz.w; h -= sz.h;
        return *this;
    }
    ASize& operator *= (const ASize& sz) {
        w *= sz.w; h *= sz.h;
        return *this;
    }
    ASize& operator /= (const ASize& sz) {
        w /= sz.w; h /= sz.h;
        return *this;
    }
    ASize& operator *= (int n) {
        w *= n; h *= n;
        return *this;
    }
    ASize& operator /= (int n) {
        w /= n; h /= n;
        return *this;
    }
    friend ASize operator + (const ASize& sz1, const ASize& sz2) {
        ASize sz(sz1.w + sz2.w, sz1.h + sz2.h);
        return sz;
    }
    friend ASize operator - (const ASize& sz1, const ASize& sz2) {
        ASize sz(sz1.w - sz2.w, sz1.h - sz2.h);
        return sz;
    }
    friend ASize operator * (const ASize& sz1, const ASize& sz2) {
        ASize sz(sz1.w * sz2.w, sz1.h * sz2.h);
        return sz;
    }
    friend ASize operator / (const ASize& sz1, const ASize& sz2) {
        ASize sz(sz1.w / sz2.w, sz1.h / sz2.h);
        return sz;
    }
    friend ASize operator * (const ASize& sz1, int n) {
        ASize sz(sz1.w * n, sz1.h * n);
        return sz;
    }
    friend ASize operator / (const ASize& sz1, int n) {
        ASize sz(sz1.w / n, sz1.h / n);
        return sz;
    }
    friend ASize operator * (int n, const ASize& sz1) {
        ASize sz(sz1.w * n, sz1.h * n);
        return sz;
    }

#define SIZE_COMPARE(op) bool operator op (const ASize& sz) const {return ((w op sz.w) && (h op sz.h));}

    SIZE_COMPARE(==);
    SIZE_COMPARE(<=);
    SIZE_COMPARE(>=);
    SIZE_COMPARE(<);
    SIZE_COMPARE(>);

    bool operator != (const ASize& sz) const {return !operator == (sz);}

    friend ASize Min(const ASize& sz1, const ASize& sz2) {return ASize(MIN(sz1.w, sz2.w), MIN(sz1.h, sz2.h));}
    friend ASize Max(const ASize& sz1, const ASize& sz2) {return ASize(MAX(sz1.w, sz2.w), MAX(sz1.h, sz2.h));}

    ASize operator - () {return ASize(-w, -h);}

#ifdef __STRING_SUPPORT__
    AString ToText() const {
        AString String;
        String.Format("%d %d", w, h);
        return String;
    }
    void FromText(const AString& text) {
        SetSize((int)text.Word(0), (int)text.Word(1));
    }
#endif

#ifdef _WIN32
    operator SIZE () const {
        SIZE sz;
        sz.cx = w; sz.cy = h;
        return sz;
    }
#endif

    int w, h;
};

inline int Justify(int w1, int w2, uint8_t just) {
    int diff   = w2 - w1;
    int offset = 0;
    if (just <= JUSTIFY_FRACTION) offset = ((int)just * diff) / JUSTIFY_FRACTION;
    else {
        switch (just) {
            case JUSTIFY_LEFT:
            case JUSTIFY_CENTRE:
            case JUSTIFY_RIGHT:
                offset = ((int)(just - JUSTIFY_LEFT) * diff) / 2;
                break;

            case JUSTIFY_FULLY:
                break;

            default:
                break;
        }
    }
    return offset;
}

class ARect {
public:
    ARect(int ix = 0, int iy = 0, int iw = 0, int ih = 0) {
        x = MIN(ix, ix + iw);
        y = MIN(iy, iy + ih);
        w = MAX(ix, ix + iw) - x;
        h = MAX(iy, iy + ih) - y;
    }
    ARect(const ARect& object) {
        x = MIN(object.x, object.x + object.w);
        y = MIN(object.y, object.y + object.h);
        w = MAX(object.x, object.x + object.w) - x;
        h = MAX(object.y, object.y + object.h) - y;
    }
    ARect(const APoint& pt, const ASize& sz) {
        x = MIN(pt.x, pt.x + sz.w);
        y = MIN(pt.y, pt.y + sz.h);
        w = MAX(pt.x, pt.x + sz.w) - x;
        h = MAX(pt.y, pt.y + sz.h) - y;
    }
#ifdef _WIN32
    ARect(const RECT& rc) {
        x = rc.left; y = rc.top; w = rc.right - rc.left; h = rc.bottom - rc.top;
        if (w < 0) {x += w; w = -w;}
        if (h < 0) {y += h; h = -h;}
    }
#endif

    ARect& SetRect(int ix = 0, int iy = 0, int iw = 0, int ih = 0) {
        x = MIN(ix, ix + iw);
        y = MIN(iy, iy + ih);
        w = MAX(ix, ix + iw) - x;
        h = MAX(iy, iy + ih) - y;
        return *this;
    }
    ARect& SetRect(const APoint& pt, const ASize& sz) {
        x = MIN(pt.x, pt.x + sz.w);
        y = MIN(pt.y, pt.y + sz.h);
        w = MAX(pt.x, pt.x + sz.w) - x;
        h = MAX(pt.y, pt.y + sz.h) - y;
        return *this;
    }

    ARect& operator = (const ARect& object) {
        x = MIN(object.x, object.x + object.w);
        y = MIN(object.y, object.y + object.h);
        w = MAX(object.x, object.x + object.w) - x;
        h = MAX(object.y, object.y + object.h) - y;
        return *this;
    }

    ARect& operator = (const APoint& pt) {
        x = pt.x; y = pt.y;
        return *this;
    }

    ARect& operator += (const APoint& pt) {
        x += pt.x; y += pt.y;
        return *this;
    }

    ARect& operator -= (const APoint& pt) {
        x -= pt.x; y -= pt.y;
        return *this;
    }

    ARect& operator = (const ASize& sz) {
        w = sz.w; h = sz.h;
        return *this;
    }

    ARect& operator += (const ASize& sz) {
        w += sz.w; h += sz.h;
        return *this;
    }

    ARect& operator -= (const ASize& sz) {
        w -= sz.w; h -= sz.h;
        return *this;
    }

#ifdef _WIN32
    ARect& operator = (const RECT& rc) {
        x = rc.left; y = rc.top; w = rc.right - rc.left; h = rc.bottom - rc.top;
        if (w < 0) {x += w; w = -w;}
        if (h < 0) {y += h; h = -h;}
        return *this;
    }
#endif

    friend ARect operator + (const ARect& rect, const APoint& pt) {
        return ARect(rect.x + pt.x, rect.y + pt.y, rect.w, rect.h);
    }

    friend ARect operator - (const ARect& rect, const APoint& pt) {
        return ARect(rect.x - pt.x, rect.y - pt.y, rect.w, rect.h);
    }

    friend ARect operator + (const ARect& rect, const ASize& sz) {
        return ARect(rect.x, rect.y, rect.w + sz.w, rect.h + sz.h);
    }

    friend ARect operator - (const ARect& rect, const ASize& sz) {
        return ARect(rect.x, rect.y, rect.w - sz.w, rect.h - sz.h);
    }

    bool operator == (const ARect& rect) const {return ((x == rect.x) && (y == rect.y) && (w == rect.w) && (h == rect.h));}
    bool operator != (const ARect& rect) const {return !operator == (rect);}

#define RECT_COMPARE(op)                                                                                                                \
    friend bool operator op (const ARect&  rect, const APoint& pt)   {return ((rect.x op pt.x)   && (rect.y op pt.y));}     \
    friend bool operator op (const APoint& pt,   const ARect&  rect) {return ((pt.x   op rect.x) && (pt.y   op rect.y));}   \
                      bool operator op (const APoint& pt)   const               {return ((x      op pt.x)   && (y      op pt.y));}      \
    friend bool operator op (const ARect&  rect, const ASize&  sz)   {return ((rect.w op sz.w)   && (rect.h op sz.h));}     \
    friend bool operator op (const ASize&  sz,   const ARect&  rect) {return ((sz.w   op rect.w) && (sz.h   op rect.h));}   \
                      bool operator op (const ASize&  sz)   const               {return ((w      op sz.w)   && (h      op sz.h));}

    RECT_COMPARE(==);
    RECT_COMPARE(<=);
    RECT_COMPARE(>=);
    RECT_COMPARE(<);
    RECT_COMPARE(>);

    friend bool operator != (const ARect&  rect, const APoint& pt)   {return !rect.operator == (pt);}
    friend bool operator != (const APoint& pt,   const ARect&  rect) {return !rect.operator == (pt);}
                      bool operator != (const APoint& pt)   const               {return !     operator == (pt);}
    friend bool operator != (const ARect&  rect, const ASize&  sz)   {return !rect.operator == (sz);}
    friend bool operator != (const ASize&  sz,   const ARect&  rect) {return !rect.operator == (sz);}
                      bool operator != (const ASize&  sz)   const               {return !     operator == (sz);}

    friend ARect Min(const ARect& rect1, const ARect& rect2) {
        APoint pt1 = Min(rect1.TopLeft(),     rect2.TopLeft());
        APoint pt2 = Max(rect1.BottomRight(), rect2.BottomRight());
        pt2 = Max(pt1, pt2);
        return ARect(pt1, ASize(pt2.x, pt2.y) - ASize(pt1.x, pt1.y));
    }
    friend ARect Max(const ARect& rect1, const ARect& rect2) {
        APoint pt1 = Max(rect1.TopLeft(),     rect2.TopLeft());
        APoint pt2 = Min(rect1.BottomRight(), rect2.BottomRight());
        pt2 = Max(pt1, pt2);
        return ARect(pt1, ASize(pt2.x, pt2.y) - ASize(pt1.x, pt1.y));
    }

    ARect& Move(const APoint& pt) {
        x += pt.x; y += pt.y;
        return *this;
    }

    ARect& Move(int x1, int y1) {
        x += x1; y += y1;
        return *this;
    }

    ARect& Inflate(const ASize& sz) {
        x -= sz.w; y -= sz.h;
        w += sz.w * 2; h += sz.h * 2;
        return *this;
    }

    ARect& Inflate(int w1, int h1) {
        x -= w1; y -= h1;
        w += w1 * 2; h += h1 * 2;
        return *this;
    }

    ARect& Deflate(const ASize& sz) {
        x += sz.w; y += sz.h;
        w -= sz.w * 2; h -= sz.h * 2;
        return *this;
    }

    ARect& Deflate(int w1, int h1) {
        x += w1; y += h1;
        w -= w1 * 2; h -= h1 * 2;
        return *this;
    }

    ARect& MoveLeft(int n) {
        x += n; w -= n;
        return *this;
    }
    ARect& MoveTop(int n) {
        y += n; h -= n;
        return *this;
    }
    ARect& MoveRight(int n) {
        w += n;
        return *this;
    }
    ARect& MoveBottom(int n) {
        h += n;
        return *this;
    }

    APoint GetPosition() const           {return APoint(x, y);}
    void   SetPosition(const APoint& pt) {x = pt.x; y = pt.y;}
    void   SetPosition(int ix, int iy)   {x = ix;   y = iy;}
    ASize  GetSize() const               {return ASize(w, h);}
    void   SetSize(const ASize& sz)      {w = sz.w; h = sz.h;}
    void   SetSize(int iw, int ih)       {w = iw;   h = ih;}

    APoint TopLeft()     const {APoint pt(x,     y);     return pt;}
    APoint TopRight()    const {APoint pt(x + w, y);     return pt;}
    APoint BottomLeft()  const {APoint pt(x,     y + h); return pt;}
    APoint BottomRight() const {APoint pt(x + w, y + h); return pt;}

    APoint TopLeftIncl()     const {APoint pt(x,         y);         return pt;}
    APoint TopRightIncl()    const {APoint pt(x + w - 1, y);         return pt;}
    APoint BottomLeftIncl()  const {APoint pt(x,         y + h - 1); return pt;}
    APoint BottomRightIncl() const {APoint pt(x + w - 1, y + h - 1); return pt;}

    int left()   const {return x;}
    int right()  const {return x + w;}
    int top()    const {return y;}
    int bottom() const {return y + h;}

    APoint centre() const {return APoint(x + w / 2, y + h / 2);}

    bool PointInRect(const APoint& pt) const {
        return ((pt.x >= x) && (pt.x < (x + w)) && (pt.y >= y) && (pt.y <= (y + h)));
    }

    ARect& Limit(const ARect& rect, bool ShrinkSize = true) {
        x = MAX(x, rect.x);
        y = MAX(y, rect.y);

        w = MIN(w, rect.w);
        h = MIN(h, rect.h);

        x = MIN(x + w, rect.x + rect.w) - w;
        y = MIN(y + h, rect.y + rect.h) - h;

        if ((x + w) > (rect.x + rect.w)) {
            if (ShrinkSize) w = rect.x + rect.w - x;
            else            x = rect.x + rect.w - w;
        }
        if ((y + h) > (rect.y + rect.h)) {
            if (ShrinkSize) h = rect.y + rect.h - y;
            else            y = rect.y + rect.h - h;
        }

        return *this;
    }

    ARect& Limit(const ASize& sz) {
        if (w < sz.w) {
            int o = sz.w - w;
            x -= o / 2; w += o;
        }
        if (h < sz.h) {
            int o = sz.h - h;
            y -= o / 2; h += o;
        }
        return *this;
    }

    bool Overlap(const ARect& rect) const {
        return (((x + w) >= rect.x) && (x < (rect.x + rect.w)) &&
                ((y + h) >= rect.y) && (y < (rect.y + rect.h)));
    }

    bool Inside(const ARect& rect) const {
        return ((x >= rect.x)                  && (y >= rect.y) &&
                ((x + w) <= (rect.x + rect.w)) && ((y + h) <= (rect.y + rect.h)));
    }

    bool Inside(const APoint& pt) const {
        return ((pt.x >= x)      && (pt.y >= y) &&
                (pt.x < (x + w)) && (pt.y < (y + h)));
    }

    ARect Centre(const ASize& size) const {
        int x1, y1;

        x1 = x + (w - size.w) / 2;
        y1 = y + (h - size.h) / 2;

        return ARect(x1, y1, size.w, size.h);
    }

    ARect TopLeftAligned() const {
        ARect rect = *this;
        rect.x = rect.y = 0;
        return rect;
    }

    ARect& Include(const ARect& rect) {
        int x1 = x, y1 = y, x2 = x + w, y2 = y + h;

        x1 = MIN(x1, rect.x);            y1 = MIN(y1, rect.y);
        x2 = MAX(x2, (rect.x + rect.w)); y2 = MAX(y2, (rect.y + rect.h));
        x  = MIN(x1, x2);                y  = MIN(y1, y2);
        w  = MAX(x1, x2) - x;            h  = MAX(y1, y2) - y;

        return *this;
    }

    ARect& Centralise(const ASize& sz) {
        x += (w - sz.w) / 2;
        y += (h - sz.h) / 2;
        w = sz.w;
        h = sz.h;

        return *this;
    }

    ARect Justify(const ASize& sz, uint8_t HJust = JUSTIFY_CENTRE, uint8_t VJust = JUSTIFY_CENTRE) const {
        ARect rect(x, y, sz.w, sz.h);

        if (HJust == JUSTIFY_FULLY) rect.w  = w;
        else                        rect.x += ::Justify(rect.w, w, HJust);
        if (VJust == JUSTIFY_FULLY) rect.h  = h;
        else                        rect.y += ::Justify(rect.h, h, VJust);

        return rect;
    }

    operator APoint () const {
        return APoint(x, y);
    }
    operator ASize () const {
        return ASize(w, h);
    }

#ifdef _WIN32
    operator RECT () const {
        RECT rc;
        rc.left = x; rc.top = y; rc.right = x + w; rc.bottom = y + h;
        return rc;
    }
#endif

#ifdef __STRING_SUPPORT__
    AString ToText() const {
        AString String;
        String.Format("%d %d %d %d", x, y, w, h);
        return String;
    }
    void FromText(const AString& text) {
        SetRect((int)text.Word(0), (int)text.Word(1), (int)text.Word(2), (int)text.Word(3));
    }
#endif

    int x, y, w, h;
};

typedef struct {
    uint8_t b, g, r;
} BGR_PIXEL;
typedef struct {
    uint8_t b, g, r, a;
} BGRA_PIXEL;
typedef struct {
    uint8_t b, g, r;
    uint8_t pad;
} BGRP_PIXEL;
typedef struct {
    uint8_t r, g, b;
} RGB_PIXEL;
typedef struct {
    uint8_t r, g, b, a;
} RGBA_PIXEL;
typedef struct {
    uint8_t r, g, b;
    uint8_t pad;
} RGBP_PIXEL;

inline uint32_t SwapColour(uint32_t col) {return ((col & 0xff) << 16) | (col & 0xff00) | ((col & 0xff0000) >> 16);}

class AColour {
public:
    AColour(uint8_t ir = 0, uint8_t ig = 0, uint8_t ib = 0, uint8_t ia = 0) {r = ir; g = ig; b = ib; a = ia;}
    AColour(uint32_t col) {r = (uint8_t)(col >> 16); g = (uint8_t)(col >> 8); b = (uint8_t)col; a = (uint8_t)(col >> 24);}
    AColour(BGR_PIXEL  pix) {r = pix.r; g = pix.g; b = pix.b; a = 0;}
    AColour(BGRA_PIXEL pix) {r = pix.r; g = pix.g; b = pix.b; a = pix.a;}
    AColour(BGRP_PIXEL pix) {r = pix.r; g = pix.g; b = pix.b; a = 0;}
    AColour(RGB_PIXEL  pix) {r = pix.r; g = pix.g; b = pix.b; a = 0;}
    AColour(RGBA_PIXEL pix) {r = pix.r; g = pix.g; b = pix.b; a = pix.a;}
    AColour(RGBP_PIXEL pix) {r = pix.r; g = pix.g; b = pix.b; a = 0;}
    AColour(const AColour& col, int scale = 0xff);
    AColour(const char *pText);

    AColour& operator = (uint32_t col) {
        r = (uint8_t)(col >> 16); g = (uint8_t)(col >> 8); b = (uint8_t)col; a = (uint8_t)(col >> 24);
        return *this;
    }
    AColour& operator = (BGR_PIXEL pix) {
        r = pix.r; g = pix.g; b = pix.b; a = 0;
        return *this;
    }
    AColour& operator = (BGRP_PIXEL pix) {
        r = pix.r; g = pix.g; b = pix.b; a = 0;
        return *this;
    }
    AColour& operator = (BGRA_PIXEL pix) {
        r = pix.r; g = pix.g; b = pix.b; a = pix.a;
        return *this;
    }
    AColour& operator = (RGB_PIXEL pix) {
        r = pix.r; g = pix.g; b = pix.b; a = 0;
        return *this;
    }
    AColour& operator = (RGBA_PIXEL pix) {
        r = pix.r; g = pix.g; b = pix.b; a = pix.a;
        return *this;
    }
    AColour& operator = (RGBP_PIXEL pix) {
        r = pix.r; g = pix.g; b = pix.b; a = 0;
        return *this;
    }
    AColour& operator = (const AString& text) {FromText(text); return *this;}
    AColour& operator = (const char    *text) {FromText(AString(text)); return *this;}

    AColour& SetColour(uint8_t ir = 0, uint8_t ig = 0, uint8_t ib = 0, uint8_t ia = 0) {
        r = ir; g = ig; b = ib; a = ia;
        return *this;
    }

    AColour& SetColour(const AColour& col, int scale = 0xff);

    AColour& Maximize();

    AColour& Mix(const AColour& col1, const AColour& col2, int scale1 = 0x80, int scale2 = 0x7f);

    AColour& operator += (AColour col) {
        r = (uint8_t)addmu(r, col.r, 255);
        g = (uint8_t)addmu(g, col.g, 255);
        b = (uint8_t)addmu(b, col.b, 255);
        a = (uint8_t)addmu(a, col.a, 255);
        return *this;
    }
    AColour& operator -= (AColour col) {
        r = (uint8_t)subzu(r, col.r);
        g = (uint8_t)subzu(g, col.g);
        b = (uint8_t)subzu(b, col.b);
        a = (uint8_t)subzu(a, col.a);
        return *this;
    }
    AColour& operator *= (AColour col) {
        r = muldivb(r, col.r);
        g = muldivb(g, col.g);
        b = muldivb(b, col.b);
        a = muldivb(a, col.a);
        return *this;
    }
    AColour& operator *= (int n) {
        r = muldivb(r, n);
        g = muldivb(g, n);
        b = muldivb(b, n);
        a = muldivb(a, n);
        return *this;
    }
    AColour& operator *= (double n) {
        double r1 = (double)r * n + .5;
        double g1 = (double)g * n + .5;
        double b1 = (double)b * n + .5;
        double a1 = (double)a * n + .5;
        r = (uint8_t)LIMIT(r1, 0.0, 255.0);
        g = (uint8_t)LIMIT(g1, 0.0, 255.0);
        b = (uint8_t)LIMIT(b1, 0.0, 255.0);
        a = (uint8_t)LIMIT(a1, 0.0, 255.0);
        return *this;
    }
    AColour& operator /= (double n) {
        double r1 = (double)r / n + .5;
        double g1 = (double)g / n + .5;
        double b1 = (double)b / n + .5;
        double a1 = (double)a / n + .5;
        r = (uint8_t)LIMIT(r1, 0.0, 255.0);
        g = (uint8_t)LIMIT(g1, 0.0, 255.0);
        b = (uint8_t)LIMIT(b1, 0.0, 255.0);
        a = (uint8_t)LIMIT(a1, 0.0, 255.0);
        return *this;
    }

    friend AColour operator + (AColour col1, AColour col2) {
        AColour col;
        col.r = (uint8_t)addmu(col1.r, col2.r, 255);
        col.g = (uint8_t)addmu(col1.g, col2.g, 255);
        col.b = (uint8_t)addmu(col1.b, col2.b, 255);
        col.a = (uint8_t)addmu(col1.a, col2.a, 255);
        return col;
    }
    friend AColour operator - (AColour col1, AColour col2) {
        AColour col;
        col.r = (uint8_t)subzu(col1.r, col2.r);
        col.g = (uint8_t)subzu(col1.g, col2.g);
        col.b = (uint8_t)subzu(col1.b, col2.b);
        col.a = (uint8_t)subzu(col1.a, col2.a);
        return col;
    }
    friend AColour operator * (AColour col1, AColour col2) {
        AColour col;
        col.r = muldivb(col1.r, col2.r);
        col.g = muldivb(col1.g, col2.g);
        col.b = muldivb(col1.b, col2.b);
        col.a = muldivb(col1.a, col2.a);
        return col;
    }
    friend AColour operator * (AColour col1, int n) {
        AColour col;
        col.r = muldivb(col1.r, n);
        col.g = muldivb(col1.g, n);
        col.b = muldivb(col1.b, n);
        col.a = muldivb(col1.a, n);
        return col;
    }
    friend AColour operator * (AColour col1, double n) {
        AColour col;
        double r1 = (double)col1.r * n + .5;
        double g1 = (double)col1.g * n + .5;
        double b1 = (double)col1.b * n + .5;
        double a1 = (double)col1.a * n + .5;
        col.r = (uint8_t)LIMIT(r1, 0.0, 255.0);
        col.g = (uint8_t)LIMIT(g1, 0.0, 255.0);
        col.b = (uint8_t)LIMIT(b1, 0.0, 255.0);
        col.a = (uint8_t)LIMIT(a1, 0.0, 255.0);
        return col;
    }
    friend AColour operator / (AColour col1, double n) {
        AColour col;
        double r1 = (double)col1.r / n + .5;
        double g1 = (double)col1.g / n + .5;
        double b1 = (double)col1.b / n + .5;
        double a1 = (double)col1.a / n + .5;
        col.r = (uint8_t)LIMIT(r1, 0.0, 255.0);
        col.g = (uint8_t)LIMIT(g1, 0.0, 255.0);
        col.b = (uint8_t)LIMIT(b1, 0.0, 255.0);
        col.a = (uint8_t)LIMIT(a1, 0.0, 255.0);
        return col;
    }

#ifdef __STRING_SUPPORT__
    AString ToText() const {
        return AValue(operator uint32_t()).ToString("$08x");
    }
    void FromText(const AString& text) {operator = ((uint32_t)text);}
#endif

    operator uint32_t() const {return (((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b | ((uint32_t)a << 24));}

    operator BGR_PIXEL()  const {BGR_PIXEL  pix; pix.r = r; pix.g = g; pix.b = b;              return pix;}
    operator BGRA_PIXEL() const {BGRA_PIXEL pix; pix.r = r; pix.g = g; pix.b = b; pix.a   = a; return pix;}
    operator BGRP_PIXEL() const {BGRP_PIXEL pix; pix.r = r; pix.g = g; pix.b = b; pix.pad = 0; return pix;}
    operator RGB_PIXEL()  const {RGB_PIXEL  pix; pix.r = r; pix.g = g; pix.b = b;              return pix;}
    operator RGBA_PIXEL() const {RGBA_PIXEL pix; pix.r = r; pix.g = g; pix.b = b; pix.a   = a; return pix;}
    operator RGBP_PIXEL() const {RGBP_PIXEL pix; pix.r = r; pix.g = g; pix.b = b; pix.pad = a; return pix;}

    uint8_t r, g, b, a;
};

#endif
