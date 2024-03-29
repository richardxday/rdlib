
#ifndef __REGEX__
#define __REGEX__

#include <regex>

#include "strsup.h"
#include "DataList.h"

typedef struct {
    AString marker;
    uint_t pos, len;
} GLOBREGION;

extern AString ParseGlob(const AString& str, char escchar = '\\');
extern AString ParseGlob(const AString& str, AString& errors, char escchar = '\\');
extern bool    MatchGlob(const AString& str, const AString& pat, bool casesens = false, char escchar = '\\');
extern bool    MatchGlob(const AString& str, const AString& pat, ADataList& regionlist, bool casesens = false, char escchar = '\\');

extern AString ExpandGlobRegions(const AString& str, const AString& expandstr, const ADataList& regionlist, char expandchar = '\\');
extern bool    IsGlobPattern(const AString& pat);
extern bool    IsExclusionPattern(const AString& pat);
extern bool    IsGlobAnyPattern(const AString& pat);

/*--------------------------------------------------------------------------------*/
/** Parse regex pattern
 *
 * @param pattern regex pattern as text
 * @param matchcase true if match should be case sensitive
 *
 * @return parsed regex pattern
 *
 * @note this uses the std::regex_constants::ECMAScript grammar
 * @note this is required because std::regex() does NOT work in an wxWidgets based files
 *
 * @note calls to this (with non-empty pattern) *must* be wrapped in a try-catch since
 * @note an exception will be thrown if an invalid pattern is passed!
 */
/*--------------------------------------------------------------------------------*/
extern std::regex ParseRegex(const std::string& pattern, bool matchcase = false);
inline std::regex ParseRegex(const AString& pattern, bool matchcase = false) {
    return ParseRegex(pattern.operator std::string(), matchcase);
}
inline std::regex ParseRegex(const char *pattern, bool matchcase = false) {
    return ParseRegex(std::string(pattern), matchcase);
}

/*--------------------------------------------------------------------------------*/
/** Return whether string matches pattern
 *
 * @param string string to match
 * @param pattern unparsed regex pattern
 * @param matchcase true if match should be case sensitive
 *
 * @return true if entire string matches
 *
 * @note this uses the std::regex_constants::ECMAScript grammar
 * @note this is required because std::regex() does NOT work in an wxWidgets based files
 *
 * @note calls to this (with non-empty pattern) *must* be wrapped in a try-catch since
 * @note an exception will be thrown if an invalid pattern is passed!
 */
/*--------------------------------------------------------------------------------*/
extern bool MatchRegex(const std::string& string, const std::string& pattern, bool matchcase = false);
inline bool MatchRegex(const AString& string, const AString& pattern, bool matchcase = false) {
    return MatchRegex(string.operator std::string(), pattern.operator std::string(), matchcase);
}
inline bool MatchRegex(const char *string, const char *pattern, bool matchcase = false) {
    return MatchRegex(std::string(string), std::string(pattern), matchcase);
}

/*--------------------------------------------------------------------------------*/
/** Return whether string matches pattern
 *
 * @param string string to match
 * @param pattern parsed regex pattern
 *
 * @return true if entire string matches
 *
 * @note this uses the std::regex_constants::ECMAScript grammar
 * @note this is required because std::regex() does NOT work in an wxWidgets based files
 */
/*--------------------------------------------------------------------------------*/
extern bool MatchRegex(const std::string& string, const std::regex& pattern);
inline bool MatchRegex(const AString& string, const std::regex& pattern)
{
    return MatchRegex(string.operator std::string(), pattern);
}
inline bool MatchRegex(const char *string, const std::regex& pattern)
{
    return MatchRegex(std::string(string), pattern);
}

/*--------------------------------------------------------------------------------*/
/** Return whether string contains pattern
 *
 * @param string string to search for
 * @param pattern unparsed regex pattern
 * @param matchcase true if match should be case sensitive
 *
 * @return true if string contains regex
 *
 * @note this uses the std::regex_constants::ECMAScript grammar
 * @note this is required because std::regex() does NOT work in an wxWidgets based files
 *
 * @note calls to this (with non-empty pattern) *must* be wrapped in a try-catch since
 * @note an exception will be thrown if an invalid pattern is passed!
 */
/*--------------------------------------------------------------------------------*/
extern bool ContainsRegex(const std::string& pattern, const std::string& string, bool matchcase = false);
inline bool ContainsRegex(const AString& pattern, const AString& string, bool matchcase = false) {
    return ContainsRegex(pattern.operator std::string(), string.operator std::string(), matchcase);
}
inline bool ContainsRegex(const char *pattern, const char *string, bool matchcase = false) {
    return ContainsRegex(std::string(pattern), std::string(string), matchcase);
}

