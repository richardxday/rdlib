#ifndef __AUTHORIZE__
#define __AUTHORIZE__

#define ALLOW_AUTH_GENERATION

#define IDENTIFIERBYTES 8

#ifdef __cplusplus
extern "C" {
#endif

extern void setAuthFileLocation(const char *dir);

extern void generateIdentifier(const char *uuid, char *identifier);

extern unsigned int isAuthorized(const char *appname, const char *uuid);
extern unsigned int authorize(const char *appname, const char *uuid, const char *authtext);

#ifdef ALLOW_AUTH_GENERATION
extern void generateAuthorization(const char *identifier, char *authtext);
#endif

#ifdef __cplusplus
};
#endif

#endif
