
#include <stdio.h>

#include "misc.h"
#include "StdFile.h"

/* end of includes */

#ifdef NEED_FEOF
#define feof(_stream)     ((_stream)->_flag & _IOEOF)
#endif
#ifdef NEED_FERROR
#define ferror(_stream)   ((_stream)->_flag & _IOERR)
#endif

AStdFile _Stdin(stdin);
AStdFile _Stdout(stdout);
AStdFile _Stderr(stderr);
AStdData *Stdin  = &_Stdin;
AStdData *Stdout = &_Stdout;
AStdData *Stderr = &_Stderr;

AStdFile _DbgOut(stdout);
AStdFile _DbgErr(stderr);
AStdData *DbgOut = &_DbgOut;
AStdData *DbgErr = &_DbgErr;

NODETYPE_IMPLEMENT(AStdData);

bool AStdData::SwapNeeded[SWAP_ITEMS] = {false, true, ::ByteSwapNeeded(false), ::ByteSwapNeeded(true)};

AStdData::AStdData()
{
}

AStdData::~AStdData()
{
	close();
}

slong_t AStdData::read(void *buf, size_t size, size_t count, uint_t swap)
{
	slong_t bytes  = readdata(buf, size * count);
	slong_t nitems = (bytes >= 0) ? bytes / size : bytes;
	
	assert(swap < NUMBEROF(SwapNeeded));

	if ((nitems > 0) && (size < MAX_SWAP_BYTES) && SwapNeeded[swap]) {
		::SwapBytes(buf, size, nitems);
	}

	return nitems;
}

slong_t AStdData::write(const void *buf, size_t size, size_t count, uint_t swap)
{
	slong_t nitems = -1;
	slong_t bytes  = size * count;

	assert(swap < NUMBEROF(SwapNeeded));

	if ((size < MAX_SWAP_BYTES) && SwapNeeded[swap]) {
		uint8_t *buf1;

		if ((buf1 = new uint8_t[bytes]) != NULL) {
			memcpy(buf1, buf, bytes);
			
			::SwapBytes(buf1, size, count);
			
			nitems = writedata(buf1, bytes);
			delete[] buf1;
		}
	}
	else nitems = writedata(buf, bytes);

	if (nitems > 0) nitems /= size;

	return nitems;
}

sint_t AStdData::printf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	sint_t chars = vprintf(format, ap);
	va_end(ap);

	return chars;
}

sint_t AStdData::vprintf(const char *format, va_list ap)
{
	char *buffer;
	sint_t res = -1;

	if ((res = vasprintf(&buffer, format, ap)) > 0) {
		res = writedata(buffer, res);
		free(buffer);
	}

	return res;
}

char *AStdData::gets(char *str, sint_t n)
{
	char *p = str;
	sint_t c;

	n--;
	while (((p - str) < n) && ((c = getc()) != EOF) && (c != '\n')) {
		p[0] = c; p++;
	}
	p[0] = 0;

	return (p > str) ? str : NULL;
}

sint_t AStdData::seek(slong_t offset, uint_t origin)
{
	sint_t ret = -1;
	if ((origin == SEEK_CUR) && (offset >= 0)) {
		static uint8_t buffer[4096];
		ulong_t n = offset;

		ret = 0;

		while (n) {
			ulong_t n1 = MIN(n, sizeof(buffer));
			slong_t n2 = readdata(buffer, n1);
			if (n2 < 0) {
				ret = -1;
				break;
			}
			else n -= n2;
		}
	}

	return ret;
}
