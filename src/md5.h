/* MD5.H - header file for MD5C.C
 *
 * $Id: md5.h,v 1.3 2002/10/24 14:44:50 bporter Exp $
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

#ifndef _h_MD5
#define _h_MD5 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UINT4_T
#define UINT4_T unsigned long
#endif

#ifndef POINTER
#define POINTER unsigned char *
#endif

/* MD5 context. */
typedef struct {
  UINT4_T state[4];                                   /* state (ABCD) */
  UINT4_T count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CONTEXT;

extern void MD5Init (MD5_CONTEXT *);
extern void MD5Update (MD5_CONTEXT *, const unsigned char *, unsigned int);
extern void MD5Final (unsigned char [16], MD5_CONTEXT *);

#ifdef __cplusplus
};
#endif

#endif /* _h_MD5 */
