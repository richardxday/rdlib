
#include <stdio.h>
#include <string.h>

typedef enum {false = 0, true} bool;

int main(int argc, char *argv[])
{
    bool cflags    = ((argc < 2) || (strcmp(argv[1], "--cflags") == 0));
    bool libs      = ((argc > 1) && (strcmp(argv[1], "--libs") == 0));
    bool rootdir   = ((argc > 1) && (strcmp(argv[1], "--rootdir") == 0));
    bool prefix    = ((argc > 1) && (strcmp(argv[1], "--prefix") == 0));
    bool makefiles = ((argc > 1) && (strcmp(argv[1], "--makefiles") == 0));

    if (cflags || libs) {
#ifdef __APPLE__
        printf(" -arch i386");
#endif

        if (cflags) {
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
        }

#ifdef __linux__
        printf(" -fPIC");
#endif
    }

    if (cflags) {
#if defined(__linux__) || defined(__CYGWIN__)
        printf(" -D__LINUX__");
#endif

#ifdef __CYGWIN__
        printf(" -D_GNU_SOURCE=1");
#endif
    }

    if (cflags) {
#ifdef __MINGW64__
        printf(" -D__WORDSIZE=64");
#else
#ifdef __MINGW32__
        printf(" -D__WORDSIZE=32");
#endif
#endif
    }

    if (libs) {
#ifdef __MINGW32__
        printf(" -lwsock32");
#else
        printf(" -lrt");
#endif
    }

    if (rootdir) {
        printf("%s", RDLIB_ROOTDIR);
    }

    if (prefix) {
        printf("%s", RDLIB_PREFIX);
    }

    if (makefiles) {
        printf("%s", RDLIB_MAKEFILES);
    }

    printf("\n");

    return 0;
}
