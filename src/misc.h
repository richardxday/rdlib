
#ifndef __MISC__
#define __MISC__

#include <stdarg.h>
#include <math.h>
#include <assert.h>

#ifdef __APPLE__
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#ifdef _WIN32
#include "windows.h"
#endif

#if !defined(__CYGWIN__) && !defined(_WIN32)
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define stristr strcasestr
#endif

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __GNUC__
extern sint64_t llabs(sint64_t val);
#endif

#ifdef _WIN32
extern int vasprintf(char **buf, const char *fmt, va_list ap);
extern void SetInstance(HINSTANCE h);
extern HINSTANCE GetInstance();
extern uint32_t Exec(const char *cmd, uint32_t timeout);
#endif

#ifdef __LINUX__
extern uint32_t GetTickCount();
extern void   Sleep(uint32_t ms);
#endif

extern uint64_t GetNanosecondTicks();
	
extern void debug(const char *fmt, ...) PRINTF_FORMAT_FUNCTION;
extern void enabledebug(bool enabled = true);

#ifdef __cplusplus
};
#endif

#ifdef __cplusplus
template<typename T>
const T& limit(const T& a, const T& b, const T& c) {
	return (a >= b) ? ((a <= c) ? a : c) : b;
}
#endif

#ifdef __cplusplus
extern double dBToGain(double val);
extern double GainTodB(double val, double mindB = -200.0);

extern sint32_t muldivs(sint32_t a, sint32_t b, sint32_t c);
extern uint32_t muldivu(uint32_t a, uint32_t b, uint32_t c);
extern sint32_t muldivsr(sint32_t a, sint32_t b, sint32_t c);
extern uint32_t muldivur(uint32_t a, uint32_t b, uint32_t c);

extern sint32_t fmuldivsr(double a, sint32_t b, sint32_t c);

extern uint64_t muldivu(uint64_t val, uint32_t mul, uint32_t div);

extern sint32_t addms(sint32_t a, sint32_t b, sint32_t l = MAX_SIGNED(sint32_t));
extern uint32_t addmu(uint32_t a, uint32_t b, uint32_t l = MAX_UNSIGNED(uint32_t));
extern sint32_t subzs(sint32_t a, sint32_t b);
extern uint32_t subzu(uint32_t a, uint32_t b);

extern uint8_t muldivb(int a, int b);

extern double InterpRead(double *pBuffer, uint32_t Length, double pos);
extern float  InterpRead(float  *pBuffer, uint32_t Length, float  pos);
extern double InterpSum(double *pBuffer, uint32_t Length, double pos1, double pos2);
extern float  InterpSum(float  *pBuffer, uint32_t Length, float  pos1, float  pos2);
extern void   InterpReadStereo(double *pBufferL, double *pBufferR, uint32_t Length, double pos, double& left, double& right);
extern void   InterpReadStereo(float  *pBufferL, float  *pBufferR, uint32_t Length, float  pos, float&  left, float&  right);

extern uint16_t SwapBytes(uint16_t data);
extern uint32_t SwapBytes(uint32_t data);
extern uint64_t SwapBytes(uint64_t data);
extern float    SwapBytes(float data);
extern double   SwapBytes(double data);

enum {
	MAX_SWAP_BYTES = 16,
};
extern void SwapBytes(void *buf, uint_t nbytes);
extern void SwapBytes(void *buf, uint_t itemsize, uint_t nitems);

#define BYTESWAP_VAR(var)     SwapBytes(&var, sizeof(var))
#define BYTESWAP_ARRAY(array) SwapBytes(array, sizeof(array[0]), NUMBEROF(array))

#ifdef BYTE_ORDER
#if BYTE_ORDER == BIG_ENDIAN
#define MachineIsBigEndian() (true)
#elif BYTE_ORDER == LITTLE_ENDIAN
#define MachineIsBigEndian() (false)
#endif
#endif

#ifndef MachineIsBigEndian
#error Unknown endianness
extern bool MachineIsBigEndian();
#endif

extern bool ByteSwapNeeded(bool be);

extern void *Allocate(void *pData, size_t nItems, size_t& MaxItems, size_t ItemSize, size_t Inc = 16);

/*--------------------------------------------------------------------------------*/
/** Fix denormalize value to prevent execution penalties
 */
/*--------------------------------------------------------------------------------*/
extern double fix_denormalized(double val);

extern bool Interpolate(double& value, double target, double coeff = .1, double limit = 1.0e-7);
extern bool InterpolateInteger(double& value, double target, double coeff = .1, double limit = 1.0e-7);
extern bool InterpolateAngle(double& value, double target, double coeff = .1, double limit = 1.0e-7);
extern void LimitAngle(double& angle);

extern bool Interpolate2ndOrder(double& value, double& velocity, double target, double coeff = .1, double speed = 1.0, double limit = 1.0e-7);
extern bool Interpolate2ndOrderAngle(double& value, double& velocity, double target, double coeff = .1, double speed = 1.0, double limit = 1.0e-7);
#endif

#endif
