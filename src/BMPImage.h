
#ifndef __BMP_IMAGE__
#define __BMP_IMAGE__

#ifdef _WIN32
#include "windows.h"
#endif

#include "PointSizeRect.h"
#include "tag.h"
#include "3DTrans.h"

#define BI_RGB 0

#ifndef _WIN32
typedef PACKEDSTRUCT {
	uint32_t	biSize;
	sint32_t	biWidth;
	sint32_t	biHeight;
	uint16_t	biPlanes;
	uint16_t	biBitCount;
	uint32_t	biCompression;
	uint32_t	biSizeImage;
	sint32_t	biXPelsPerMeter;
	sint32_t	biYPelsPerMeter;
	uint32_t	biClrUsed;
	uint32_t	biClrImportant;
} BITMAPINFOHEADER;

typedef PACKEDSTRUCT  {
	uint8_t	rgbBlue;
	uint8_t	rgbGreen;
	uint8_t	rgbRed;
	uint8_t	rgbReserved;
} RGBQUAD;

typedef PACKEDSTRUCT  {
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[1];
} BITMAPINFO;

typedef long FXPT16DOT16,*LPFXPT16DOT16;
typedef long FXPT2DOT30,*LPFXPT2DOT30;

typedef PACKEDSTRUCT  {
	FXPT2DOT30 ciexyzX;
	FXPT2DOT30 ciexyzY;
	FXPT2DOT30 ciexyzZ;
} CIEXYZ;

typedef PACKEDSTRUCT {
	CIEXYZ ciexyzRed;
	CIEXYZ ciexyzGreen;
	CIEXYZ ciexyzBlue;
} CIEXYZTRIPLE;

typedef PACKEDSTRUCT {
	uint32_t	bV4Size;
	sint32_t	bV4Width;
	sint32_t	bV4Height;
	uint16_t	bV4Planes;
	uint16_t	bV4BitCount;
	uint32_t	bV4V4Compression;
	uint32_t	bV4SizeImage;
	sint32_t	bV4XPelsPerMeter;
	sint32_t	bV4YPelsPerMeter;
	uint32_t	bV4ClrUsed;
	uint32_t	bV4ClrImportant;
	uint32_t	bV4RedMask;
	uint32_t	bV4GreenMask;
	uint32_t	bV4BlueMask;
	uint32_t	bV4AlphaMask;
	uint32_t	bV4CSType;
	CIEXYZTRIPLE bV4Endpoints;
	uint32_t	bV4GammaRed;
	uint32_t	bV4GammaGreen;
	uint32_t	bV4GammaBlue;
} BITMAPV4HEADER;

typedef PACKEDSTRUCT {
	uint16_t	bfType;
	uint32_t	bfSize;
	uint16_t	bfReserved1;
	uint16_t	bfReserved2;
	uint32_t	bfOffBits;
} BITMAPFILEHEADER;
#endif

typedef PACKEDSTRUCT {
	BITMAPV4HEADER bmiHeader;
	RGBQUAD        bmiColors[1];
} BITMAPV4INFO;

#define LOC(x,y) (pPixData - (y) * Rect.w + (x))

class AImage;
class APixelHandler {
public:
	APixelHandler(AImage *image = NULL);
	virtual ~APixelHandler();

	typedef BGRP_PIXEL PIXEL;

	virtual void Attach(AImage *image);
	virtual void Detach();

	bool IsAttached() const {return (pImage != NULL);}

	virtual void Begin() {}
	virtual void End() {}