/*--------------------------------------------------------------------------------*/
/** Return whether string contains pattern
 *
 * @param string string to search for
 * @param pattern parsed regex pattern
 *
 * @return true if string contains regex
 *
 * @note this uses the std::regex_constants::ECMAScript grammar
 * @note this is required because std::regex() does NOT work in an wxWidgets based files
 */
/*--------------------------------------------------------------------------------*/
extern bool ContainsRegex(const std::string& string, const std::regex& pattern);
inline bool ContainsRegex(const AString& string, const std::regex& pattern) {
    return ContainsRegex(string.operator std::string(), pattern);
}
inline bool ContainsRegex(const char *string, const std::regex& pattern) {
    return ContainsRegex(std::string(string), pattern);
}

typedef struct
{
    std::regex  regex;
    std::string replace;
} regex_replace_t;

/*--------------------------------------------------------------------------------*/
/** Parse a regex/replacement pair from a single string
 *
 * @param str string
 * @param replace regex_replace_t structure to be populated
 * @param matchcase true if contains is case sensitive
 *
 * @return true if parsed correctly
 *
 * @note the string should be in the format <sep><regex><sep><replacement><sep> like
 * @note sed, e.g. '/<regex>/<replacement>/'
 * @note the string will be de-escaped using backslashes, this can be used to include
 * @note <sep> characters as part of the regex or replacement.  It also means existing
 * @note backslashes need to be double-escaped, e.g. '/path\/to\/\\(whereever|whenever\\)/new\/path/'
 * @note regex is basic regex so parentheses for groups do not need to be escaped
 * @note (literal parentheses need to be escaped)
 * @note e.g. '/^.+\/([a-zA-Z0-9]+)\/.+$/\\1/'
 * @note       ^------------------------------ separator marking start of search regex
 * @note        ^----------------------------- start at start of string
 * @note         ^^--------------------------- at least one of any character
 * @note           ^^------------------------- separator character as part of regex (will be converted to '/')
 * @note             ^------------------------ group start
 * @note              ^^^^^^^^^^^------------- class specifier
 * @note                         ^------------ at least one of class required
 * @note                          ^----------- group end
 * @note                           ^^--------- separator character as part of regex (will be converted to '/')
 * @note                             ^^------- at least one of any character
 * @note                               ^------ end at end of string
 * @note                                ^----- separator separating regex and replacement
 * @note                                 ^^^-- double escaped replace group (will be converted to '\1')
 * @note                                    ^- separator marking end of replacement
 *
 * @note calls to this (with non-empty pattern) *must* be wrapped in a try-catch since
 * @note an exception will be thrown if an invalid pattern is passed!
 */
/*--------------------------------------------------------------------------------*/
extern bool ParseRegexReplacement(const std::string& str, regex_replace_t& replace, bool matchcase = false);
inline bool ParseRegexReplacement(const AString& str, regex_replace_t& replace, bool matchcase = false) {
    return ParseRegexReplacement(str.operator std::string(), replace, matchcase);
}
inline bool ParseRegexReplacement(const char *str, regex_replace_t& replace, bool matchcase = false) {
    return ParseRegexReplacement(std::string(str), replace, matchcase);
}

/*--------------------------------------------------------------------------------*/
/** Perform a regex replacement on a string
 *
 * @param str string
 * @param replace regex_replace_t structure describing search regex and replacement
 * @param flags match flags
 *
 * @return modified string unless regex is empty in which case the original string is returned
 *
 * @note see note above regarding regex format
 *
 * @note calls to this (with non-empty pattern) *must* be wrapped in a try-catch since
 * @note an exception will be thrown if an invalid pattern is passed!
 */
/*--------------------------------------------------------------------------------*/
extern std::string RegexReplace(const std::string& str, const regex_replace_t& replace, std::regex_constants::match_flag_type flags = std::regex_constants::format_sed);
inline AString RegexReplace(const AString& str, const regex_replace_t& replace, std::regex_constants::match_flag_type flags = std::regex_constants::format_sed) {
    return AString(RegexReplace(str.operator std::string(), replace, flags));
}
inline AString RegexReplace(const char *str, const regex_replace_t& replace, std::regex_constants::match_flag_type flags = std::regex_constants::format_sed) {
    return AString(RegexReplace(std::string(str), replace, flags));
}

#endif
