
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strsup.h"
#include "StdFile.h"
#include "blowfish.h"
#include "md5.h"

#include "authorize.h"

static const char *key =
 "9f0df128e17e129e4f544ef8256fea8f"
 "bd05936c2375afe6793918c9e7086e9f"
 "cbfc38d0e85bc9cb1dc9dfa17285df1d"
 "70f46dcde43ee5f06251d060146fd8a7";

static AString homedir = ".";

void setAuthFileLocation(const char *dir)
{
	homedir = dir;
}

static bool openAuthorizationFile(AStdFile& fp, const char *appname, const char *mode = "rb")
{
	return fp.open(homedir.CatPath(AString(appname) + AString("-authz.txt")), mode);
}

static void hash(const char *uuid, uint8_t result[IDENTIFIERBYTES])
{
	uint8_t res[16];
	uint_t i;

	{
		MD5_CONTEXT context;

		memset(&context, 0, sizeof(context));

		MD5Init(&context);
		MD5Update(&context, (const unsigned char *)uuid, strlen(uuid));
		MD5Final(res, &context);

		memset(&context, 0, sizeof(context));
	}

	for (i = 0; i < IDENTIFIERBYTES; i++) {
		result[i] = res[i] ^ res[i + IDENTIFIERBYTES];
	}
}

static void genString(const uint8_t result[IDENTIFIERBYTES], char *str)
{
	uint_t i, p = 0;

	for (i = 0; i < IDENTIFIERBYTES; i++) {
		if ((i > 0) && ((i % sizeof(uint32_t)) == 0)) p += sprintf(str + p, "-");

		p += sprintf(str + p, "%02X", (uint_t)result[i]);
	}
}

static void encrypt(const uint8_t identifier[IDENTIFIERBYTES], uint8_t result[IDENTIFIERBYTES])
{
	ABlowfish bf((const uint8_t *)key, strlen(key));

	bf.Encrypt(identifier, result, IDENTIFIERBYTES);
}

static uint_t getIdentifier(const char *str, uint8_t identifier[IDENTIFIERBYTES])
{
	uint_t i, p = 0;

	for (i = 0; (i < IDENTIFIERBYTES) && str[p];) {
		char text[3];
			
		while (str[p] && !IsHexChar(str[p])) p++;

		memset(text, 0, sizeof(text));
		strncpy(text, str + p, 2);

		p += strlen(text);

		uint_t val = 0;
		if (sscanf(text, "%x", &val) == 1) {
			identifier[i++] = val;
		}
		else {
			debug("Illegal hex data '%s' found\n", text);
			break;
		}
	}

	return i;
}

void generateIdentifier(const char *uuid, char *identifier)
{
	uint8_t result[IDENTIFIERBYTES];
	
	hash(uuid, result);

	genString(result, identifier);
}

unsigned int isAuthorized(const char *appname, const char *uuid)
{
	unsigned int authorized = 0;
	uint8_t result[IDENTIFIERBYTES];
	AStdFile fp;

	hash(uuid, result);
	encrypt(result, result);
	
	if (openAuthorizationFile(fp, appname)) {
		uint8_t authdata[sizeof(result)];

		if (fp.readbytes(authdata, sizeof(authdata)) == sizeof(authdata)) {
			if (memcmp(authdata, result, sizeof(result)) == 0) {
				authorized = 1;
			}
			else debug("Incorrect authorization key!\n");
		}
		else debug("Incorrect authorization key size!\n");

		fp.close();
	}
	
	return authorized;
}

unsigned int authorize(const char *appname, const char *uuid, const char *authtext)
{
	AStdFile fp;

	if (openAuthorizationFile(fp, appname, "wb")) {
		uint8_t authdata[IDENTIFIERBYTES];
		uint_t n = getIdentifier(authtext, authdata);
		
		fp.writebytes(authdata, n);

		fp.close();
	}

	return isAuthorized(appname, uuid);
}

#ifdef ALLOW_AUTH_GENERATION
void generateAuthorization(const char *identifier, char *authtext)
{
	uint8_t result[IDENTIFIERBYTES];
	
	memset(result, 0, sizeof(result));

	getIdentifier(identifier, result);

	encrypt(result, result);

	genString(result, authtext);
}
#endif