	const ARect& GetRect() const {return Rect;}
	AImage *GetImage()     const {return pImage;}

protected:
	friend class AImage;
	inline bool LocationValid(int x, int y) const {
		x -= Rect.x; y -= Rect.y;
		return (pPixData && (x >= 0) && (y >= 0) && (x < Rect.w) && (y < Rect.h));
	}
	inline PIXEL *GetLocation(int x, int y) const {
		x -= Rect.x; y -= Rect.y;
		return LocationValid(x, y) ? LOC(x, y) : NULL;
	}
	inline PIXEL *GetLocationEx(int x, int y) const {
		x -= Rect.x; y -= Rect.y;
		return LOC(x, y);
	}
	inline PIXEL *GetLocationTiled(int x, int y) const {
		x -= Rect.x; y -= Rect.y;
		x %= Rect.w; if (x < 0) x += Rect.w;
		y %= Rect.h; if (y < 0) y += Rect.h;
		return LOC(x, y);
	}
	inline PIXEL *GetLocationLimited(int x, int y) const {
		x -= Rect.x; y -= Rect.y;
		x = MAX(x, 0); x = MIN(x, Rect.w - 1);
		y = MAX(y, 0); y = MIN(y, Rect.h - 1);
		return LOC(x, y);
	}

protected:
	AImage *pImage;
	PIXEL  *pPixData;
	PIXEL  BlankColour;
	ARect  Rect;
};

class APixelWriter : public APixelHandler {
public:
	APixelWriter(AImage *image = NULL) : APixelHandler(image),
										 pChild(NULL) {}
	virtual ~APixelWriter() {}

	virtual void Attach(AImage *image) {APixelHandler::Attach(image); if (pChild) pChild->Attach(image);}
	virtual void Detach() {if (pChild) pChild->Detach(); APixelHandler::Detach();}

	virtual void Begin() {APixelHandler::Begin(); if (pChild) pChild->Begin();}
	virtual void End()   {if (pChild) pChild->End(); APixelHandler::End();}

	virtual APixelWriter *SetChild(APixelWriter *child) {APixelWriter *oldchild = pChild; pChild = child; if (pChild && IsAttached()) pChild->Attach(pImage); return oldchild;}

	virtual void Plot(int x, int y, PIXEL col)   {
		if      (pChild) pChild->Plot(x, y, col);
		else if (LocationValid(x, y)) *GetLocationEx(x, y) = col;
	}
	virtual void PlotEx(int x, int y, PIXEL col) {
		if (pChild) pChild->PlotEx(x, y, col);
		else		*GetLocationEx(x, y) = col;
	}

protected:
	APixelWriter *pChild;
};

class APixelReader : public APixelHandler {
public:
	APixelReader(AImage *image = NULL) : APixelHandler(image),
										 pChild(NULL) {}
	virtual ~APixelReader() {}

	virtual void Attach(AImage *image) {APixelHandler::Attach(image); if (pChild) pChild->Attach(image);}
	virtual void Detach() {if (pChild) pChild->Detach(); APixelHandler::Detach();}

	virtual void Begin() {APixelHandler::Begin(); if (pChild) pChild->Begin();}
	virtual void End()   {if (pChild) pChild->End(); APixelHandler::End();}

	virtual APixelReader *SetChild(APixelReader *child) {APixelReader *oldchild = pChild; pChild = child; if (pChild && IsAttached()) pChild->Attach(pImage); return oldchild;}

