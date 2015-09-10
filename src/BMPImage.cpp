
#include <stdio.h>

#include "StdFile.h"
#include "StdMemFile.h"

#include "BMPImage.h"

#include "jpegfix.h"

/* end of includes */

#define JPEG_IDENT		0xffd8ffe0
#define JPEG_PROG_IDENT 0xffd8ffe1

#define SWAP(a,b) {(a) ^= (b); (b) ^= (a); (a) ^= (b);}

/*----------------------------------------------------------------------------------------------------*/

APixelHandler::APixelHandler(AImage *image) : pImage(NULL),
											  pPixData(NULL)
{
	if (image) Attach(image);
}

APixelHandler::~APixelHandler()
{
}

void APixelHandler::Attach(AImage *image)
{
	pImage      = image;

	Rect		= pImage->GetRect();
	pPixData    = pImage->GetPixelData() + (Rect.h - 1) * Rect.w;
	BlankColour = pImage->GetBlankColour();
}

void APixelHandler::Detach()
{
	pImage   = NULL;
	pPixData = NULL;
	Rect.SetRect(0, 0, 0, 0);
}

/*----------------------------------------------------------------------------------------------------*/

APixelHandler::PIXEL APixelReader::ReadPixelScaled(double x, double y, double w, double h, uint_t mode) const
{
	PIXEL pix = {0, 0, 0, 0};

	if      (mode == SCALE_MODE_FAST) pix = ReadPixel((int)x, (int)y);
	else if (mode == SCALE_MODE_BEST) {
		double r = 0.0, g = 0.0, b = 0.0, mul = 1.0;
		int    ix = (int)x, iy = (int)y, iw = (int)w, ih = (int)h, i, j;

		if (iw && ih) {
			for (j = 0; j < ih; j++) {
				for (i = 0; i < iw; i++) {
					pix = ReadPixelLimited(ix + i, iy + j);
					r += pix.r; g += pix.g; b += pix.b;
				}
			}

			mul = 1.0 / (double)(iw * ih);
		}
		else if (iw && !ih) {
			double r1 = 0.0, g1 = 0.0, b1 = 0.0;
			double r2 = 0.0, g2 = 0.0, b2 = 0.0;

			for (i = 0; i < iw; i++) {
				pix = ReadPixelLimited(ix + i, iy);
				r1 += pix.r; g1 += pix.g; b1 += pix.b;
				pix = ReadPixelLimited(ix + i, iy + 1);
				r2 += pix.r; g2 += pix.g; b2 += pix.b;
			}

			double b = y - floor(y), a = 1.0 - b;
			r = a * r1 + b * r2; g = a * g1 + b * g2; b = a * b1 + b * b2;

			mul = 1.0 / (double)iw;
		}
		else if (!iw && ih) {
			double r1 = 0.0, g1 = 0.0, b1 = 0.0;
			double r2 = 0.0, g2 = 0.0, b2 = 0.0;

			for (j = 0; j < ih; j++) {
				pix = ReadPixelLimited(ix,     iy + j);
				r1 += pix.r; g1 += pix.g; b1 += pix.b;
				pix = ReadPixelLimited(ix + 1, iy + j);
				r2 += pix.r; g2 += pix.g; b2 += pix.b;
			}

			double b = x - floor(x), a = 1.0 - b;
			r = a * r1 + b * r2; g = a * g1 + b * g2; b = a * b1 + b * b2;

			mul = 1.0 / (double)ih;
		}
		else {
			double r1, g1, b1;
			double r2, g2, b2;
			double r3, g3, b3;
			double r4, g4, b4;

			pix = ReadPixelLimited(ix,     iy);
			r1 = pix.r; g1 = pix.g; b1 = pix.b;
			pix = ReadPixelLimited(ix + 1, iy);
			r2 = pix.r; g2 = pix.g; b2 = pix.b;
			pix = ReadPixelLimited(ix,     iy + 1);
			r3 = pix.r; g3 = pix.g; b3 = pix.b;
			pix = ReadPixelLimited(ix + 1, iy + 1);
			r4 = pix.r; g4 = pix.g; b4 = pix.b;

			double xb = x - floor(x), xa = 1.0 - xb;
			double yb = y - floor(y), ya = 1.0 - yb;
			r = xa * ya * r1 + xb * ya * r2 + xa * yb * r3 + xb * yb * r4;
			g = xa * ya * g1 + xb * ya * g2 + xa * yb * g3 + xb * yb * g4;
			b = xa * ya * b1 + xb * ya * b2 + xa * yb * b3 + xb * yb * b4;
		}

		r *= mul; g *= mul; b *= mul;

		pix.r = (uint8_t)(r + .5);
		pix.g = (uint8_t)(g + .5);
		pix.b = (uint8_t)(b + .5);
	}
	else assert(false);

	return pix;
}

/*----------------------------------------------------------------------------------------------------*/

