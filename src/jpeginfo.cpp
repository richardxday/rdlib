
#include <stdio.h>

#include "jpeginfo.h"
#include "exif.h"

#include "jpegfix.h"

/* end of includes */

bool ReadJPEGInfo(const char *filename, JPEG_INFO& info)
{
	EXIFINFO exinfo;
	Exif exif(&exinfo);
	FILE *fp;
	bool success = false;

	memset(&exinfo, 0, sizeof(exinfo));

	if ((fp = fopen(filename, "rb")) != NULL) {
		if (exif.DecodeExif(fp)) {
			info.CameraMake  = exinfo.CameraMake;
			info.CameraModel = exinfo.CameraModel;
			info.Width  	 = exinfo.Width;
			info.Height 	 = exinfo.Height;
			info.Orientation = exinfo.Orientation;
			info.bDateValid  = false;
			info.Comments    = exinfo.Comments;

			AString str = exinfo.DateTime;

			if (str.Valid()) {
				//printf("File '%s' has date '%s'\n", filename, str.str());

				str.Replace(":/-.", "    ");
				
				ADateTime& dt = info.DateTime;
				uint_t word = 0;
				
				uint16_t year   = str.Word(word++);
				uint8_t  month  = str.Word(word++);
				uint8_t  day    = str.Word(word++);
				uint8_t  hour   = str.Word(word++);
				uint8_t  minute = str.Word(word++);
				uint8_t  second = str.Word(word++);

				if ((year >= 1980) && RANGE(month, 1, 12) && RANGE(day, 1, 31) && (hour <= 23) && (minute <= 59) && (second <= 61)) {
					dt.Set(day, month, year, hour, minute, second);
					
					info.bDateValid = true;
				}
			}

			success = true;
		}
		
		fclose(fp);
	}

	return success;
}

#if 0
bool LoadJPEG(ACanvas *pC, const char *filename, ACanvas *pC1)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JDIMENSION l, x, y, num_scanlines;
	AStdFile file;
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

	if (file.open(filename, "rb")) {
		/* Specify data source for decompression */
		jpeg_stdio_src(&cinfo, file.GetFile());
		
		/* Read file header, set default decompression parameters */
		(void) jpeg_read_header(&cinfo, true);

		jpeg_start_decompress(&cinfo);

		if (!pC->Valid() || (pC->GetWidth() != (sint_t)cinfo.output_width) || (pC->GetHeight() != (sint_t)cinfo.output_height)) {
			pC->Delete();
			pC->CreateCompatibleDC(pC1);
			pC->CreateCompatibleBitmap(pC1, cinfo.output_width, cinfo.output_height);
		}

		if (pC->Valid()) {
			JSAMPLE *buffer;

			if ((buffer = new JSAMPLE[cinfo.output_width * cinfo.output_components]) != NULL) {
				success = true;
				y = 0;

				/* Process data */
				while (cinfo.output_scanline < cinfo.output_height) {
					num_scanlines = jpeg_read_scanlines(&cinfo, &buffer, 1);
					
					if (cinfo.output_components == 1) {
						for (l = 0; l < num_scanlines; l++, y++) {
							for (x = 0; x < cinfo.output_width; x++) {
								pC->SetPixelV(x, y, AColour(buffer[x], buffer[x], buffer[x]));
							}
						}
					}
					else if (cinfo.output_components == 3) {
						for (l = 0; l < num_scanlines; l++, y++) {
							for (x = 0; x < cinfo.output_width; x++) {
								pC->SetPixelV(x, y, AColour(buffer[x * 3], buffer[x * 3 + 1], buffer[x * 3 + 2]));
							}
						}
					}
					else {
						success = false;
						break;
					}
				}

				delete[] buffer;
			}

			/* Finish decompression and release memory.
			 * Must do it in this order because output module has allocated memory
			 * of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
			 */
			(void) jpeg_finish_decompress(&cinfo);
		}

		file.close();
	}

	jpeg_destroy_decompress(&cinfo);

	return success;
}

bool SaveJPEG(ACanvas *pC, const char *filename, int quality)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	AStdFile file;
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

	if (file.open(filename, "wb")) {
		/* Specify data destination for compression */
		jpeg_stdio_dest(&cinfo, file.GetFile());

		cinfo.image_width  	   = pC->GetWidth();		/* image width and height, in pixels */
		cinfo.image_height 	   = pC->GetHeight();
		cinfo.input_components = 3;				/* # of color components per pixel */
		cinfo.in_color_space   = JCS_RGB;		/* colorspace of input image */
		
		jpeg_set_defaults(&cinfo);
		
		jpeg_set_quality(&cinfo, quality, true /* limit to baseline-JPEG values */);

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
					col = pC->GetPixel(x, y);
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

		file.close();
	}

	jpeg_destroy_compress(&cinfo);

	return success;
}
#endif
