
#include <stdio.h>

int main(void)
{
#ifdef __APPLE__
	printf("-arch i386");
#endif

#ifdef __i386__
	printf("-msse3");
#endif

#ifdef __x86_64__
	printf("-msse3");
#endif

#ifdef __arm__
#ifdef __SOFTFP__
	printf("-mfloat-abi=softfp");
#endif
#endif

	printf("\n");
	
	return 0;
}