	virtual PIXEL ReadPixel(int x, int y)        const {return pChild ? pChild->ReadPixel(x, y)        : (LocationValid(x, y) ? *GetLocationEx(x, y) : BlankColour);}
	virtual PIXEL ReadPixelEx(int x, int y)      const {return pChild ? pChild->ReadPixelEx(x, y)      : *GetLocationEx(x, y);}
	virtual PIXEL ReadPixelTiled(int x, int y)   const {return pChild ? pChild->ReadPixelTiled(x, y)   : *GetLocationTiled(x, y);}
	virtual PIXEL ReadPixelLimited(int x, int y) const {return pChild ? pChild->ReadPixelLimited(x, y) : *GetLocationLimited(x, y);}

#define READ_PIXEL_FLOAT(name)															\
	virtual PIXEL name(float x, float y, float rnd = .5f) const {						\
		const int x1 = (int)x, x2 = x1 + 1;												\
		const int y1 = (int)y, y2 = y1 + 1;												\
		const float xfb = x - floorf(x), xfa = 1.0f - xfb;								\
		const float yfb = y - floorf(y), yfa = 1.0f - yfb;								\
		const float l1 = xfa * yfa;														\
		const float l2 = xfb * yfa;														\
		const float l3 = xfa * yfb;														\
		const float l4 = xfb * yfb;														\
		const PIXEL pix1 = name(x1, y1);												\
		const PIXEL pix2 = name(x2, y1);												\
		const PIXEL pix3 = name(x1, y2);												\
		const PIXEL pix4 = name(x2, y2);												\
		PIXEL pix;																		\
		pix.pad = 0;																	\
		pix.r = (uint8_t)(l1 * pix1.r + l2 * pix2.r + l3 * pix3.r + l4 * pix4.r + rnd);	\
		pix.g = (uint8_t)(l1 * pix1.g + l2 * pix2.g + l3 * pix3.g + l4 * pix4.g + rnd);	\
		pix.b = (uint8_t)(l1 * pix1.b + l2 * pix2.b + l3 * pix3.b + l4 * pix4.b + rnd);	\
		return pix;																		\
	}																					\
	virtual PIXEL name(double x, double y, double rnd = .5) const {						\
		const int x1 = (int)x, x2 = x1 + 1;												\
		const int y1 = (int)y, y2 = y1 + 1;												\
		const double xfb = x - floor(x), xfa = 1.0 - xfb;								\
		const double yfb = y - floor(y), yfa = 1.0 - yfb;								\
		const double l1 = xfa * yfa;													\
		const double l2 = xfb * yfa;													\
		const double l3 = xfa * yfb;													\
		const double l4 = xfb * yfb;													\
		const PIXEL  pix1 = name(x1, y1);												\
		const PIXEL  pix2 = name(x2, y1);												\
		const PIXEL  pix3 = name(x1, y2);												\
		const PIXEL  pix4 = name(x2, y2);												\
		PIXEL pix;																		\
		pix.pad = 0;																	\
		pix.r = (uint8_t)(l1 * pix1.r + l2 * pix2.r + l3 * pix3.r + l4 * pix4.r + rnd);	\
		pix.g = (uint8_t)(l1 * pix1.g + l2 * pix2.g + l3 * pix3.g + l4 * pix4.g + rnd);	\
		pix.b = (uint8_t)(l1 * pix1.b + l2 * pix2.b + l3 * pix3.b + l4 * pix4.b + rnd);	\
		return pix;																		\
	}
	READ_PIXEL_FLOAT(ReadPixel);
	READ_PIXEL_FLOAT(ReadPixelTiled);
	READ_PIXEL_FLOAT(ReadPixelLimited);
#undef READ_PIXEL_FLOAT

	enum {
		SCALE_MODE_FAST = 0,
		SCALE_MODE_BEST,
	};
	virtual PIXEL ReadPixelScaled(double x, double y, double w, double h, uint_t mode = SCALE_MODE_BEST) const;

protected:
	APixelReader *pChild;
};

class APixelReaderWriter : public APixelWriter {
public:
	APixelReaderWriter(AImage *image = NULL) : APixelWriter(image),
											   pReader(NULL) {
	}
	virtual ~APixelReaderWriter() {}

	virtual APixelReader *SetReader(APixelReader *reader) {APixelReader *oldreader = pReader; pReader = reader; return oldreader;}

	virtual void Plot(int x, int y, PIXEL col)   {APixelWriter::Plot(x, y, pReader ? pReader->ReadPixel(x, y) : col);}
	virtual void PlotEx(int x, int y, PIXEL col) {APixelWriter::PlotEx(x, y, pReader ? pReader->ReadPixel(x, y) : col);}

protected:
	APixelReader *pReader;
};

