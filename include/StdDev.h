#ifndef __STANDARD_DEVICE__
#define __STANDARD_DEVICE__

#include "misc.h"
#include "StdData.h"

#include <unistd.h>

class AStdDev : public AStdData {
public:
	AStdDev();
	virtual ~AStdDev();

	virtual bool open(const char *name, uint_t mode = O_RDONLY);
	virtual sint_t close();
	
	virtual bool isopen() const;

	virtual sint_t flush();

	NODETYPE(AStdDev, AStdData);

protected:
	virtual slong_t readdata(void *buf, size_t bytes);
	virtual slong_t writedata(const void *buf, size_t bytes);

protected:
	int fd;

	NODETYPE_DEFINE(AStdDev);
};

#endif
