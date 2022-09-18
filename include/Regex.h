
#ifndef __REGEX__
#define __REGEX__

#include <regex>

#include "strsup.h"
#include "DataList.h"

typedef struct {
    AString marker;
    uint_t pos, len;
} REGEXREGION;

extern AString ParseRegex(const AString& str, char escchar = '\\');
extern AString ParseRegex(const AString& str, AString& errors, char escchar = '\\');
extern bool    MatchRegex(const AString& str, const AString& pat, bool casesens = false, char escchar = '\\');
extern bool    MatchRegex(const AString& str, const AString& pat, ADataList& regionlist, bool casesens = false, char escchar = '\\');

extern AString ExpandRegexRegions(const AString& str, const AString& expandstr, const ADataList& regionlist, char expandchar = '\\');
extern bool    IsRegexPattern(const AString& pat);
extern bool    IsExclusionPattern(const AString& pat);
extern bool    IsRegexAnyPattern(const AString& pat);


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
extern bool MatchRegex(const std::string& pattern, const std::string& string, bool matchcase = false);

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

#endif