class AMatrixPixelReader : public APixelReader {
public:
	AMatrixPixelReader(AImage *image = NULL) : APixelReader(image) {
		memset(Matrix, 0, sizeof(Matrix));
		Matrix[0][0] = 1; Matrix[1][1] = 1;
		DivVal = 1; RndVal = 0;
	}
	virtual ~AMatrixPixelReader() {}

	void SetMatrix(const int mat[2][2], int divval = 1, int rndval = 0) {memcpy(Matrix, mat, sizeof(Matrix)); DivVal = divval; RndVal = rndval;}
	void SetDivisor(int divval = 1) {DivVal = divval;}
	int  GetDivisor() const {return DivVal;}
	void SetRoundValue(int rndval = 0) {RndVal = rndval;}
	int  GetRoundValue() const {return RndVal;}

	void GetMatrix(int mat[2][2], int *divval = NULL, int *rndval = NULL) {
		memcpy(mat, Matrix, sizeof(Matrix));
		if (divval) *divval = DivVal;
		if (rndval) *rndval = RndVal;
	}
	void SetOrigin(const APoint& origin) {Origin = origin;}
	const APoint& GetOrigin() const {return Origin;}
	
	void ApplyMatrix(const int mat[2][2], int divval = 1, int rndval = 0) {
		const int a = Matrix[0][0] * mat[0][0] + Matrix[0][1] * mat[1][0];
		const int b = Matrix[0][0] * mat[0][1] + Matrix[0][1] * mat[1][1];
		const int c = Matrix[1][0] * mat[0][0] + Matrix[1][1] * mat[1][0];
		const int d = Matrix[1][0] * mat[0][1] + Matrix[1][1] * mat[1][1];
		Matrix[0][0] = (a + rndval) / divval; Matrix[0][1] = (b + rndval) / divval;
		Matrix[1][0] = (c + rndval) / divval; Matrix[1][1] = (d + rndval) / divval;
	}

	virtual PIXEL ReadPixel(int x, int y)        const {Apply(x, y); return APixelReader::ReadPixel(x, y);}
	virtual PIXEL ReadPixelEx(int x, int y)      const {Apply(x, y); return APixelReader::ReadPixelEx(x, y);}
	virtual PIXEL ReadPixelTiled(int x, int y)   const {Apply(x, y); return APixelReader::ReadPixelTiled(x, y);}
	virtual PIXEL ReadPixelLimited(int x, int y) const {Apply(x, y); return APixelReader::ReadPixelLimited(x, y);}

	enum {
		ORIENTATION_TOP_LEFT = 0,
		ORIENTATION_TOP_RIGHT,
		ORIENTATION_BOTTOM_LEFT,
		ORIENTATION_BOTTOM_RIGHT,
		ORIENTATION_TOP_LEFT_ROTATED,
		ORIENTATION_TOP_RIGHT_ROTATED,
		ORIENTATION_BOTTOM_LEFT_ROTATED,
		ORIENTATION_BOTTOM_RIGHT_ROTATED,

		ORIENTATION_MODES,
	};

	void SetOrientation(uint_t mode);
	void FlipHorizontally() {Matrix[0][0] = -Matrix[0][0]; Matrix[0][1] = -Matrix[0][1]; Origin.x = Rect.w - 1 - Origin.x;}
	void FlipVertically()   {Matrix[1][0] = -Matrix[1][0]; Matrix[1][1] = -Matrix[1][1]; Origin.y = Rect.h - 1 - Origin.y;}
	void RotateLeft() {
		const int mat[2][2] = {
			{ 0,  1},
			{-1,  0},
		};
		ApplyMatrix(mat);
	}
	void RotateRight() {
		const int mat[2][2] = {
			{ 0, -1},
			{ 1,  0},
		};
		ApplyMatrix(mat);
	}

	ARect CalcMatrixRect() const;