void AMatrixPixelReader::SetOrientation(uint_t mode)
{
	if (IsAttached()) {
		switch (mode) {
			case ORIENTATION_TOP_LEFT:
				Matrix[0][0] =  1; Matrix[0][1] =  0;
				Matrix[1][0] =  0; Matrix[1][1] =  1;
				Origin.x = 0; Origin.y = 0;
				break;

			case ORIENTATION_TOP_RIGHT:
				Matrix[0][0] = -1; Matrix[0][1] =  0;
				Matrix[1][0] =  0; Matrix[1][1] =  1;
				Origin.x = Rect.w - 1; Origin.y = 0;
				break;

			case ORIENTATION_BOTTOM_LEFT:
				Matrix[0][0] =  1; Matrix[0][1] =  0;
				Matrix[1][0] =  0; Matrix[1][1] = -1;
				Origin.x = 0; Origin.y = Rect.h - 1;
				break;

			case ORIENTATION_BOTTOM_RIGHT:
				Matrix[0][0] = -1; Matrix[0][1] =  0;
				Matrix[1][0] =  0; Matrix[1][1] = -1;
				Origin.x = Rect.w - 1; Origin.y = Rect.h - 1;
				break;

			case ORIENTATION_TOP_LEFT_ROTATED:
				Matrix[0][0] =  0; Matrix[0][1] = -1;
				Matrix[1][0] =  1; Matrix[1][1] =  0;
				Origin.x = Rect.w - 1; Origin.y = 0;
				break;

			case ORIENTATION_TOP_RIGHT_ROTATED:
				Matrix[0][0] =  0; Matrix[0][1] =  1;
				Matrix[1][0] =  1; Matrix[1][1] =  0;
				Origin.x = 0; Origin.y = 0;
				break;

			case ORIENTATION_BOTTOM_LEFT_ROTATED:
				Matrix[0][0] =  0; Matrix[0][1] = -1;
				Matrix[1][0] = -1; Matrix[1][1] =  0;
				Origin.x = Rect.w - 1; Origin.y = Rect.h - 1;
				break;

			case ORIENTATION_BOTTOM_RIGHT_ROTATED:
				Matrix[0][0] =  0; Matrix[0][1] =  1;
				Matrix[1][0] = -1; Matrix[1][1] =  0;
				Origin.x = 0; Origin.y = Rect.h - 1;
				break;

			default:
				break;
		}
	}
}

ARect AMatrixPixelReader::CalcMatrixRect() const
{
	int x1 = Rect.x,		  y1 = Rect.y;
	int x2 = Rect.x + Rect.w, y2 = Rect.y;
	int x3 = Rect.x,		  y3 = Rect.y + Rect.h;
	int x4 = Rect.x + Rect.w, y4 = Rect.y + Rect.h;

	Apply(x1, y1);
	Apply(x2, y2);
	Apply(x3, y3);
	Apply(x4, y4);

	ARect rect;

	rect.x 	= MIN(x1, x2);
	rect.x 	= MIN(rect.x, x3);
	rect.x 	= MIN(rect.x, x4);
	rect.y 	= MIN(y1, y2);
	rect.y 	= MIN(rect.y, y3);
	rect.y 	= MIN(rect.y, y4);

	rect.w 	= MAX(x1, x2);
	rect.w 	= MAX(rect.w, x3);
	rect.w 	= MAX(rect.w, x4);
	rect.h 	= MAX(y1, y2);
	rect.h 	= MAX(rect.h, y3);
	rect.h 	= MAX(rect.h, y4);

	rect.w -= rect.x;
	rect.h -= rect.y;

	return rect;
}

/*----------------------------------------------------------------------------------------------------*/

AImage::AImage() : pData(NULL),
				   pWriter(&DefaultWriter),
				   pReader(&DefaultReader),
				   TransformOptimization(1),
				   TransformInterpolation(false)
{
	memset(&Header, 0, sizeof(Header));
	memset(&Info,   0, sizeof(Info));

	Colour      = AColour("$000000");
	BlankColour = AColour("$000000");
}

AImage::AImage(const AImage& img) : pData(NULL),
									pWriter(&DefaultWriter),
									pReader(&DefaultReader),
									TransformOptimization(1),
									TransformInterpolation(false)
{
	memset(&Header, 0, sizeof(Header));
	memset(&Info,   0, sizeof(Info));

	Colour      = AColour("$000000");
	BlankColour = AColour("$000000");

	operator = (img);
}

AImage::~AImage()
{
	Delete();
}

AImage& AImage::operator = (const AImage& img)
{
	if ((img.Rect.w * img.Rect.h) != (Rect.w * Rect.h)) {
		Delete();
		Rect = img.Rect;
		CreateData();
	}
	else {
		pReader->Detach();
		pWriter->Detach();
		Rect = img.Rect;
		pWriter->Attach(this);
		pReader->Attach(this);
	}

	if (pData) {
		memcpy(pData, img.pData, Rect.w * Rect.h * sizeof(*pData));

		Header 		= img.Header;
		Info   		= img.Info;
		Colour      = img.Colour;
		BlankColour = img.BlankColour;

		TransformOptimization  = img.TransformOptimization;
		TransformInterpolation = img.TransformInterpolation;
	}
	else Delete();

	return *this;
}

