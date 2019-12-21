
#ifndef __LINUXLIB_TYPES__
#define __LINUXLIB_TYPES__

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#ifndef __WORDSIZE
#error __WORDSIZE not defined
#endif

#if __WORDSIZE==64
#define SYSTEM_IS_64BITS    1
#if defined(__APPLE__) || defined(__MINGW64__)
#define LONG_IS_64BITS      0
#else
#define LONG_IS_64BITS      1
#endif
#else
#define SYSTEM_IS_64BITS    0
#define LONG_IS_64BITS      0
#endif

#define INT32_IS_INT        (!LONG_IS_64BITS)

typedef int8_t         sint8_t;
typedef int16_t        sint16_t;
typedef int32_t        sint32_t;
typedef int64_t        sint64_t;

typedef signed   short sshort_t;
typedef unsigned short ushort_t;
typedef signed   int   sint_t;
typedef unsigned int   uint_t;
typedef signed   long  slong_t;
typedef unsigned long  ulong_t;
typedef signed   long  long sllong_t;
typedef unsigned long  long ullong_t;

#if SYSTEM_IS_64BITS
typedef uint64_t       uptr_t;
#else
typedef uint32_t       uptr_t;
#endif

#define MIN_SIGNED(type)   (-(type)(((ullong_t)1 << ((sizeof(type) << 3) - 1)) - 1) - (type)1)
#define MAX_SIGNED(type)   ( (type)(((ullong_t)1 << ((sizeof(type) << 3) - 1)) - 1))

#define MIN_UNSIGNED(type) ((type)0)
#define MAX_UNSIGNED(type) ((type)(((ullong_t)1 << (sizeof(type) << 3)) - 1))

#ifndef LOGN
#define LOGN(n,p) ((int)(log((double)(n)) / log((double)(p)) + .5))
#endif

#ifdef __GNUC__
#define PRINTF_FORMAT_FUNCTION __attribute__ ((format(printf, 1, 2)))
#define PRINTF_FORMAT_METHOD   __attribute__ ((format(printf, 2, 3)))
#define SCANF_FORMAT_METHOD    __attribute__ ((format(scanf, 2, 3)))
#else
#define PRINTF_FORMAT_FUNCTION
#define PRINTF_FORMAT_METHOD
#define SCANF_FORMAT_METHOD
#endif

#define forptr(ptr,initptr) for (ptr = initptr; ptr; ptr = ptr->Next())

#define IsPositiveChar(c) ((c) == '+')
#define IsNegativeChar(c) ((c) == '-')
#define IsSignChar(c) (IsPositiveChar(c) || IsNegativeChar(c))
#define IsNotChar(c) ((c) == '~')
#define IsPointChar(c) ((c) == '.')
#define IsNumeralChar(c) (RANGE(c, '0', '9'))
#define IsHexLowerChar(c) (RANGE(c, 'a', 'f'))
#define IsHexUpperChar(c) (RANGE(c, 'A', 'F'))
#define IsHexChar(c) (IsNumeralChar(c) || IsHexLowerChar(c) || IsHexUpperChar(c))
#define IsBinChar(c) (RANGE(c, '0', '1'))
#define IsOctChar(c) (RANGE(c, '0', '7'))
#define IsAlphaChar(c) (RANGE(c, 'a', 'z') || RANGE(c, 'A', 'Z'))
#define IsEngSymbolChar(c) (((c) == 'e') || ((c) == 'E') || ((c) == 'p') || ((c) == 'P'))
#define IsSymbolStart(c) (IsAlphaChar(c) || ((c) == '_'))
#define IsSymbolChar(c) (IsAlphaChar(c) || ((c) == '_') || IsNumeralChar(c))
#define IsSymbolCharPoint(c) (IsAlphaChar(c) || ((c) == '_') || IsNumeralChar(c) || ((c) == '.'))
#define IsWhiteSpace(c) (((c) == ' ') || ((c) == '\t'))
#define IsQuoteChar(c) (((c) == '\'') || ((c) == '\"'))
#define IsHexStartChar(c) ((c) == '$')
#define IsOctStartChar(c) ((c) == '@')
#define IsDoubleStartChar(c) ((c) == '#')
#define IsHexString(p) (((p)[0] == '0') && (((p)[1] == 'x') || ((p)[1] == 'X')))
#define IsOctString(p) (((p)[0] == '0') && (((p)[1] == 'o') || ((p)[1] == 'O')))
#define IsBinString(p) (((p)[0] == '0') && (((p)[1] == 'b') || ((p)[1] == 'B')))

#define PACKEDSTRUCT struct __attribute__ ((packed))

#define COMPARE_ITEMS(a,b) (((a) == (b)) ? 0 : (((a) < (b)) ? -1 : 1))

#define UNUSED(x) ((void)x)

#endif

// Common macros that are sometimes helpfully undefined by other includes
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef LIMIT
#define LIMIT(a, b, c) (((a) < (b)) ? (b) : (((a) > (c)) ? (c) : (a)))
#endif
#ifndef CLIP
#define CLIP(a, b) (((b) >= 0) ? MIN((a), (b)) : MAX((a), -(b)))
#endif
#ifndef SUBZ
#define SUBZ(a, b) (((a) >= (b)) ? (a) - (b) : 0)
#endif

#ifndef NUMBEROF
#define NUMBEROF(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef ROUND_NEAREST
#define ROUND_NEAREST(v) (((v) < 0.0) ? v - .5 : v + .5)
#endif

#ifndef RANGE
#define RANGE(c, a, b) (((c) >= (a)) && ((c) <= (b)))
#endif

#ifndef WITHIN
#define WITHIN(c, a, b) (((c) >= (a)) && ((c) < (b)))
#endif