	void Apply(int& x, int& y) const {
		const int x1 = x * Matrix[0][0] + y * Matrix[0][1] + RndVal;
		const int y1 = x * Matrix[1][0] + y * Matrix[1][1] + RndVal;
		x = x1 / DivVal + Origin.x; y = y1 / DivVal + Origin.y;
	}
#if 0
	void Remove(int& x, int& y, bool fromOrigin = true) const {
		if (fromOrigin) x -= Origin.x; y -= Origin.y;
		x *= DivVal; y *= DivVal;
		const int a   = Matrix[0][0];
		const int b   = Matrix[0][1];
		const int c   = Matrix[1][0];
		const int d   = Matrix[1][1];
		const int det = abs(a * d - b * c);
		const int x1  = x * d + y * c + det / 2;
		const int y1  = x * b + y * a + det / 2;
		x = x1 / det; y = y1 / det;
	}
#endif

protected:
	int    Matrix[2][2];
	int    DivVal,  RndVal;
	APoint Origin;
};

class AFMatrixPixelReader : public APixelReader {
public:
	AFMatrixPixelReader(AImage *image = NULL) : APixelReader(image) {
		memset(Matrix, 0, sizeof(Matrix));
		Matrix[0][0] = 1.0; Matrix[1][1] = 1.0;
	}
	virtual ~AFMatrixPixelReader() {}

	void SetMatrix(const double mat[2][2]) {memcpy(Matrix, mat, sizeof(Matrix));}

#define READ_PIXEL_FLOAT(name)																\
	virtual PIXEL name(float x, float y, float rnd = .5f) const {							\
		float x1 = (float)(x * Matrix[0][0] + y * Matrix[0][1]);							\
		float y1 = (float)(x * Matrix[1][0] + y * Matrix[1][1]);							\
		return APixelReader::name(x1, y1, rnd);												\
	}																						\
	virtual PIXEL name(double x, double y, double rnd = .5f) const {						\
		double x1 = x * Matrix[0][0] + y * Matrix[0][1];									\
		double y1 = x * Matrix[1][0] + y * Matrix[1][1];									\
		return APixelReader::name(x1, y1, rnd);												\
	}
	READ_PIXEL_FLOAT(ReadPixel);
	READ_PIXEL_FLOAT(ReadPixelTiled);
	READ_PIXEL_FLOAT(ReadPixelLimited);
#undef READ_PIXEL_FLOAT

protected:
	double Matrix[2][2];
};

class AImage {
public:
	AImage();
	AImage(const AImage& img);
	virtual ~AImage();

	virtual AImage& operator = (const AImage& img);
	virtual bool    operator == (const AImage& img) const;
	virtual bool    operator != (const AImage& img) const {return !operator == (img);}
	
	virtual bool Create(int Width, int Height);
	virtual void Delete();

	virtual bool Valid() const {return (pData != NULL);}

	virtual void Clear(const AColour& col = AColour("$000000"));

	typedef BGRP_PIXEL PIXEL;

	enum {
		TAG_FILETYPE = _TAG_START,
		TAG_JPEG_QUALITY,
	};
		
	enum {
		FILETYPE_BMP = 0,
		FILETYPE_JPEG,
	};

	virtual AColour SetColour(const AColour& col) {AColour oldcol = Colour; Colour = col; return oldcol;}

	virtual void SetBlankColour(const AColour& col) {BlankColour = col;}

	APixelWriter *GetWriter() const {return pWriter;}
	APixelReader *GetReader() const {return pReader;}

	APixelWriter *SetWriter(APixelWriter *writer) {
		APixelWriter *oldwriter = pWriter;
		pWriter = writer ? writer : &DefaultWriter;
		if (Valid()) pWriter->Attach(this);
		return oldwriter;
	}

	APixelReader *SetReader(APixelReader *reader) {
		APixelReader *oldreader = pReader;
		pReader = reader ? reader : &DefaultReader; 
		if (Valid()) pReader->Attach(this);
		return oldreader;
	}

