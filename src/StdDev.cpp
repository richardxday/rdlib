
#include <stdio.h>

#include "StdDev.h"

NODETYPE_IMPLEMENT(AStdDev);

AStdDev::AStdDev() : AStdData(),
					 fd(-1)
{
}

AStdDev::~AStdDev()
{
	close();
}

bool AStdDev::open(const char *name, uint_t mode)
{
	bool success = false;

	if ((fd = ::open(name, mode)) >= 0) {
		success = true;
	}

	return success;
}

sint_t AStdDev::close()
{
	if (isopen()) {
		flush();
		::close(fd);
		fd = -1;
	}

	return 0;
}
	
bool AStdDev::isopen() const
{
	return (fd >= 0);
}

sint_t AStdDev::flush()
{
	sint_t res = -1;

	if (isopen()) {
#ifdef __LINUX__
		res = ::fsync(fd);
#else
		res = ::_commit(fd);
#endif
	}

	return res;
}

slong_t AStdDev::readdata(void *buf, size_t bytes)
{
	slong_t n = -1;

	if (isopen()) {
		n = ::read(fd, buf, bytes);
	}

	return n;
}

slong_t AStdDev::writedata(const void *buf, size_t bytes)
{
	slong_t n = -1;

	if (isopen()) {
		n = ::write(fd, buf, bytes);
	}

	return n;
}
