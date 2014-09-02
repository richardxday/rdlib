#ifndef __MT19937AR__
#define __MT19937AR__

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned long genrand_int32(void);
extern long genrand_int31(void);
extern double genrand_real1(void);
extern double genrand_real2(void);
extern double genrand_real3(void);
extern double genrand_res53(void) ;

#ifdef __cplusplus
};
#endif

#endif
