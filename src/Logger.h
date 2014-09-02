#ifndef __LOGGER__
#define __LOGGER__

#include "DateTime.h"

class ALogger {
public:
	ALogger(const AString& ibasename = "log", bool ms = false);

	void Log(const char *fmt, ...) PRINTF_FORMAT_METHOD;

protected:
	AString basename;
	bool    msresolution;
};

#endif

