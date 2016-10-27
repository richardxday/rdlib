
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "StdSerial.h"

#ifdef __LINUX__
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#include "Recurse.h"

NODETYPE_IMPLEMENT(AStdSerial);

AStdSerial::AStdSerial() : AStdData(),
						   fd(-1)
{
}

AStdSerial::~AStdSerial()
{
	close();
}

bool AStdSerial::open(const char *name, uint_t baudrate, bool flow, uint_t datamode)
{
	bool success = false;

	if ((fd = ::open(name, O_RDWR | O_NOCTTY | O_NDELAY)) > 0) {
		struct termios options;

#ifdef FNDELAY
		/*
		 * Set non-blocking behaviour
		 */

		if (fcntl(fd, F_SETFL, FNDELAY) < 0) {
			debug("Failed to set non-blocking: %s\n", strerror(errno));
		}
#endif

		/*
		 * Get the current options for the port...
		 */

		if (tcgetattr(fd, &options) < 0) {
			debug("tcgetattr failed %s\n", strerror(errno));
		}

		/*
		 * Set the baud rates
		 */

#if B115200==115200
		// can just use the baud rate directly

		if (cfsetispeed(&options, baudrate) < 0) {
			debug("Failed to set serial input custom baud rate to %u (%s)\n", baudrate, strerror(errno));
		}
		if (cfsetospeed(&options, baudrate) < 0) {
			debug("Failed to set serial output custom baud rate to %u (%s)\n", baudrate, strerror(errno));
		}
#else
		static const struct {
			uint32_t  baudrate;
			speed_t speed;
		} speeds[] = {
			{50, B50},
			{75, B75},
			{110, B110},
			{134, B134},
			{150, B150},
			{200, B200},
			{300, B300},
			{600, B600},
			{1200, B1200},
			{1800, B1800},
			{2400, B2400},
			{4800, B4800},
			{9600, B9600},
			{19200, B19200},
			{38400, B38400},
			{57600, B57600},
			{115200, B115200},
			{230400, B230400},
#ifdef B460800
			{460800, B460800},
#endif
#ifdef B500000
			{500000, B500000},
#endif
#ifdef B576000
			{576000, B576000},
#endif
#ifdef B921600
			{921600, B921600},
#endif
#ifdef B1000000
			{1000000, B1000000},
#endif
#ifdef B1152000
			{1152000, B1152000},
#endif
#ifdef B1500000
			{1500000, B1500000},
#endif
#ifdef B2000000
			{2000000, B2000000},
#endif
#ifdef B2500000
			{2500000, B2500000},
#endif
#ifdef B3000000
			{3000000, B3000000},
#endif
#ifdef B3500000
			{3500000, B3500000},
#endif
#ifdef B4000000
			{4000000, B4000000},
#endif
		};
		uint_t i;
		for (i = 0; i < NUMBEROF(speeds); i++) {
			if (speeds[i].baudrate == baudrate) {
				if (cfsetispeed(&options, speeds[i].speed) < 0) {
					debug("Failed to set serial input standard baud rate to %u (%s)\n", baudrate, strerror(errno));
				}
				if (cfsetospeed(&options, speeds[i].speed) < 0) {
					debug("Failed to set serial output standard baud rate to %u (%s)\n", baudrate, strerror(errno));
				}
				break;
			}
		}

		if (i == NUMBEROF(speeds)) {
			debug("Baudrate %u not supported!\n", baudrate);
		}
#endif

		/*
		 * Enable the receiver and set local mode...
		 */

		options.c_cflag |= (CLOCAL | CREAD);

		switch (datamode) {
			default:
			case DataMode_8N1:
				options.c_cflag &= ~PARENB;
				options.c_cflag &= ~CSTOPB;
				options.c_cflag &= ~CSIZE;
				options.c_cflag |=  CS8;
				break;

			case DataMode_8E1:
				options.c_cflag |=  PARENB;
				options.c_cflag &= ~PARODD;
				options.c_cflag &= ~CSTOPB;
				options.c_cflag &= ~CSIZE;
				options.c_cflag |=  CS8;
				break;

			case DataMode_8O1:
				options.c_cflag |=  PARENB;
				options.c_cflag |=  PARODD;
				options.c_cflag &= ~CSTOPB;
				options.c_cflag &= ~CSIZE;
				options.c_cflag |=  CS8;
				break;

			case DataMode_8N2:
				options.c_cflag &= ~PARENB;
				options.c_cflag |=  CSTOPB;
				options.c_cflag &= ~CSIZE;
				options.c_cflag |=  CS8;
				break;

			case DataMode_8E2:
				options.c_cflag |=  PARENB;
				options.c_cflag &= ~PARODD;
				options.c_cflag |=  CSTOPB;
				options.c_cflag &= ~CSIZE;
				options.c_cflag |=  CS8;
				break;

			case DataMode_8O2:
				options.c_cflag |=  PARENB;
				options.c_cflag |=  PARODD;
				options.c_cflag |=  CSTOPB;
				options.c_cflag &= ~CSIZE;
				options.c_cflag |=  CS8;
				break;
		}

		/*
		 * Setup hardware flow control
		 */

		if (flow) {
			options.c_cflag |= CRTSCTS;    /* Also called CRTSCTS */
		}
		else {
			options.c_cflag &= ~CRTSCTS;
		}

		/*
		 * Setup raw mode
		 */

		options.c_iflag = 0;
		options.c_lflag = 0;
		options.c_oflag = 0;

		/*
		 * Clear all control characters
		 */

		memset(options.c_cc, 0, sizeof(options.c_cc));

		/*
		 * Set the new options for the port...
		 */

		if (tcsetattr(fd, TCSANOW, &options) < 0) {
			debug("tcsetattr failed %s\n", strerror(errno));
		}

		success = true;
	}

	return success;
}