bool AImage::operator == (const AImage& img) const
{
	return ((Rect == img.Rect) && (memcmp(pData, img.pData, Rect.w * Rect.h * sizeof(*pData)) == 0));
}

bool AImage::CreateData()
{
	bool success = false;

	if (pData || !(Rect.w * Rect.h)) return false;

	if ((pData = new PIXEL[Rect.w * Rect.h]) != NULL) {
		memset(pData, 0, Rect.w * Rect.h * sizeof(*pData));

		// Create .bmp headers for display and saving
		memset(&Header, 0, sizeof(Header));
		memset(&Info, 0, sizeof(Info));

		/* generate header information */
		Header.bfType    = 'B' | ('M' << 8);
		Header.bfSize    = sizeof(Header) + sizeof(Info) + Rect.w * Rect.h * sizeof(*pData);
		Header.bfOffBits = sizeof(Header) + sizeof(Info);

		/* fill in members according to MS documentation */
		Info.bmiHeader.bV4Size        	= sizeof(Info.bmiHeader);
		Info.bmiHeader.bV4BitCount    	= sizeof(*pData) * 8;
		Info.bmiHeader.bV4Width       	= Rect.w;
		Info.bmiHeader.bV4Height      	= Rect.h;
		Info.bmiHeader.bV4Planes      	= 1;
		Info.bmiHeader.bV4V4Compression = BI_RGB;

		pWriter->Attach(this);
		pReader->Attach(this);

		success = true;
	}

	return success;
}

void AImage::DeleteData()
{
	if (pData) {
		pReader->Detach();
		pWriter->Detach();

		delete[] pData;
		pData = NULL;
	}
}

bool AImage::Create(int Width, int Height)
{
	bool success = false;

	if ((pData == NULL) && (Width > 0) && (Height > 0)) {
		Rect.SetRect(0, 0, Width, Height);

		success = CreateData();
	}

	return success;
}

void AImage::Delete()
{
	DeleteData();

	Rect.SetRect(0, 0, 0, 0);
}

void AImage::Clear(const AColour& col)
{
	if (pData) {
		PIXEL pix = col;
		uint32_t v   = (uint32_t&)pix;

		if (!v) memset(pData, 0, Rect.w * Rect.h * sizeof(*pData));
		else {
			PIXEL *p  = pData;
			uint_t  n   = Rect.w * Rect.h;

			while (n & 7) {
				p[0] = pix;
				p++; n--;
			}
			while (n) {
				p[0] = pix;
				p[1] = pix;
				p[2] = pix;
				p[3] = pix;
				p[4] = pix;
				p[5] = pix;
				p[6] = pix;
				p[7] = pix;
				p += 8; n -= 8;
			}
		}
	}
}

bool AImage::Load(const char *filename)
{
	AStdFile file;
	uint32_t ident = 0;
	bool success = false;

	Delete();

	if (file.open(filename, "rb")) {
		file.readitem(ident, AStdFile::SWAP_ALWAYS);
		file.close();
	}

	if ((ident == JPEG_IDENT) || (ident == JPEG_PROG_IDENT)) success = LoadJPEG(filename);
	else if ((ident & 0xffff0000) == 0x424d0000ul)			 success = LoadBMP(filename);

	return success;
}

bool AImage::Save(const char *filename, const TAG *tags)
{
	AStdFile file;
	bool success = false;

	if (file.open(filename, "wb")) {
		success = Save(file, tags);
		file.close();
	}

	return success;
}

bool AImage::Save(AStdData& fp, const TAG *tags)
{
	bool success = false;

	switch (TagValue(tags, TAG_FILETYPE, FILETYPE_BMP)) {
		case FILETYPE_BMP:
			success  = (fp.writebytes(&Header, sizeof(Header)) == sizeof(Header));
			success &= (fp.writebytes(&Info,   sizeof(Info))   == sizeof(Info));
			success &= (fp.writebytes(pData, Rect.w * Rect.h * sizeof(*pData)) == (slong_t)(Rect.w * Rect.h * sizeof(*pData)));
			break;

		case FILETYPE_JPEG:
			success = SaveJPEG(fp, tags);
			break;

		default:
			break;
	}

	return success;
}

bool AImage::LoadJPEG(const char *filename)
{
	AStdFile file;
	bool success = false;

	if (file.open(filename, "rb")) {
		success = LoadJPEG(file);
		file.close();
	}

	return success;
}

bool AImage::LoadJPEG(AStdData& fp)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	AStdMemFile mem;
	JDIMENSION l, x, y, num_scanlines;
	bool success = false;

	/* Initialize the JPEG decompression object with default error handling. */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