	inline void Plot(int x, int y, const AColour& col)     {BeginPlot(); pWriter->Plot(x,    y,    col); 	EndPlot();}
	inline void Plot(const APoint& pt, const AColour& col) {BeginPlot(); pWriter->Plot(pt.x, pt.y, col); 	EndPlot();}
	inline void Plot(int x, int y)     					   {BeginPlot(); pWriter->Plot(x,    y,    Colour); EndPlot();}
	inline void Plot(const APoint& pt) 					   {BeginPlot(); pWriter->Plot(pt.x, pt.y, Colour); EndPlot();}

	AColour ReadPixel(int x, int y)     	   const {return pReader->ReadPixel(x, y);}
	AColour ReadPixel(const APoint& pt) 	   const {return pReader->ReadPixel(pt.x, pt.y);}
	AColour ReadPixelTiled(int x, int y)       const {return pReader->ReadPixelTiled(x, y);}
	AColour ReadPixelTiled(const APoint& pt)   const {return pReader->ReadPixelTiled(pt.x, pt.y);}
	AColour ReadPixelLimited(int x, int y)     const {return pReader->ReadPixelLimited(x, y);}
	AColour ReadPixelLimited(const APoint& pt) const {return pReader->ReadPixelLimited(pt.x, pt.y);}

	AColour ReadPixel(float  x, float  y, float  rnd = .5) 		  const {return pReader->ReadPixel(x, y, rnd);}
	AColour ReadPixel(double x, double y, double rnd = .5) 		  const {return pReader->ReadPixel(x, y, rnd);}
	AColour ReadPixelTiled(float  x, float  y, float  rnd = .5)   const {return pReader->ReadPixelTiled(x, y, rnd);}
	AColour ReadPixelTiled(double x, double y, double rnd = .5)   const {return pReader->ReadPixelTiled(x, y, rnd);}
	AColour ReadPixelLimited(float  x, float  y, float  rnd = .5) const {return pReader->ReadPixelLimited(x, y, rnd);}
	AColour ReadPixelLimited(double x, double y, double rnd = .5) const {return pReader->ReadPixelLimited(x, y, rnd);}

	void Line(int x1, int y1, int x2, int y2);
	void Line(const APoint& pt1, const APoint& pt2) {Line(pt1.x, pt1.y, pt2.x, pt2.y);}
	void Box(int x1, int y1, int x2, int y2);
	void Box(const ARect& rect) {Box(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);}

	void Blit(int x1, int y1, int x2, int y2, APixelReader& reader, int x3, int y3, bool tile = false);
	void Blit(const ARect& rect, APixelReader& reader, const APoint& pt, bool tile = false) {
		Blit(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, reader, pt.x, pt.y, tile);
	}

	void MatrixBlit(const ARect& rect, APixelReader& reader, const APoint& offset, const double mat[2][2]);

	typedef struct {
		double   xy, yz, xz;
		struct {double x, y, z;} Offset;
		double 	 dist, perspective;
		double   scale;
	} TRANSFORM;

	void TransformBlit(const ARect& rect, APixelReader& reader, const ARect& rect1, const TRANSFORM& transform, bool drawborder = false, const AColour& borderColour = AColour());

	typedef struct {
		APixelReader *reader;
		int   x1, y1, x2, y2, xc, yc;
		A3DMatrix inverse;
		double normalx, normaly, normalz;
		double xs1, ys1, xs2, ys2;
		double dist, perspective;
		double cp, dcp, scale;
		double offsetx, offsety;
		struct {
			double m, c;
			int    y1, y2;
			bool   valid;
		} border[4];
		struct {int x, y;} corner[4];
	} TRANSFORM_DATA;

