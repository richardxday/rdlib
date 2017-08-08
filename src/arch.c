
#include <stdio.h>

int main(void)
{
#ifdef __APPLE__
	printf(" -arch i386");
#endif

#ifdef __i386__
	printf(" -msse3");
#endif

#ifdef __x86_64__
	printf(" -msse3");
#endif

#ifdef __arm__
#ifdef __SOFTFP__
	printf(" -mfloat-abi=softfp");
#endif
#endif

#ifdef __CYGWIN__
	printf(" -D_GNU_SOURCE=1");
#else
	printf(" -fPIC");
#endif
	
	printf("\n");
	
	return 0;
}
