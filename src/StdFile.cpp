
#include <stdio.h>

#ifdef __LINUX__
// Linux/Cygwin
#include <sys/types.h>
#include <sys/stat.h>

#if defined(ANDROID) || defined(__IPHONEOS__)
#include <dirent.h>
#endif

#if defined(__CYGWIN__) || defined(__IPHONEOS__)
extern "C" {
#include <sys/dirent.h>
};
#elif !defined(ANDROID)
#include <sys/dir.h>
#endif

#else
#include <fcntl.h>
#include <io.h>
#endif

#include "StdFile.h"

/* end of includes */

NODETYPE_IMPLEMENT(AStdFile);

AStdFile::AStdFile(const char *filename, const char *mode) : AStdData(),
															 fp(NULL),
															 flags(0)
{
	if (filename) open(filename, mode);
}

AStdFile::AStdFile(FILE *p, const char *filename, bool binary) : AStdData(),
																 fp(NULL),
																 flags(0)
{
	open(p, filename, binary);
}

AStdFile::~AStdFile()
{
	close();
}

bool AStdFile::open(const char *filename, const char *mode)
{
	bool success = false;

	if (!fp) {
		if ((fp = fopen(filename, mode)) != NULL) {
			this->filename = filename;
			flags = Flag_Opened;
			if (strchr(mode, 'b')) flags |= Flag_Binary;
			success = true;
		}
	}

	return success;
}

bool AStdFile::open(FILE *p, const char *filename, bool binary)
{
	bool success = false;

	if (!fp && p) {
		if (filename) this->filename = filename;
		else		  this->filename.Delete();
		fp      = p;
		flags   = binary ? Flag_Binary : 0;
		success = true;
	}

	return success;
}

sint_t AStdFile::close()
{
	sint_t ret = 0;

	if (fp) {
		if (wasopened()) {
			ret = ::fclose(fp);
		}
		else ret = 0;
		fp = NULL;
	}

	flags = 0;
	filename.Delete();

	return ret;
}

AStdFile *AStdFile::dup() const
{
	AStdFile *newfile = NULL;

	if (isopen() && filename.Valid()) {
		newfile = new AStdFile(filename, binarymode() ? "rb" : "r");
	}

	return newfile;
}

slong_t AStdFile::bytesavailable()
{
	slong_t res = -1;

	if (fp && (eof() == 0)) {
		if ((fp == stdout) || (fp == stderr)) res = 0;
		else								  res = MAX_SIGNED(slong_t);
	}

	return res;
}

slong_t AStdFile::readdata(void *buf, size_t bytes)
{
	return fp ? ::fread(buf, 1, bytes, fp) : -1;
}

slong_t AStdFile::writedata(const void *buf, size_t bytes)
{
	slong_t res = fp ? ::fwrite(buf, 1, bytes, fp) : -1;
	// this prevents some problems on Linux when writing files
	(void)tell();
	return res;
}

sint_t AStdFile::vprintf(const char *format, va_list ap)
{
	sint_t res = fp ? ::vfprintf(fp, format, ap) : -1;
	// this prevents some problems on Linux when writing files
	(void)tell();
	return res;
}

sint_t AStdFile::putc(sint_t c)
{
	sint_t res = fp ? fputc(c, fp) : EOF;
	// this prevents some problems on Linux when writing files
	(void)tell();
	return res;
}

sint_t AStdFile::getc()
{
	return fp ? fgetc(fp) : EOF;
}

sint_t AStdFile::puts(const char *str)
{
	sint_t res = fp ? ::fputs(str, fp) : EOF;
	// this prevents some problems on Linux when writing files
	(void)tell();
	return res;
}

char *AStdFile::gets(char *str, int n)
{
	return fp ? ::fgets(str, n, fp) : NULL;
}

slong_t AStdFile::tell()
{
	return fp ? ::ftell(fp) : -1;
}

sint_t AStdFile::seek(slong_t offset, uint_t origin)
{
	sint_t res = fp ? ::fseek(fp, offset, origin) : -1;
	// this prevents some problems on Linux when writing files
	(void)tell();
	return res;
}

sint_t AStdFile::flush()
{
	return fp ? ::fflush(fp) : -1;
}

void AStdFile::rewind()
{
	if (fp) ::rewind(fp);
}

sint_t AStdFile::eof()
{
	return fp ? feof(fp) : -1;
}

sint_t AStdFile::error()
{
	return fp ? ferror(fp) : -1;
}

sint_t AStdFile::setvbuf(char *buffer, int mode, size_t size)
{
	return fp ? ::setvbuf(fp, buffer, mode, size) : -1;
}

sint_t AStdFile::setbinarymode(bool binary)
{
	int res = -1;

	if (isopen()) {
#ifdef __LINUX__
		// not needed for Linux and BSD cos all files are treated as binary
		UNUSED(binary);
		res = 0;
#else
		res = setmode(_fileno(fp), binary ? _O_BINARY : _O_TEXT);
#endif
	}

	return res;
}

bool AStdFile::exists(const char *filename)
{
	FILE *fp;
	bool success = false;

	if ((fp = fopen(filename, "rb")) != NULL) {
		fclose(fp);

		success = true;
	}

	return success;
}
