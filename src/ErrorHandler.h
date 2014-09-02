
#ifndef __ERROR_HANDLER__
#define __ERROR_HANDKER__

#include "strsup.h"

class AErrorHandler {
public:
	AErrorHandler() {}
	~AErrorHandler() {DumpErrors();}

	void AddDetails(const char *filename, uint_t ln) {ErrString.printf("%s:%u: ", filename, ln);}
	void AddDetails(uint_t ln)					   {ErrString.printf("%u: ",    ln);}

	void SetError(const char *format, ...) PRINTF_FORMAT_METHOD {
		va_list ap;
		va_start(ap, format);
		ErrString.vprintf(format, ap);
		ErrString.Append("\n");
		va_end(ap);
	}
	void SetErrorV(const char *format, va_list ap) {
		ErrString.vprintf(format, ap);
		ErrString.Append("\n");
	}

	void DumpErrors() {
		if (ErrString.GetLength() != 0) {
			printf("%s", ErrString.GetBuffer());
			debug("%s", ErrString.GetBuffer());
		}
		ErrString.Delete();
	}

	const AString& GetErrors() const {return ErrString;}

	void ClearErrors() {ErrString.Delete();}

	operator bool() const {return (ErrString.GetLength() > 0);}

protected:
	AString ErrString;
};
	
#endif
