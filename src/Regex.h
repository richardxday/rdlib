
#ifndef __REGEX__
#define __REGEX__

#include "strsup.h"
#include "DataList.h"

typedef struct {
	AString marker;
	uint_t pos, len;
} REGEXREGION;

extern AString ParseRegex(const AString& str, char escchar = '\\');
extern AString ParseRegex(const AString& str, AString& errors, char escchar = '\\');
extern AString ParsePathRegex(const AString& str);
extern AString ParsePathRegex(const AString& str, AString& errors);
extern bool    MatchRegex(const AString& str, const AString& pat, bool casesens = false, char escchar = '\\');
extern bool    MatchRegex(const AString& str, const AString& pat, ADataList& regionlist, bool casesens = false, char escchar = '\\');
extern bool    MatchPathRegex(const AString& str, const AString& pat);
extern bool    MatchPathRegex(const AString& str, const AString& pat, ADataList& regionlist);

extern AString ExpandRegexRegions(const AString& str, const AString& expandstr, const ADataList& regionlist, char expandchar = '\\');
extern bool    IsRegexPattern(const AString& pat);
extern bool    IsExclusionPattern(const AString& pat);
extern bool    IsRegexAnyPattern(const AString& pat);

#endif