#if 0
	/* Add some application-specific error messages (from cderror.h) */
	jerr.addon_message_table = cdjpeg_message_table;
	jerr.first_addon_message = JMSG_FIRSTADDONCODE;
	jerr.last_addon_message = JMSG_LASTADDONCODE;

	/* Insert custom marker processor for COM and APP12.
	 * APP12 is used by some digital camera makers for textual info,
	 * so we provide the ability to display it as text.
	 * If you like, additional APPn marker types can be selected for display,
	 * but don't try to override APP0 or APP14 this way (see libjpeg.doc).
	 */
	jpeg_set_marker_processor(&cinfo, JPEG_COM, print_text_marker);
	jpeg_set_marker_processor(&cinfo, JPEG_APP0+12, print_text_marker);
#endif

	AStdFile    *filefp;
	AStdMemFile *memfp;
	if ((filefp = AStdFile::Cast(&fp)) != NULL) {
		/* Specify data source for decompression */
		jpeg_stdio_src(&cinfo, filefp->GetFile());
	}
	else if ((memfp = AStdMemFile::Cast(&fp)) != NULL) {
		/* Specify data source for decompression */
		jpeg_mem_src(&cinfo, memfp->GetData(), memfp->GetLength());
	}
	else {
		size_t inc = 32 * 1024;

		mem.open("w");

		while (true) {
			if (mem.AllocData(inc)) {
				size_t n;

				if ((n = fp.readbytes(mem.GetCurrentData(), inc)) == 0) break;

				mem.AddBytes(n);
				if (inc < (1024 * 1024)) inc <<= 1;
			}
		}

		/* Specify data source for decompression */
		jpeg_mem_src(&cinfo, mem.GetData(), mem.GetLength());
	}

	/* Read file header, set default decompression parameters */
	(void) jpeg_read_header(&cinfo, true);

	jpeg_start_decompress(&cinfo);

	if (Create(cinfo.output_width, cinfo.output_height)) {
		JSAMPLE *buffer;

		if ((buffer = new JSAMPLE[cinfo.output_width * cinfo.output_components]) != NULL) {
			success = true;
			y = 0;

			BeginPlot();

			/* Process data */
			while (cinfo.output_scanline < cinfo.output_height) {
				num_scanlines = jpeg_read_scanlines(&cinfo, &buffer, 1);

				if (cinfo.output_components == 1) {
					for (l = 0; l < num_scanlines; l++, y++) {
						for (x = 0; x < cinfo.output_width; x++) {
							Plot(x, y, AColour(buffer[x], buffer[x], buffer[x]));
						}
					}
				}
				else if (cinfo.output_components == 3) {
					for (l = 0; l < num_scanlines; l++, y++) {
						for (x = 0; x < cinfo.output_width; x++) {
							Plot(x, y, AColour(buffer[x * 3], buffer[x * 3 + 1], buffer[x * 3 + 2]));
						}
					}
				}
				else {
					success = false;
					break;
				}
			}

			EndPlot();

			delete[] buffer;
		}

		/* Finish decompression and release memory.
		 * Must do it in this order because output module has allocated memory
		 * of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
		 */
		(void) jpeg_finish_decompress(&cinfo);
	}

	jpeg_destroy_decompress(&cinfo);

	return success;
}

bool AImage::SaveJPEG(const char *filename, const TAG *tags)
{
	AStdFile file;
	bool success = false;

	if (file.open(filename, "wb")) {
		success = SaveJPEG(file, tags);
		file.close();
	}

	return success;
}

bool AImage::SaveJPEG(AStdData& fp, const TAG *tags)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	bool success = false;

	/* Initialize the JPEG compression object with default error handling. */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

#if 0
	/* Add some application-specific error messages (from cderror.h) */
	jerr.addon_message_table = cdjpeg_message_table;
	jerr.first_addon_message = JMSG_FIRSTADDONCODE;
	jerr.last_addon_message = JMSG_LASTADDONCODE;
#endif

	/* Initialize JPEG parameters.
	 * Much of this may be overridden later.
	 * In particular, we don't yet know the input file's color space,
	 * but we need to provide some value for jpeg_set_defaults() to work.
	 */

	cinfo.in_color_space = JCS_RGB; /* arbitrary guess */
	jpeg_set_defaults(&cinfo);

	/* Now that we know input colorspace, fix colorspace-dependent defaults */
	jpeg_default_colorspace(&cinfo);

	/* Specify data destination for compression */
	jpeg_stdio_dest(&cinfo, ((AStdFile&)fp).GetFile());

	cinfo.image_width  	   = Rect.w;		/* image width and height, in pixels */
	cinfo.image_height 	   = Rect.h;
	cinfo.input_components = 3;				/* # of color components per pixel */
	cinfo.in_color_space   = JCS_RGB;		/* colorspace of input image */

	jpeg_set_defaults(&cinfo);

	jpeg_set_quality(&cinfo, TagValue(tags, TAG_JPEG_QUALITY, 75), true /* limit to baseline-JPEG values */);

	/* Start compressor */
	jpeg_start_compress(&cinfo, true);

	JSAMPLE *buffer;
	if ((buffer = new JSAMPLE[cinfo.image_width * cinfo.input_components]) != NULL) {
		AColour col;
		JDIMENSION x, y = 0;
		uint_t p;

		success = true;

		/* Process data */
		while (cinfo.next_scanline < cinfo.image_height) {
			for (x = p = 0; x < cinfo.image_width; x++) {
				col = ReadPixel((int)x, (int)y);
				buffer[p++] = (JSAMPLE)col.r;
				buffer[p++] = (JSAMPLE)col.g;
				buffer[p++] = (JSAMPLE)col.b;
			}
			(void) jpeg_write_scanlines(&cinfo, &buffer, 1);
			y++;
		}

		delete[] buffer;
	}

	/* Finish compression and release memory */
	jpeg_finish_compress(&cinfo);

	jpeg_destroy_compress(&cinfo);

	return success;
}

