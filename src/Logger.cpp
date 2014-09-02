
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "StdFile.h"

#include "Logger.h"

ALogger::ALogger(const AString& ibasename, bool ms) : basename(ibasename),
													  msresolution(ms)
{
}


void ALogger::Log(const char *fmt, ...)
{
	ADateTime dt;		// timestamp current time
	AStdFile  fp;		// file handler
	va_list   ap;		// arbitary argument list

	va_start(ap, fmt);	// set up argument list

	// APPEND to log file for today (new file for each day)
	if (fp.open(basename + "-" + dt.DateFormat("%Y-%M-%D") + ".txt", "a")) {
		// write current date and time
		if (msresolution) {
			// write with ms resolution
			fp.printf("%s: ", dt.DateFormat("%Y-%M-%D %h:%m:%s.%S").str());
		}
		else {
			// write with seconds resolution
			fp.printf("%s: ", dt.DateFormat("%Y-%M-%D %h:%m:%s").str());
		}

		// write supplied message
		fp.vprintf(fmt, ap);

		// close file
		fp.close();
	}

	va_end(ap);
}
