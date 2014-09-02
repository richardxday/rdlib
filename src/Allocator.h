
#ifndef __ALLOCATOR__
#define __ALLOCATOR__

#include "misc.h"
#include "DataList.h"

class APermanentAllocation {
public:
	APermanentAllocation(uint32_t bytes) : p(NULL),
										 size(bytes) {
		p = new char[size];
	}
	~APermanentAllocation() {
		if (p) delete[] p;
	}

	uint32_t Size() const {return size;}

	operator void *() {return (void *)p;}
	operator char *() {return (char *)p;}
	operator int  *() {return (int  *)p;}
	operator long *() {return (long *)p;}
	operator unsigned char *() {return (unsigned char *)p;}
	operator unsigned int  *() {return (unsigned int  *)p;}
	operator unsigned long *() {return (unsigned long *)p;}
	operator float  *() {return (float  *)p;}
	operator double *() {return (double *)p;}

protected:
	char   *p;
	uint32_t size;
};

#endif