bool AImage::LoadBMP(const char *filename)
{
	AStdFile fp;
	bool success = false;

	if (fp.open(filename, "rb")) {
		success = LoadBMP(fp);
		fp.close();
	}

	return success;
}

bool AImage::LoadBMP(AStdData& fp)
{
	bool success = false;

	Delete();

	if ((fp.readbytes(&Header, sizeof(Header)) == sizeof(Header)) &&
		(fp.readbytes(&Info,   sizeof(Info))   == sizeof(Info))) {
		if (Create(Info.bmiHeader.bV4Width, Info.bmiHeader.bV4Height)) {
			success = (fp.readbytes(pData, Rect.w * Rect.h * sizeof(*pData)) == (slong_t)(Rect.w * Rect.h * sizeof(*pData)));
		}
	}

	return success;
}

bool AImage::GenLineInfo(int x1, int y1, int x2, int y2, LINE_INFO& info) const
{
	bool ok = false;

	memset(&info, 0, sizeof(info));

	if (pWriter->LocationValid(x2, y2)) {
		SWAP(x1, x2);
		SWAP(y1, y2);
	}

	int xd = x2 - x1;
	int yd = y2 - y1;

	if (!xd && !yd) return false;

	if ((x1 >= Rect.w) || (y1 >= Rect.h)) {
		int x0 = MAX(x1 - (Rect.w - 1), 0);
		int y0 = MAX(y1 - (Rect.h - 1), 0);

		if      ((x0 > y0) && (xd != 0)) {x1 -= x0; y1 -= muldivsr(x0, yd, xd);}
		else if (yd != 0)				 {y1 -= y0; x1 -= muldivsr(y0, xd, yd);}
	}
	else if ((x1 < 0) || (y1 < 0)) {
		int x0 = MAX(-x1, 0);
		int y0 = MAX(-y1, 0);

		if      ((x0 > y0) && (xd != 0)) {x1 += x0; y1 += muldivsr(x0, yd, xd);}
		else if (yd != 0)				 {y1 += y0; x1 += muldivsr(y0, xd, yd);}
	}

	if (pWriter->LocationValid(x1, y1)) {
		info.x = x1;
		info.y = y1;

		if (x2 > x1) {
			info.greater  = x2 - x1;
			info.gt_count = MIN(info.greater, Rect.w - 1 - x1);
			info.xadd_gt  = 1;
		} else {
			info.greater  = x1 - x2;
			info.gt_count = MIN(info.greater, x1);
			info.xadd_gt  = -1;
		}

		if (y2 > y1) {
			info.smaller  = y2 - y1;
			info.sm_count = MIN(info.smaller, Rect.h - 1 - y1);
			info.yadd_sm  = 1;
		} else {
			info.smaller  = y1 - y2;
			info.sm_count = MIN(info.smaller, y1);
			info.yadd_sm  = -1;
		}

		if (info.smaller > info.greater) {
			SWAP(info.smaller,  info.greater);
			SWAP(info.sm_count, info.gt_count);
			SWAP(info.xadd_gt, info.xadd_sm);
			SWAP(info.yadd_gt, info.yadd_sm);
		}

		info.step = info.greater / 2 - 1;
		if (info.step < 0) info.step = 0;

		ok = true;
	}

	return ok;
}

void AImage::Line(int x1, int y1, int x2, int y2)
{
	LINE_INFO linfo;

	if (GenLineInfo(x1, y1, x2, y2, linfo)) {
		BeginPlot();
		do {
			assert((linfo.x >= 0) && (linfo.x < Rect.w) && (linfo.y >= 0) && (linfo.y < Rect.h));
			pWriter->PlotEx(linfo.x, linfo.y, Colour);
		} while (NextLinePoint(linfo));
		EndPlot();
	}
}