sint_t AStdSerial::close()
{
	if (isopen()) {
		flush();
		::close(fd);
		fd = -1;
	}

	return 0;
}
	
bool AStdSerial::isopen() const
{
	return (fd >= 0);
}

slong_t AStdSerial::bytesavailable()
{
	slong_t n = -1;

	if (isopen()) {
		int bytes;

#ifdef __APPLE__
		if (::ioctl(fd, FIONREAD, &bytes) >= 0) {
			n = bytes;
		}
#else
		if (::ioctl(fd, TIOCINQ, &bytes) >= 0) {
			n = bytes;
		}
#endif
	}

	return n;
}

slong_t AStdSerial::bytesqueued()
{
	slong_t n = -1;

	if (isopen()) {
		int bytes;

 		n = 0;

		(void)bytes;

#ifdef FIONWRITE
		if (::ioctl(fd, FIONWRITE, &bytes) >= 0) {
			n = bytes;
		}
#endif


#ifdef TIOCOUTQ
		if (::ioctl(fd, TIOCOUTQ, &bytes) >= 0) {
			n = bytes;
		}
#endif
	}

	return n;
}

sint_t AStdSerial::flush()
{
	sint_t res = -1;

	if (isopen()) {
		res = ::tcflush(fd, TCIOFLUSH);
	}

	return res;
}

slong_t AStdSerial::readdata(void *buf, size_t bytes)
{
	slong_t avail;
	slong_t n = -1;

	if (isopen() && ((avail = bytesavailable()) >= 0)) {
		bytes = MIN(avail, (slong_t)bytes);

		if (bytes == 0) n = 0;
		else			n = ::read(fd, buf, bytes);
	}

	return n;
}

slong_t AStdSerial::writedata(const void *buf, size_t bytes)
{
	slong_t n = -1;

	if (isopen() && (bytesavailable() >= 0)) {
		n = ::write(fd, buf, bytes);
	}

	return n;
}

uint_t AStdSerial::findusbserialports(AList& list, uint_t maxports)
{
	UNUSED(maxports);

	list.DeleteAll();

#ifdef __APPLE__
	// Mac
	CollectFiles("/dev", "tty.usbserial*", 0, list);
#else
	CollectFiles("/dev", "ttyUSB*", 0, list);
#endif

#if 0
	debug("USB serial ports:\n");
	const AString *str = AString::Cast(list.First());
	uint_t index = 0;
	while (str) {
		debug("%u: %s\n", index++, str->str());
		str = str->Next();
	}
	debug("Total %u comport%s found\n", index, (index == 1) ? "" : "s");
#endif

	return list.Count();
}
#endif

#ifdef _WIN32
#include "strsup.h"

NODETYPE_IMPLEMENT(AStdSerial);

AStdSerial::AStdSerial() : AStdData(),
						   file(INVALID_HANDLE_VALUE)
{
}

AStdSerial::~AStdSerial()
{
	close();
}