	bool TransformConfig(const ARect& rect, APixelReader& reader, const ARect& rect1, const TRANSFORM& transform, TRANSFORM_DATA& data);
	void TransformXLimits(const TRANSFORM_DATA& data, int y, int& x1, int& x2);
	void TransformBlit(TRANSFORM_DATA& data, bool drawborder = false, const AColour& borderColour = AColour());
	bool TransformPoint(const TRANSFORM_DATA& data, int x, int y, double& xf, double& yf) {
		return Transform(data, x, y, xf, yf);
	}

	int  SetTransformOptimization(int n) {int oldvalue = TransformOptimization; TransformOptimization = n; return oldvalue;}
	bool EnableTransformInterpolation(bool enable = true) {bool oldmode = TransformInterpolation; TransformInterpolation = enable; return oldmode;}

	void ScaleBlit(const ARect& rect1, APixelReader& reader, const ARect& rect2, uint_t mode = APixelReader::SCALE_MODE_BEST);

	virtual bool Load(const char *filename);
	virtual bool Save(const char *filename, const TAG *tags = NULL);
	virtual bool Save(AStdData& fp, const TAG *tags = NULL);

	operator APixelWriter& () {return *pWriter;}
	operator APixelWriter *() {return pWriter;}
	operator APixelReader& () {return *pReader;}
	operator APixelReader *() {return pReader;}

	const ARect& GetRect() const {return Rect;}

	const PIXEL *GetPixelData() const {return pData;}
	PIXEL *GetPixelData() {return pData;}

	PIXEL GetBlankColour() const {return BlankColour;}

	bool LoadJPEG(const char *filename);
	bool LoadJPEG(AStdData& fp);
	bool SaveJPEG(const char *filename, const TAG *tags);
	bool SaveJPEG(AStdData& fp, const TAG *tags);

	bool LoadBMP(const char *filename);
	bool LoadBMP(AStdData& fp);

protected:
	bool CreateData();
	void DeleteData();

	virtual void BeginPlot() {pWriter->Begin();}
	virtual void EndPlot()   {pWriter->End();}

	typedef struct {
		int x, y;
		int xadd_gt, yadd_gt;
		int xadd_sm, yadd_sm;
		int greater, smaller;
		int step;
		int gt_count, sm_count;
	} LINE_INFO;

	bool GenLineInfo(int x1, int y1, int x2, int y2, LINE_INFO& info) const;
	inline bool NextLinePoint(LINE_INFO& info) const {
		info.x += info.xadd_gt; info.y += info.yadd_gt; info.step -= info.smaller;
		if (info.step < 0) {
			if ((--info.sm_count) < 0) return false;
			info.x += info.xadd_sm; info.y += info.yadd_sm; info.step += info.greater;
		}
		return ((--info.gt_count) > 0);
	}

	bool CopyPixelData(PIXEL *dst, const APoint& dstpt, const ASize& dstsz, const PIXEL *src, const APoint& srcpt, const ASize& srcsz, const ASize& copysz);

	inline bool Transform(const TRANSFORM_DATA& data, int x, int y, double& xf, double& yf) {
		double px, py, pz;
		double div, t;

		//BREAKPOINT();

		px = (double)(x - data.xc) * data.scale;
		py = (double)(y - data.yc) * data.scale;

		div = data.normalx * px + data.normaly * py + data.cp;
		if (div == 0.0) return false;

		t   = data.dcp / div;
		pz  = t * data.perspective - data.dist;
		px *= t;
		py *= t;
		data.inverse.RotateXY(px, py, pz);
		
		xf = px + data.offsetx;
		yf = py + data.offsety;
			
		return true;
	}

protected:
	BITMAPFILEHEADER Header;
	BITMAPV4INFO	 Info;

	ARect Rect;

	PIXEL *pData;
	PIXEL Colour, BlankColour;

	APixelWriter *pWriter, DefaultWriter;
	APixelReader *pReader, DefaultReader;
	
	int  TransformOptimization;
	bool TransformInterpolation;
};

#endif