void AImage::Box(int x1, int y1, int x2, int y2)
{
	if (x1 > x2) SWAP(x1, x2);
	if (y1 > y2) SWAP(y1, y2);

	x1 = MAX(x1, 0);
	y1 = MAX(y1, 0);
	x2 = MIN(x2, Rect.w);
	y2 = MIN(x2, Rect.h);

	if ((x2 > x1) && (y2 > y1)) {
		int x, y;

		BeginPlot();
		for (y = y1; y < y2; y++) {
			for (x = x1; x < x2; x++) pWriter->PlotEx(x, y, Colour);
		}
		EndPlot();
	}
}

bool AImage::CopyPixelData(PIXEL *dst, const APoint& dstpt, const ASize& dstsz, const PIXEL *src, const APoint& srcpt, const ASize& srcsz, const ASize& copysz)
{
	bool success = false;

	if (src && dst) {
		int w = copysz.w, h = copysz.h;
		int x1, y1, x2, y2;

		x1 = dstpt.x; y1 = dstpt.y; x2 = x1 + w; y2 = y1 + h;
		x1 = MAX(x1, 0); x2 = MIN(x2, dstsz.w);
		y1 = MAX(y1, 0); y2 = MIN(y2, dstsz.h);

		dst += x1 + (dstsz.h - 1 - y1) * dstsz.w;
		w = MIN(w, x2 - x1);
		h = MIN(h, y2 - y1);

		x1 = srcpt.x; y1 = srcpt.y; x2 = x1 + w; y2 = y1 + h;
		x1 = MAX(x1, 0); x2 = MIN(x2, srcsz.w);
		y1 = MAX(y1, 0); y2 = MIN(y2, srcsz.h);

		src += x1 + (srcsz.h - 1 - y1) * srcsz.w;
		w = MIN(w, x2 - x1);
		h = MIN(h, y2 - y1);

		if ((w > 0) && (h > 0)) {
			while (h > 0) {
				memcpy(dst, src, w * sizeof(*dst));
				dst += dstsz.w; src += srcsz.w; h--;
			}
			success = true;
		}
	}

	return success;
}

void AImage::Blit(int x1, int y1, int x2, int y2, APixelReader& reader, int x3, int y3, bool tile)
{
	if (x1 > x2) SWAP(x1, x2);
	if (y1 > y2) SWAP(y1, y2);

	BeginPlot();

	x3 -= x1;
	y3 -= y1;

	int x, y;
	if (tile) {
		for (y = y1; y < y2; y++) {
			for (x = x1; x < x2; x++) {
				pWriter->Plot(x3 + x, y3 + y, reader.ReadPixelTiled(x, y));
			}
		}
	}
	else {
		for (y = y1; y < y2; y++) {
			for (x = x1; x < x2; x++) {
				pWriter->Plot(x3 + x, y3 + y, reader.ReadPixel(x, y));
			}
		}
	}

	EndPlot();
}

void AImage::MatrixBlit(const ARect& rect, APixelReader& reader, const APoint& offset, const double mat[2][2])
{
	if ((rect.w != 0) && (rect.h != 0)) {
		const double xo = offset.x;
		const double yo = offset.y;
		const int    xc = rect.x + rect.w / 2;
		const int    yc = rect.y + rect.h / 2;
		const int 	 x1 = -rect.w / 2;
		const int 	 y1 = -rect.h / 2;
		const int 	 x2 = x1 + rect.w;
		const int 	 y2 = y1 + rect.h;
		double xf, yf, xi, yi;
		int    x, y;

		BeginPlot();
		for (y = y1; y < y2; y++) {
			for (x = x1; x < x2; x++) {
				xf = (double)x; yf = (double)y;
				xi = xf * mat[0][0] + yf * mat[1][0];
				yi = xf * mat[0][1] + yf * mat[1][1];

				Plot(x + xc, y + yc, reader.ReadPixel(xi + xo, yi + yo));
			}
		}
		EndPlot();
	}
}