bool AStdSerial::open(const char *name, uint_t baudrate, bool flow, uint_t datamode)
{
	AString filename;
	bool success = false;

	filename.printf("\\\\?\\%s", name);
	if ((file = CreateFile(filename,
						   GENERIC_READ | GENERIC_WRITE,
						   0,
						   NULL,
						   OPEN_EXISTING,
						   FILE_ATTRIBUTE_NORMAL,
						   NULL)) != INVALID_HANDLE_VALUE) {
        DCB dcb;
        
		dcb.DCBlength = sizeof(DCB);
		
		GetCommState(file, &dcb);
		
		dcb.BaudRate = baudrate;

		switch (datamode) {
			default:
			case DataMode_8N1:
				dcb.ByteSize = 8;
				dcb.Parity   = NOPARITY;
				dcb.StopBits = ONESTOPBIT;
				break;

			case DataMode_8E1:
				dcb.ByteSize = 8;
				dcb.Parity   = EVENPARITY;
				dcb.StopBits = ONESTOPBIT;
				break;

			case DataMode_8O1:
				dcb.ByteSize = 8;
				dcb.Parity   = ODDPARITY;
				dcb.StopBits = ONESTOPBIT;
				break;

			case DataMode_8N2:
				dcb.ByteSize = 8;
				dcb.Parity   = NOPARITY;
				dcb.StopBits = TWOSTOPBITS;
				break;

			case DataMode_8E2:
				dcb.ByteSize = 8;
				dcb.Parity   = EVENPARITY;
				dcb.StopBits = TWOSTOPBITS;
				break;

			case DataMode_8O2:
				dcb.ByteSize = 8;
				dcb.Parity   = ODDPARITY;
				dcb.StopBits = TWOSTOPBITS;
				break;
		}
		
		// setup hardware flow control
		
		dcb.fOutxDsrFlow = flow;
		dcb.fDtrControl  = DTR_CONTROL_HANDSHAKE;
		
		dcb.fOutxCtsFlow = flow;
		dcb.fRtsControl  = RTS_CONTROL_HANDSHAKE;
		
		// setup software flow control
		
		dcb.fInX = dcb.fOutX = false;
		
		// other various settings
		
		dcb.fBinary = true;
		dcb.fParity = true;
		
		SetCommState(file, &dcb);

		GetCommState(file, &dcb);
		
		SetCommMask(file, EV_RXCHAR);
		
		// setup device buffers
		
		SetupComm(file, 262144, 262144);
		
		// purge any information in the buffer
		
		PurgeComm(file, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
		
		EscapeCommFunction(file, SETDTR);
		EscapeCommFunction(file, SETRTS);

		success = true;
	}

	return success;
}

sint_t AStdSerial::close()
{
	if (isopen()) {
		CloseHandle(file);
		file = INVALID_HANDLE_VALUE;
	}

	return 0;
}
	
bool AStdSerial::isopen() const
{
	return (file != INVALID_HANDLE_VALUE);
}

slong_t AStdSerial::bytesavailable()
{
	slong_t n = -1;

	if (isopen()) {
		COMSTAT stat;
		DWORD   errors;

		memset(&stat, 0, sizeof(stat));

		if (ClearCommError(file, &errors, &stat)) {
			n = stat.cbInQue;
		}
	}

	return n;
}

slong_t AStdSerial::bytesqueued()
{
	slong_t n = -1;

	if (isopen()) {
		COMSTAT stat;
		DWORD   errors;

		memset(&stat, 0, sizeof(stat));

		if (ClearCommError(file, &errors, &stat)) {
			n = stat.cbOutQue;
		}
	}

	return n;
}

sint_t AStdSerial::flush()
{
	sint_t res = -1;

	if (isopen()) {
		if (PurgeComm(file, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR)) res = 0;
		else res = -1;
	}

	return res;
}

slong_t AStdSerial::readdata(void *buf, size_t bytes)
{
    slong_t avail;
	slong_t n = -1;

	if (isopen() && ((avail = bytesavailable()) >= 0)) {
		DWORD n1 = 0;

        bytes = MIN(bytes, (uint32_t)avail);

		if (bytes && ReadFile(file, (uint8_t *)buf, bytes, &n1, NULL)) n = n1;
		else n = 0;
	}

	return n;
}

slong_t AStdSerial::writedata(const void *buf, size_t bytes)
{
	slong_t n = -1;

	if (isopen() && (bytesavailable() >= 0)) {
		DWORD n1 = 0;

		if (bytes && WriteFile(file, (uint8_t *)buf, bytes, &n1, NULL)) n = n1;
		else n = 0;
	}

	return n;
}

uint_t AStdSerial::findusbserialports(AList& list, uint_t maxports)
{
	list.DeleteAll();

	uint_t i;
	for (i = 1; i < maxports; i++) {
		AString    name;
		COMMCONFIG cc;
		DWORD      dwSize = sizeof(COMMCONFIG);

		name.printf("COM%u", i);

		if (GetDefaultCommConfigA(name, &cc, &dwSize)) {
			list.Add(new AString(name));
		}
	}

	return list.Count();
}
#endif