bool AImage::TransformConfig(const ARect& rect, APixelReader& reader, const ARect& rect1, const TRANSFORM& transform, TRANSFORM_DATA& data)
{
	data.reader = NULL;
	data.x1  = data.y1  = data.x2  = data.y2  = data.xc = data.yc = 0;
	data.xs1 = data.ys1 = data.xs2 = data.ys2 = 0;

	data.dist = data.perspective = data.cp = data.dcp = data.scale = 0.0;
	data.offsetx = data.offsety = 0.0;
	memset(data.border, 0, sizeof(data.border));
	memset(data.corner, 0, sizeof(data.corner));

	A3DMatrix forward(transform.xy, transform.yz, transform.xz);
	A3DPoint  normal = A3DPoint(0.0, 0.0, 1.0) * forward;

	if (normal.z == 0.0) return false;

	data.reader      = &reader;
	data.normalx	 = normal.x;
	data.normaly	 = normal.y;
	data.normalz	 = normal.z;
	data.dist        = transform.perspective + transform.dist;
	data.perspective = transform.perspective;
	data.inverse.CalcInverse(transform.xy, transform.yz, transform.xz);

	data.xc = rect.x + rect.w / 2;
	data.yc = rect.y + rect.h / 2;

	A3DPoint pt[4], ptp[4];
	pt[0] = A3DPoint(transform.Offset.x - rect1.w / 2, transform.Offset.y - rect1.h / 2, transform.Offset.z) * forward;
	pt[1] = A3DPoint(transform.Offset.x + rect1.w / 2, transform.Offset.y - rect1.h / 2, transform.Offset.z) * forward;
	pt[2] = A3DPoint(transform.Offset.x + rect1.w / 2, transform.Offset.y + rect1.h / 2, transform.Offset.z) * forward;
	pt[3] = A3DPoint(transform.Offset.x - rect1.w / 2, transform.Offset.y + rect1.h / 2, transform.Offset.z) * forward;

	uint_t i;
	for (i = 0; i < NUMBEROF(pt); i++) {
		pt[i].z += transform.dist;
		ptp[i] = pt[i];
		ptp[i].ApplyPerspective(transform.perspective);
		ptp[i].x *= transform.scale;
		ptp[i].y *= transform.scale;
		ptp[i].x += data.xc;
		ptp[i].y += data.yc;
	}

	for (i = 0; i < NUMBEROF(data.border); i++) {
		uint_t j = (i + 1) % NUMBEROF(ptp);
		double xd = ptp[j].x - ptp[i].x;
		double yd = ptp[j].y - ptp[i].y;

		if (yd != 0.0) {
			data.border[i].valid = true;
			data.border[i].m     = (xd / yd);
			data.border[i].c     = (ptp[i].x - data.border[i].m * ptp[i].y + .5);

			double y1 = MIN(ptp[i].y, ptp[j].y), y2 = MAX(ptp[i].y, ptp[j].y);
			data.border[i].y1 = (int)MIN(floor(y1), ceil(y1));
			data.border[i].y2 = (int)MAX(floor(y2), ceil(y2));
		}

		data.corner[i].x = (int)(ptp[i].x + .5);
		data.corner[i].y = (int)(ptp[i].y + .5);
	}

	double x1f, x2f, y1f, y2f;

	x1f = MIN(ptp[0].x, ptp[1].x);
	x2f = MAX(ptp[0].x, ptp[1].x);
	y1f = MIN(ptp[0].y, ptp[1].y);
	y2f = MAX(ptp[0].y, ptp[1].y);

	for (i = 2; i < NUMBEROF(ptp); i++) {
		x1f = MIN(x1f, ptp[i].x);
		x2f = MAX(x2f, ptp[i].x);
		y1f = MIN(y1f, ptp[i].y);
		y2f = MAX(y2f, ptp[i].y);
	}

	x1f = MAX(x1f, (double)rect.x);
	x2f = MIN(x2f, (double)(rect.x + rect.w - 1));

	y1f = MAX(y1f, (double)rect.y);
	y2f = MIN(y2f, (double)(rect.y + rect.h - 1));

	data.x1 = (int)MIN(floor(x1f), ceil(x1f));
	data.y1 = (int)MIN(floor(y1f), ceil(y1f));
	data.x2 = (int)MAX(floor(x2f), ceil(x2f));
	data.y2 = (int)MAX(floor(y2f), ceil(y2f));

	ARect rect2 = reader.GetRect();

	data.xs1 = rect1.x;             data.ys1 = rect1.y;
	data.xs2 = (rect1.x + rect1.w); data.ys2 = (rect1.y + rect1.h);
	data.xs1 = MAX(data.xs1, 0.0);
	data.xs1 = MIN(data.xs1, (rect2.w - 1));
	data.xs2 = MAX(data.xs2, 0.0);
	data.xs2 = MIN(data.xs2, (rect2.w - 1));
	data.ys1 = MAX(data.ys1, 0.0);
	data.ys1 = MIN(data.ys1, (rect2.h - 1));
	data.ys2 = MAX(data.ys2, 0.0);
	data.ys2 = MIN(data.ys2, (rect2.h - 1));

	double d   	 = normal.x * pt[0].x + normal.y * pt[0].y + normal.z * pt[0].z;
	data.cp    	 = normal.z * transform.perspective;
	data.dcp   	 = d + data.cp;
	data.scale 	 = 1.0 / transform.scale;
	data.offsetx = rect1.x + rect1.w / 2 - transform.Offset.x;
	data.offsety = rect1.y + rect1.h / 2 - transform.Offset.y;

	return true;
}

void AImage::TransformXLimits(const TRANSFORM_DATA& data, int y, int& x1, int& x2)
{
	double xf, x1f, x2f;
	uint_t i, n;

	x1f = data.x2;
	x2f = data.x1;
	for (i = n = 0; i < NUMBEROF(data.border); i++) {
		if (data.border[i].valid && (y >= data.border[i].y1) && (y <= data.border[i].y2)) {
			xf  = (double)y * data.border[i].m + data.border[i].c;
			x1f = MIN(x1f, xf);
			x2f = MAX(x2f, xf);
		}
	}
	x1 = (int)MAX(x1f, (double)data.x1);
	x2 = (int)MIN(x2f, (double)data.x2);

	//debug("XLimits(%d): %d , %d\n", y, x1, x2);
}

void AImage::TransformBlit(const ARect& rect, APixelReader& reader, const ARect& rect1, const TRANSFORM& transform, bool drawborder, const AColour& borderColour)
{
	TRANSFORM_DATA data;

	if (TransformConfig(rect, reader, rect1, transform, data)) {
		TransformBlit(data, drawborder, borderColour);
	}
}

void AImage::TransformBlit(TRANSFORM_DATA& data, bool drawborder, const AColour& borderColour)
{
	if (data.reader) {
		APixelReader *reader = data.reader;
		double xi, yi;
		int x, y;
		int x1, x2;

		for (y = data.y1; y <= data.y2; y++) {
			TransformXLimits(data, y, x1, x2);

			if (TransformOptimization >= 4) {
				while (x2 >= x1) {
					double x1f, y1f, x2f, y2f;
					int xn = MIN(x2 - x1 + 1, TransformOptimization);

					if (Transform(data, x1, y, x1f, y1f) && Transform(data, x1 + xn, y, x2f, y2f)) {
						double t1 = 1.0, t2 = 0.0, inc = ((xn > 1) ? 1.0 / (double)(xn - 1) : 0.0);

						if (TransformInterpolation) {
							for (x = 0; x < xn; x++) {
								xi = t1 * x1f + t2 * x2f;
								yi = t1 * y1f + t2 * y2f;

								if ((xi >= data.xs1) && (yi >= data.ys1) && (xi <= data.xs2) && (yi <= data.ys2)) {
									assert((x >= 0) && (x < Rect.w) && (y >= 0) && (y < Rect.h));
									pWriter->PlotEx(x1 + x, y, reader->ReadPixelLimited(xi, yi));
								}

								t1 -= inc; t2 += inc;
							}
						}
						else {
							for (x = 0; x < xn; x++) {
								xi = t1 * x1f + t2 * x2f + .5;
								yi = t1 * y1f + t2 * y2f + .5;

								if ((xi >= data.xs1) && (yi >= data.ys1) && (xi <= data.xs2) && (yi <= data.ys2)) {
									assert((x >= 0) && (x < Rect.w) && (y >= 0) && (y < Rect.h));
									pWriter->PlotEx(x1 + x, y, reader->ReadPixelEx((int)xi, (int)yi));
								}

								t1 -= inc; t2 += inc;
							}
						}
					}

					x1 += xn;
				}
			}
			else if (TransformInterpolation) {
				for (x = x1; x <= x2; x++) {
					if (Transform(data, x, y, xi, yi)) {
						if ((xi >= data.xs1) && (yi >= data.ys1) && (xi <= data.xs2) && (yi <= data.ys2)) {
							assert((x >= 0) && (x < Rect.w) && (y >= 0) && (y < Rect.h));
							pWriter->PlotEx(x, y, reader->ReadPixelLimited(xi, yi));
						}
					}
				}
			}
			else {
				for (x = x1; x <= x2; x++) {
					if (Transform(data, x, y, xi, yi)) {
						xi += .5; yi += .5;
						if ((xi >= data.xs1) && (yi >= data.ys1) && (xi <= data.xs2) && (yi <= data.ys2)) {
							assert((x >= 0) && (x < Rect.w) && (y >= 0) && (y < Rect.h));
							pWriter->PlotEx(x, y, reader->ReadPixelEx((int)xi, (int)yi));
						}
					}
				}
			}
		}

		if (drawborder) {
			uint_t i;

			AColour oldColour = SetColour(borderColour);

			for (i = 0; i < NUMBEROF(data.corner); i++) {
				uint_t j = (i + 1) % NUMBEROF(data.corner);

				Line(data.corner[i].x, data.corner[i].y, data.corner[j].x, data.corner[j].y);
			}

			SetColour(oldColour);
		}
	}
}

void AImage::ScaleBlit(const ARect& rect1, APixelReader& reader, const ARect& rect2, uint_t mode)
{
	if (rect1.w && rect1.h && rect2.w && rect2.h) {
		double xscale = (double)rect2.w / (double)rect1.w;
		double yscale = (double)rect2.h / (double)rect1.h;
		double x1, y1;
		int    x, y, w = abs(rect1.w), h = abs(rect1.h);

		for (y = 0; y < h; y++) {
			y1 = rect2.y + yscale * (double)y;

			for (x = 0; x < w; x++) {
				x1 = rect2.x + xscale * (double)x;

				if (pWriter->LocationValid(rect1.x + x, rect1.y + y)) {
					pWriter->PlotEx(rect1.x + x, rect1.y + y, reader.ReadPixelScaled(x1, y1, xscale, yscale, mode));
				}
			}
		}
	}
}
