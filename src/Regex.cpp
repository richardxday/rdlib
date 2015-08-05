
#include <stdio.h>
#include <ctype.h>

#include "Regex.h"

/* end of includes */

#define IsWordChar(c) (IsAlphaChar(c) || IsNumeralChar(c) || IsNegativeChar(c) || (c == '\''))

#define PATH_REGEX_CASE_SENSITIVE true

//#define DEBUG_REGEX
//#define SHOW_REGEX_PATTERNS
//#define SHOW_REGEX_REGIONS

// Regex tokens to differentiate between normal characters
// and special regex characters.  Most important is the 
// TOKEN_REGEX token is signifies the string is a parsed
// regex string
enum {
	TOKEN_REGEX = (char)224,
	TOKEN_OR,
	TOKEN_ESCAPE,
};

// structure to keep track of number of parentheses, brackets and braces matched
typedef struct {
	int  paren;
	int  brackets;
	int  braces;
} BRACKETS;

typedef struct {
	ADataList *regions;
	AString   *errors;
	bool	  casesens;
	uint_t	  recurselevel;
} REGEXDATA;

/*------------------------------------------------------------
  Function: Convert escaped character sequence to a single
			character
  Enter   : str = string containing escape sequence
			pos = position of character AFTER escape character
				  (updated to position BEYOND escape sequence)
  Exit    : decoded character value
  Notes   : 
  ----------------------------------------------------------*/
static char deescape(const char *str, uint_t& pos)
{
	static uint8_t vals[256] = {0};
	uint_t val;
	sint_t i;

	if (!vals[(uint_t)'1']) {
		memset(vals, 0xff, sizeof(vals));
		for (i = '0'; i <= '9'; i++) vals[i] = i - '0';
		for (i = 'a'; i <= 'f'; i++) vals[i] = i - 'a' + 10;
		for (i = 'A'; i <= 'F'; i++) vals[i] = i - 'A' + 10;
	}

	char c = str[pos];
	switch (c) {
		case '0': c = 0; break;
		case 'n': c = '\n'; pos++; break;
		case 'r': c = '\r'; pos++; break;
		case 't': c = '\t'; pos++; break;
		default:
			// decimal escape sequence?
			if (IsNumeralChar(c)) {
				// 3 or less numeral characters make up escaped character
				c = 0;
				for (i = 0; (i < 3) && ((val = vals[(uint_t)str[pos]]) != 0xff) && (val < 10); i++, pos++) {
					c = (c * 10) + val;
				}
			}
			// hexidecimal escape sequence
			else if (tolower(c) == 'x') {
				pos++;

				// 2 or less hex characters make up escaped character
				c = 0;
				for (i = 0; (i < 2) && ((val = vals[(uint_t)str[pos]]) != 0xff); i++, pos++) {
					c = (c << 4) + val;
				}
			}
			// character is returned character (and the position is advanced)
			else pos++;
			break;
	}
	return c;
}

/*------------------------------------------------------------
  Function: Parse characters following a '$'
  Enter   : str    = pattern string
			i      = current position in pattern (on the
					 character after the '$' character)
			errors = optional ptr to errors string (changed if
					 an error is found)
  Exit    : 
  Notes   : 
  ----------------------------------------------------------*/
static void check_dollar_str(const char *str, uint_t& i, AString *errors = NULL)
{
	static const char *brackets  = ")]}";
	static const char *operators = "dwlupqsbea";
	uint_t i0 = i;

	// reset brackets count
	if (str[i] == 'r') {
		i++;

		// check bracket character
		if		(!errors) i++;
		else if (str[i] && strchr(brackets, str[i])) i++;
		else	errors->printf("Illegal character '%c' for bracket count reset at '%s'\n", str[i], str + i0);
	}
	else if (str[i] == 'c') {
		i++;

		// skip over optional minus indicator
		if (IsNegativeChar(str[i])) i++;

		// skip over index value
		while (IsNumeralChar(str[i])) i++;

		// check bracket character
		if		(!errors) i++;
		else if	(str[i] && strchr(brackets, str[i])) i++;
		else	errors->printf("Illegal character '%c' for bracket count check at '%s'\n", str[i], str + i0);
	}
	else if ((str[i] == '<') || (str[i] == '>') || (str[i] == '*')) i++;
	else {
		// skip over optional modifiers
		if (str[i] == '^') i++;
		if ((str[i] == 'u') || (str[i] == 'l')) i++;

		if		(!errors) i++;
		else if	(str[i] && strchr(operators, str[i])) i++;
		else	errors->printf("Illegal character '%c' for character check at '%s'\n", str[i], str + i0);
	}
}

/*------------------------------------------------------------
  Function: Find terminator char, skipping over escaped characters
  Enter   : str    	= string
			i      	= starting position
			term   	= terminating char
			escchar = escape char (usually '\')
  Exit    : position of terminating char or at error
  Notes   : 
  ----------------------------------------------------------*/
static uint_t find_simple(const char *str, uint_t i, char term, char escchar)
{
	while (str[i] && (str[i] != term)) {
		if (str[i] == escchar) {
			i++;
			deescape(str, i);
		}
		else i++;
	}

	return i;
}

/*------------------------------------------------------------
  Function: Find terminating char skipping over groups of (),
			{} and [] and also correctly skipping escape 
			sequences
  Enter   : str    	= string
			i      	= starting position
			term   	= terminating char
			errors 	= error string (UPDATED)
			escchar = escape char (usually '\')
  Exit    : position of terminating char or at error
  Notes   : 
  ----------------------------------------------------------*/
static uint_t find(const char *str, uint_t i, char term, AString& errors, char escchar)
{
	uint_t j;

	while (errors.Empty() && str[i] && (str[i] != term)) {
		switch (str[i]) {
			case '(':
				j = find(str, i + 1, ')', errors, escchar);
				if (str[j] == ')') i = j + 1;
				else if (errors.Empty()) errors.printf("Missing closing ')' for '%c' at '%s'\n", str[i], str + i);
				break;

			case '[':
				j = find_simple(str, i + 1, ']', escchar);
				if (str[j] == ']') i = j + 1;
				else if (errors.Empty()) errors.printf("Missing closing ']' for '%c' at '%s'\n", str[i], str + i);
				break;

			case '{':
				j = find(str, i + 1, '}', errors, escchar);
				if (str[j] == '}') i = j + 1;
				else if (errors.Empty()) errors.printf("Missing closing '}' for '%c' at '%s'\n", str[i], str + i);
				break;
				
			case '<':
				j = find(str, i + 1, '>', errors, escchar);
				if (str[j] == '>') i = j + 1;
				else if (errors.Empty()) errors.printf("Missing closing '>' for '%c' at '%s'\n", str[i], str + i);
				break;
				
			case ')':
			case ']':
			case '}':
			case '>':
				if (errors.Empty()) {
					if (term) errors.printf("Found '%c' when looking for a '%c' at '%s'\n", str[i], term, str + i);
					else	  errors.printf("Too many '%c's at '%s'\n", str[i], str + i);
				}
				break;

			case '$':
				i++;
				check_dollar_str(str, i, &errors);
				break;

			default:
				if (str[i++] == escchar) deescape(str, i);
				break;
		}

		if (errors.Valid()) break;
	}

	return i;
}

/*------------------------------------------------------------
  Function: Split pattern out into sections of 'or's, leaving
			any parenthsised or bracketed sections along
  Enter   : pat     = pattern
			errors  = error string (UPDATED)
			escchar = escape character or 0 for no tokenisation
					  of escape characters
  Exit    : modified pattern
  Notes   : 
  ----------------------------------------------------------*/
static AString split(const AString& pat, AString& errors, char escchar = 0)
{
	// cache of previous patterns and results
	static struct {
		AString pat1, pat2;
		uint_t    age;
	} cache[20];
	static uint_t age = 0;
	AString pat1;
	char *ppat1;
	uint_t i, maxage = 0, oldest = 0;

	// copy pattern ready for modification
	pat1  = pat;
	// grab modifiable copy of pattern
	ppat1 = pat1.Steal();

	// replace escape characters in pattern with escape token
	if (escchar != 0) {
		char *p = ppat1;

		// find escape character
		while ((p = strchr(p, escchar)) != NULL) {
			// replace with token
			*p++ = TOKEN_ESCAPE;
			
			// find length of escape sequence
			uint_t pos = 0;
			deescape(p, pos);

			// advance pointer by length
			p += pos;
		}
	}

	// look pattern up in cache and return result
	// if it's found
	age++;
	for (i = 0; i < NUMBEROF(cache); i++) {
		// test cache entry
		if (cache[i].pat1 == ppat1) {
			delete[] ppat1;
			cache[i].age = age;
			return cache[i].pat2;
		}

		// find oldest cache entry
		if (!i || ((age - cache[i].age) > maxage)) {
			maxage = age - cache[i].age;
			oldest = i;
		}
	}

	// if pattern contains either the standard 'or' character ('|') or the
	// DOS 'or' character then try to split the string
	if (strchr(ppat1, '|')) {
		for (i = 0; ppat1[i];) {
			switch (ppat1[i]) {
				case '(':
					// skip over parentheses
					i = find(ppat1, i + 1, ')', errors, TOKEN_ESCAPE);
					if (ppat1[i] == ')') i++;
					break;

				case '[':
					// skip over square brackets
					i = find_simple(ppat1, i + 1, ']', TOKEN_ESCAPE);
					if (ppat1[i] == ']') i++;
					break;

				case '{':
					// skip over braces
					i = find(ppat1, i + 1, '}', errors, TOKEN_ESCAPE);
					if (ppat1[i] == '}') i++;
					break;
				
				case '<':
					// skip over chevrons
					i = find(ppat1, i + 1, '>', errors, TOKEN_ESCAPE);
					if (ppat1[i] == '>') i++;
					break;
				
				case '|':
				case 90:
					// replace character with OR token
					ppat1[i++] = TOKEN_OR;
					break;

				case '$':
					i++;
					check_dollar_str(ppat1, i);
					break;

				case TOKEN_ESCAPE:
					// correctly handle escape sequences
					i++;
					deescape(ppat1, i);
					break;

				default:
					i++;
					break;
			}
		}
	}

	// put modified string back into object
	pat1.Take(ppat1);

	// replace oldest item in cache with new data
	cache[oldest].pat1 	  = pat;
	cache[oldest].pat2 	  = pat1;
	cache[oldest].age  	  = age;

#ifdef SHOW_REGEX_PATTERNS
	{
		static const char lf[] = {TOKEN_OR, 0};
		debug("Converted '%s' into '%s'\n", pat.SearchAndReplace(lf, "<OR>").str(), pat1.SearchAndReplace(lf, "<OR>").str());
	}
#endif

	return pat1;
}

/*------------------------------------------------------------
  Function: Extract region marker from pattern
  Enter   : pat		= pattern (ALTERED)
			errors  = error string (UPDATED)
  Exit    : region marker (if any)
  Notes   : 1. marker is assumed to be at the start of the
			   supplied pattern
			2. marker is removed from pattern
  ----------------------------------------------------------*/
static AString getmarker(AString& pat, AString& errors)
{
	AString marker;
	uint_t pos = 0;

	// ':' signifies marker text to follow
	if ((pat[pos] == ':') && strchr(pat.str() + pos + 1, ':')) {
		AStringUpdate updater(&marker);

		// find end of marker
		pos++;
		while (pat[pos] && (pat[pos] != ':')) {
			// decode character
			char c = pat[pos++];
			if (c == TOKEN_ESCAPE) c = deescape(pat, pos);

			// update marker string
			updater.Update(c);
		}

		// skip over closing ':', if present
		if (pat[pos] == ':') pat = pat.Mid(pos + 1);
	}

	// split remaining pattern
	pat = split(pat, errors);

	return marker;
}

/*------------------------------------------------------------
  Function: Convert class string into array of flags
  Enter   : pat    	 = pattern
			pos    	 = start position within pattern (UPDATED)
			errors 	 = error string (UPDATED)
			flags  	 = array to receive match flags for each
				   	   possible character
			casesens = true if search is case sensitive
  Exit    : 
  Notes   : 
  ----------------------------------------------------------*/
static void getclass(const char *pat, uint_t& pos, AString& errors, uint8_t flags[256], bool casesens)
{
	// Cache of previous patterns and results
	static struct {
		AString pat;
		uint8_t flags[256];
		uint_t  age;
	} cache[20];
	static uint_t age = 0;
	uint_t maxage = 0, oldest = 0;
	uint_t i;

	// check cache for pattern being previously decoded
	age++;
	for (i = 0; i < NUMBEROF(cache); i++) {
		uint_t len = cache[i].pat.len();

		// check cache entry
		if (len && (CompareCaseN(pat + pos, cache[i].pat, len) == 0)) {
			cache[i].age = age;			// update cache entry's age
			memcpy(flags, cache[i].flags, sizeof(cache[i].flags));
			// skip over pattern
			pos += len;
			return;
		}

		// find oldest cache entry
		if (!i || ((age - cache[i].age) > maxage)) {
			maxage = age - cache[i].age;
			oldest = i;
		}
	}

	{	
		bool flag = true;
		uint_t pos0 = pos;
		
		// clear flags (no matching characters)
		memset(flags, false, 256 * sizeof(*flags));
		
		// inverse mode
		if (pat[pos] == '^') {
			memset(flags, true, 256 * sizeof(*flags));
			flag = false;
			pos++;
		}
		
		while (pat[pos] && (pat[pos] != ']')) {
			// decode first character
			uint_t c = pat[pos++];
			if (c == (uint_t)TOKEN_ESCAPE) c = deescape(pat, pos);

			// if next character is a '-', then the set is a range
			// instead of a single character
			if (pat[pos] == '-') {
				// decode second character
				uint_t c1 = pat[++pos];
				if (c1 == (uint_t)TOKEN_ESCAPE) c1 = deescape(pat, pos);

				uint_t v1 = MIN(c, c1);
				uint_t v2 = MAX(c, c1);

				// set range
				if (casesens) memset(flags + v1, flag, v2 + 1 - v1);
				else {
					c  = tolower(c);
					c1 = tolower(c1);
					v1 = MIN(c, c1);
					v2 = MAX(c, c1);
					memset(flags + v1, flag, v2 + 1 - v1);

					c  = toupper(c);
					c1 = toupper(c1);
					v1 = MIN(c, c1);
					v2 = MAX(c, c1);
					memset(flags + v1, flag, v2 + 1 - v1);
				}
			}
			// else single character
			else if (casesens) flags[c] = flag;
			else flags[tolower(c)] = flags[toupper(c)] = flag;
		}

		// move over terminator, if present
		if (pat[pos] == ']') {
			pos++;

			// ensure nul character does NOT succeed!
			flags[0] = false;

			// update cache
			cache[oldest].pat.Delete();
			cache[oldest].pat.Create(pat + pos0, pos - pos0);
			memcpy(cache[oldest].flags, flags, sizeof(cache[oldest].flags));
			cache[oldest].age = age;
		}
		else errors.printf("Unterminated class at '%s'\n", AString(pat + pos0 - 1).str());
	}
}

/*------------------------------------------------------------
  Function: Function prototype
  ----------------------------------------------------------*/
static bool match(const AString& str, const AString& pat, uint_t strpos, uint_t patpos, REGEXDATA& data, const BRACKETS& brackets);

/*------------------------------------------------------------
  Function: Handle multiple patterns separated by 'OR' tokens
  Enter   : str      = plain text string
			strpos   = position in string
			pat      = parsed regex pattern
			patpos   = position in pattern
			pat1     = pattern to follow on from main pattern
			data     = REGEXDATA structure
			brackets = depths of parentheses, brackets and braces
  Exit    : true if match succeeds
  Notes   : 
  ----------------------------------------------------------*/
static bool match_or(const AString& str, uint_t strpos, const AString& pat, uint_t patpos, const AString& pat1, REGEXDATA& data, const BRACKETS& brackets)
{
	const char *p1 = (const char *)pat + patpos, *p2;
	bool success = false;
	bool allexclude = (p1[0] != 0);

	// keep trying!
	while (data.errors->Empty()) {
		// find end of pattern terminated by 'OR' token
		// or end of string
		p2 = strchr(p1, TOKEN_OR);
		if (!p2) p2 = p1 + strlen(p1);

		// create new pattern by concatenating sub-pattern
		// with follow on pattern
		AString  pat2 = AString(p1, p2 - p1) + pat1;
		BRACKETS brackets1 = brackets;
		bool     exclude = (pat2[0] == '~');		// 'NOT' token (exclusion)?

		// allexclude remains true if all parts are exclusions
		allexclude &= exclude;

		// try matching
		success = match(str, pat2, strpos, exclude ? 1 : 0, data, brackets1);
		
		if (success) {
#ifdef DEBUG_REGEX
			debug("Match '%s' with '%s' succeeded, breaking out of OR-loop\n", str.str(), pat2.str());
#endif
			
			// if match succeeds then break out (but set success
			// to false if a 'NOT' token was found)
			success &= !exclude;

#ifdef DEBUG_REGEX
			debug("Match '%s' with '%s' succeeded, exclude=%u success=%u\n", str.str(), pat2.str(), exclude, success);
#endif
			break;
		}

		// continue if pattern is terminated by 'OR' token
		if (p2[0] == TOKEN_OR) p1 = p2 + 1;
		else {
			p1 = p2;
			break;
		}
	}

	// if we reached the end of the string and all entries were exclusions,
	// set success = true (removes need for '|*' on the end of list of exclusions)
	if (!p1[0] && allexclude) success = data.errors->Empty();

	return success;
}

/*------------------------------------------------------------
  Function: 
  ----------------------------------------------------------*/
static inline void update(char c, BRACKETS& brackets)
{
	switch (c) {
		case '(': brackets.paren++; break;
		case ')': brackets.paren--; break;
		case '[': brackets.brackets++; break;
		case ']': brackets.brackets--; break;
		case '{': brackets.braces++; break;
		case '}': brackets.braces--; break;
		default: break;
	}
}

/*------------------------------------------------------------
  Function: Match pattern against string
  Enter   : str      = plain text string
			pat      = parsed regex pattern
			strpos   = position in string
			patpos   = position in pattern
			data     = REGEXDATA structure
			brackets = depths of parentheses, brackets and braces
  Exit    : true if match succeeds
  Notes   : This function is highly recursive!!
  ----------------------------------------------------------*/
static bool match(const AString& str, const AString& pat, uint_t strpos, uint_t patpos, REGEXDATA& data, const BRACKETS& brackets)
{
#ifdef DEBUG_REGEX
	static uint_t level = 0;
	AString dstr;
#endif
	BRACKETS brackets1 = brackets;
	bool success = false;

#ifdef DEBUG_REGEX
	// dstr.Format("Try match '%s' (%08lx:%08lx+%u) with '%s' (%08lx:%08lx+%u) (brackets: %d, %d, %d)",
	// 			str.Mid(strpos).str(), (ulong_t)&str, (ulong_t)str.str(), strpos,
	// 			pat.Mid(patpos).str(), (ulong_t)&pat, (ulong_t)pat.str(), patpos,
	// 			brackets.paren, brackets.brackets, brackets.braces);
	dstr.Format("Try match '%s' with '%s' (brackets: %d, %d, %d)",
				str.Mid(strpos).str(),
				pat.Mid(patpos).str(),
				brackets.paren, brackets.brackets, brackets.braces);
	debug("%s%s\n", AString("  ").Copies(level++).str(), dstr.str());
#endif
	
	// too much recursion!!
	if (data.recurselevel >= 2000) return false;

	if (data.errors->Valid()) return false;

	data.recurselevel++;

	switch (pat[patpos]) {
		case 0:
			// match is successful if pat[patpos] = str[strpos] = 0
			success = !str[strpos];
			break;
			
		case '?':
			// match any character
			update(str[strpos], brackets1);
			success = (str[strpos] && match(str, pat, strpos + 1, patpos + 1, data, brackets1));
			break;
			
		case '*':
			// match any character(s)
			success = match(str, pat, strpos, patpos + 1, data, brackets1);
			if (!success) {
				update(str[strpos], brackets1);
				success = (str[strpos] && match(str, pat, strpos + 1, patpos, data, brackets1));
			}
			break;

		case '#':		// zero or more of the following sub-pattern
		case '@':		// one or more of the following sub-pattern
		case '<': {		// specified range of the following sub-pattern
			// For multiple sub-pattern matches, a new pattern is made up
			// from a number of repeats of the sub-pattern concatenated with
			// the rest of the main pattern.  The number of repeats of the
			// sub-pattern is varied in a loop until the match is successful.
			//
			// For normal, non-greedy, matches, the number is varied from the
			// lowest number of repeats up to the highest number of repeats
			// until a match is found.
			//
			// For greedy matches, the number is varied from the HIGHEST number
			// of repeats down to the LOWEST number of repeats until a match is
			// found.
			AString pat1;
			uint_t  n1 = 0, n2 = 0;		// n1, n2 = minimum and maximum number of repeats of the sub-pattern
			uint_t  patpos1;

			if (pat[patpos] == '#') {
				patpos++;
				n1 = 0;			// set limits
				n2 = MAX_UNSIGNED(uint_t);	// set limits
			}
			else if (pat[patpos] == '@') {
				patpos++;
				n1 = 1;			// set limits
				n2 = MAX_UNSIGNED(uint_t);	// set limits
			}
			else if (pat[patpos] == '<') {
				AValue val1, val2;

				patpos++;

				// set lower limit

				// swallow whitespace
				while (IsWhiteSpace(pat[patpos])) patpos++;

				// allow '*' to be used to specifiy a maximum value
				if (pat[patpos] == '*') {
					val1 = MAX_UNSIGNED(uint_t);
					patpos++;
				}
				// else evaluate number
				else patpos = val1.EvalNumber(pat, patpos, false);

				// swallow whitespace
				while (IsWhiteSpace(pat[patpos])) patpos++;

				// if next character is ':', '-' or ',', then set the 
				// upper limit
				if ((pat[patpos] == ':') ||
					(pat[patpos] == '-') ||
					(pat[patpos] == ',')) {
					patpos++;

					// swallow whitespace
					while (IsWhiteSpace(pat[patpos])) patpos++;

					// allow '*' to be used to specifiy a maximum value
					if (pat[patpos] == '*') {
						val2 = MAX_UNSIGNED(uint_t);
						patpos++;
					}
					// else evaluate number
					else patpos = val2.EvalNumber(pat, patpos, false);

					// swallow whitespace
					while (IsWhiteSpace(pat[patpos])) patpos++;
				}

				// end terminator
				if (pat[patpos] == '>') {
					patpos++;

					// set limits according to evaluated values
					n1 = val1.IsValid() ? (uint_t)val1 : 0;
					n2 = val2.IsValid() ? (uint_t)val2 : MAX_UNSIGNED(uint_t);
					
					// make sure n1 < n2
					if (n1 > n2) {
						n2 ^= n1;
						n1 ^= n2;
						n2 ^= n1;
					}
				}
				else data.errors->printf("Missing '>' after range specified at '%s'\n", pat.Mid(patpos).str());
			}

			if (data.errors->Empty()) {
				// test for 'greedy' match
				bool greedy = (pat[patpos] == '*');

				if (greedy) patpos++;

				// extract sub-pattern
				if (pat[patpos] == '(') {
					// simple group marked by parentheses
					patpos1 = find(pat, patpos + 1, ')', *data.errors, TOKEN_ESCAPE);
					if (data.errors->Empty()) {
						if (pat[patpos1] == ')') patpos1++;
						pat1 = pat.Mid(patpos, patpos1 - patpos);
					}
				}
				else if (pat[patpos] == '[') {
					// class marked by brackets
					patpos1 = find_simple(pat, patpos + 1, ']', TOKEN_ESCAPE);
					if (data.errors->Empty()) {
						if (pat[patpos1] == ']') patpos1++;
						pat1 = pat.Mid(patpos, patpos1 - patpos);
					}
				}
				else if (pat[patpos] == '{') {
					// region marked by braces
					patpos1 = find(pat, patpos + 1, '}', *data.errors, TOKEN_ESCAPE);
					if (data.errors->Empty()) {
						if (pat[patpos1] == '}') patpos1++;
						pat1 = pat.Mid(patpos, patpos1 - patpos);
					}
				}
				else if (pat[patpos] == TOKEN_ESCAPE) {
					// escaped character
					patpos1 = patpos + 1;
					deescape(pat, patpos1);
					pat1 = pat.Mid(patpos, patpos1 - patpos);
				}
				else if (pat[patpos] == '$') {
					// 'dollar' string (pre-defined classes)
					patpos1 = patpos + 1;
					check_dollar_str(pat, patpos1);
					pat1 = pat.Mid(patpos, patpos1 - patpos);
				}
				else {
					// single character
					pat1 = pat.Mid(patpos, 1);
					patpos1 = patpos + 1;
				}

				// limit limits to reasonnable values
				if (n1 == MAX_UNSIGNED(uint_t)) n1 = MIN(n1, str.len() - strpos);
				if (n2 == MAX_UNSIGNED(uint_t)) n2 = MIN(n2, MAX(n1, str.len() - strpos));
				
				if (data.errors->Empty()) {
					AString pat2 = pat1.Copies(n2) + pat.Mid(patpos1);
					uint_t i, l = pat1.len(), n = n2 - n1, endindex = n2 - n1;
					
#ifdef DEBUG_REGEX
					debug("%ssub pat = '%s', next pat = '%s', n1 = %u, n2 = %u, pat2 = '%s'\n", AString("  ").Copies(level).str(), pat1.str(), pat.Mid(patpos1).str(), n1, n2, pat2.str());
#endif

					for (i = 0; data.errors->Empty() && (i <= n); i++) {
						uint_t index = greedy ? i : (endindex - i);

#ifdef DEBUG_REGEX
						debug("%sleft = '%s', right = '%s'\n", AString("  ").Copies(level).str(), pat2.Left(index * l).str(), pat2.Mid(index * l).str());
#endif

						success = match(str, pat2, strpos, index * l, data, brackets1);
						if (success) break;
					}
					
#ifdef DEBUG_REGEX
					debug("%ssub pat = '%s', next pat = '%s', success = %u, i = %u\n", AString("  ").Copies(level).str(), pat1.str(), pat.Mid(patpos1).str(), success, i);
#endif
				}
			}
			break;
		}

		case '&': {		// repeated opening, centre and closing patterns
			uint_t patpos1;
			bool greedy = (pat[++patpos] == '*');

			if (greedy) patpos++;

			// sequence is specified using <opening>:<centre>:<closing>: layout
			// find end of opening pattern
			patpos1 = find(pat, patpos, ':', *data.errors, TOKEN_ESCAPE);
			if (data.errors->Empty() && (pat[patpos1] == ':')) {
				AString opening = pat.Mid(patpos, patpos1 - patpos);

				// find end of centre pattern
				patpos  = patpos1 + 1;
				patpos1 = find(pat, patpos, ':', *data.errors, TOKEN_ESCAPE);
				if (data.errors->Empty() && (pat[patpos1] == ':')) {
					AString centre = pat.Mid(patpos, patpos1 - patpos);
					
					// find end of closing pattern
					patpos  = patpos1 + 1;
					patpos1 = find(pat, patpos, ':', *data.errors, TOKEN_ESCAPE);
					if (data.errors->Empty() && (pat[patpos1] == ':')) {
						AString closing = pat.Mid(patpos, patpos1 - patpos);
						AString rest, pat1;
						uint_t i, n = (str.len() - strpos + 1) / 2;

						patpos = patpos1 + 1;
						rest   = pat.Mid(patpos);

#ifdef DEBUG_REGEX
						debug("%sopening = '%s', centre = '%s', closing = '%s', rest = '%s', n = %u\n", AString("  ").Copies(level).str(), opening.str(), centre.str(), closing.str(), rest.str(), n);
#endif

						// try n * <opening> + centre + n * <closing>
						for (i = 0; data.errors->Empty() && (i < n); i++) {
							if (greedy) pat1 = opening.Copies(n - i) + centre + closing.Copies(n - i) + rest;
							else		pat1 = opening.Copies(i + 1) + centre + closing.Copies(i + 1) + rest;
							success = match(str, pat1, strpos, 0, data, brackets1);
							if (success) break;
						}
					}
					else if (data.errors->Empty()) data.errors->printf("Missing terminating ':' for closing pattern for '&' at '%s'\n", pat.Mid(patpos).str());
				}
				else if (data.errors->Empty()) data.errors->printf("Missing terminating ':' for centre pattern for '&' at '%s'\n", pat.Mid(patpos).str());
			}
			else if (data.errors->Empty()) data.errors->printf("Missing terminating ':' for opening pattern for '&' at '%s'\n", pat.Mid(patpos).str());
			break;
		}

		case '[': {		// class
			uint8_t flags[256];

			patpos++;
			getclass(pat, patpos, *data.errors, flags, data.casesens);
			if (data.errors->Empty() && flags[(uint_t)str[strpos]]) {
				success = match(str, pat, strpos + 1, patpos, data, brackets1);
			}
			break;
		}

		case '{': {		// region
			AString marker;
			uint_t 	patpos1;
			AString pat1, pat2;
			REGEXREGION *reg = NULL;

			patpos1 = find(pat, patpos + 1, '}', *data.errors, TOKEN_ESCAPE);
			pat1    = pat.Mid(patpos + 1, patpos1 - patpos - 1);

			if (data.errors->Empty() && (pat[patpos1] == '}')) {
				patpos1++;

				// extract optional marker string
				marker = getmarker(pat1, *data.errors);

				if (data.errors->Empty()) {
					// if region list supplied, create region structure
					if (data.regions && ((reg = new REGEXREGION) != NULL)) {
#ifdef DEBUG_REGEX
						debug("%sRegion %u: marker = '%s', pos = %u - '%s'\n", AString("  ").Copies(level).str(), data.regions->Count(), marker.str(), strpos, str.Mid(strpos, 20).str());
#endif

						// add [empty] region structre to list
						data.regions->Add((uptr_t)reg);

						// fill in parts of region structure
						reg->marker   = marker;
						reg->pos      = strpos;
						
						// mark end of this region in new pattern
#if SYSTEM_IS_64BITS
						pat2.Format("}$%016lx:%s", (ulong_t)reg, pat.str() + patpos1);
#else
						pat2.Format("}$%08lx:%s", (ulong_t)reg, pat.str() + patpos1);
#endif

						// try matching
						success = match_or(str, strpos, pat1, 0, pat2, data, brackets1);
						
						if (!success) {
#ifdef DEBUG_REGEX
							debug("%sDeleting region %u\n", AString("  ").Copies(level).str(), data.regions->Count() - 1);
#endif
							
							data.regions->Remove((uptr_t)reg);
							delete reg;
						}
					}
					else {		// no region list, don't store the data!
						pat2.Format("}:%s", pat.str() + patpos1);
						success = match_or(str, strpos, pat1, 0, pat2, data, brackets1);
					}
				}
			}
			else if (data.errors->Empty()) data.errors->printf("Missing '}' at '%s'\n", pat.Mid(patpos).str());
			break;
		}

		case '(': {		// standard group match
			uint_t 	patpos1 = find(pat, patpos + 1, ')', *data.errors, TOKEN_ESCAPE);
			AString pat1    = split(pat.Mid(patpos + 1, patpos1 - patpos - 1), *data.errors);

			if (data.errors->Empty()) {
				if (pat[patpos1] == ')') patpos1++;
				
				success = match_or(str, strpos, pat1, 0, pat.Mid(patpos1), data, brackets1);
			}
			break;
		}

		case '}': {		// close region (either real or fake)
			REGEXREGION *reg = NULL;

			patpos++;
			if (pat[patpos] != ':') {
				// assume real region -> extract address of region structure
				reg = (REGEXREGION *)(uptr_t)pat.Mid(patpos);

#if SYSTEM_IS_64BITS
				patpos += 17;
#else
				patpos += 9;
#endif

				assert(reg);

				// set region length
				reg->len = strpos - reg->pos;

#ifdef DEBUG_REGEX
				debug("%sClosed region %u: pos = %u, len = %u, str = '%s'\n", AString("  ").Copies(level).str(), data.regions->Find((uptr_t)reg), reg->pos, reg->len, str.Mid(reg->pos, reg->len).str());
#endif
			}

			if (pat[patpos] == ':') patpos++;

			// try to match the rest of the pattern
			success = match(str, pat, strpos, patpos, data, brackets1);
			break;
		}

		case '$':		// clever classes
			patpos++;

			// bracket count reset
			if (pat[patpos] == 'r') {
				patpos++;
				switch (pat[patpos]) {
					case ')': brackets1.paren    = 0; patpos++; break;
					case ']': brackets1.brackets = 0; patpos++; break;
					case '}': brackets1.braces   = 0; patpos++; break;
					default: break;
				}
				success = match(str, pat, strpos, patpos, data, brackets1);
			}
			// bracket count check
			else if (pat[patpos] == 'c') {
				// closing bracket checking
				int  n = 0, n1 = 0;
				bool neg = false;

				patpos++;

				// parse optional minus indicator
				neg = IsNegativeChar(pat[patpos]);
				if (neg) patpos++;

				// calc index
				while (IsNumeralChar(pat[patpos])) {
					n = n * 10 + (pat[patpos++] - '0');
				}
				if (neg) n = -n;

				update(str[strpos], brackets1);

				switch (pat[patpos]) {
					case ')': n1 = brackets1.paren; break;
					case ']': n1 = brackets1.brackets; break;
					case '}': n1 = brackets1.braces; break;
					default: break;
				}

#ifdef DEBUG_REGEX
				debug("%sstr[strpos] = '%c', pat[patpos] = '%c', brackets1 = {%d, %d, %d}\n", AString("  ").Copies(level).str(), str[strpos], pat[patpos], brackets1.paren, brackets1.brackets, brackets1.braces);
#endif

				if ((str[strpos] == pat[patpos]) && (n1 == n)) {
					success = match(str, pat, strpos + 1, patpos + 1, data, brackets1);
				}
			}
			else if (pat[patpos] == '<') {
				// start of line
				if (strpos == 0) {
					success = match(str, pat, strpos, patpos + 1, data, brackets1);
				}
			}
			else if (pat[patpos] == '>') {
				// end of line
				if (!str[strpos]) {
					success = match(str, pat, strpos, patpos + 1, data, brackets1);
				}
			}
			else if (pat[patpos] == '*') {
				// anything gives success
				success = true;
			}
			else {		// pre-defined character type classes
				static const char *whitespace = "\r\n\t ";
				static const char *punct      = ".,;:!?-~ '\"";
				char c = str[strpos], c1;
				bool matched = false;
				
				bool invert = (pat[patpos] == '^');
				if (invert) patpos++;

				bool justupper = (pat[patpos] == 'u');
				bool justlower = (pat[patpos] == 'l');

				if (justupper || justlower) patpos++;

				if (c) {
					update(c, brackets1);

					switch (pat[patpos]) {
						case 'd':
							matched = IsNumeralChar(c);
							break;
					
						case 'w':
							matched = IsWordChar(c);
							break;
					
						case 'a':
							matched = (IsAlphaChar(c) || IsNumeralChar(c));
							break;
					
						case 's':
							matched = (strchr(whitespace, c) != NULL);
							break;

						case 'p':
							matched = (strchr(punct, c) != NULL);
							break;

						case 'q':
							matched = IsQuoteChar(c);
							break;

						case 'b':
							c1 = str[strpos - 1];
							matched = ((IsAlphaChar(c) || IsNumeralChar(c)) && !IsWordChar(c1));
							break;

						case 'e':
							c1 = str[strpos + 1];
							matched = ((IsAlphaChar(c) || IsNumeralChar(c) || (c == '\'')) && !IsWordChar(c1));
							break;

						default:
							break;
					}

					// if justupper or justlower, eliminate those which
					// contain the other
					if (justupper) matched &= !RANGE(c, 'a', 'z');
					if (justlower) matched &= !RANGE(c, 'A', 'Z');

					if (matched != invert) {
						success = match(str, pat, strpos + 1, patpos + 1, data, brackets1);
					}
				}
			}
			break;

		case TOKEN_ESCAPE: {
			// escaped character match
			char c = str[strpos];

			patpos++;
			char c1 = deescape(pat, patpos);

			update(c, brackets1);

			if (c && ((data.casesens && (c == c1)) || (!data.casesens && (tolower(c) == tolower(c1))))) {
				success = match(str, pat, strpos + 1, patpos, data, brackets1);
			}
			break;
		}

		default: {
			// default character match
			char c = str[strpos], c1 = pat[patpos];

			update(c, brackets1);

			if (c && ((data.casesens && (c == c1)) || (!data.casesens && (tolower(c) == tolower(c1))))) {
				//printf("New match for %c/%c\n", str[strpos + 1], pat[patpos + 1]);
				success = match(str, pat, strpos + 1, patpos + 1, data, brackets1);
			}
			break;
		}
	}

#ifdef DEBUG_REGEX
	debug("%s%s: %u\n", AString("  ").Copies(--level).str(), dstr.str(), success);
#endif

	data.recurselevel--;

	return success;
}

/*------------------------------------------------------------
  Function: Match regex pattern against string
  Enter   : str      = string
			pat      = parsed regex pattern
			regions  = optional ptr to list of regex regions
			errors   = error string (UPDATED)
			casesens = true for case sensitive match
			recurselevel = recursion level
  Exit    : true if match successful
  Notes   : 
  ----------------------------------------------------------*/
static bool matchex(const AString& str, const AString& pat, ADataList *regions, AString& errors, bool casesens)
{
	REGEXDATA data = {regions, &errors, casesens, 0};
	BRACKETS brackets = {0, 0, 0};
	return match_or(str, 0, pat, (pat[0] == TOKEN_REGEX) ? 1 : 0, "", data, brackets);
}

/*----------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------
  Function: Parse regex pattern
  Enter   : str	    = pattern
			errors  = error string (UPDATED)
			escchar = escape character
  Exit    : Parsed regex pattern
  Notes   : 
  ----------------------------------------------------------*/
AString ParseRegex(const AString& str, AString& errors, char escchar)
{
	uint_t pos = find(str, 0, 0, errors, escchar);
	if (errors.Empty() && str[pos]) errors.printf("Extra characters at '%s'\n", str.Mid(pos).str());
	AString pat = errors.Empty() ? (AString((char)TOKEN_REGEX) + split(str, errors, escchar)) : "";
	//debug("Parsed regex '%s' to give '%s', errors '%s'\n", str.str(), pat.str(), errors.str());
	return pat;
}

/*------------------------------------------------------------
  Function: Parse regex pattern
  Enter   : str	    = pattern
			escchar = escape character
  Exit    : Parsed regex pattern
  Notes   : 
  ----------------------------------------------------------*/
AString ParseRegex(const AString& str, char escchar)
{
	AString errors;
	return ParseRegex(str, errors, escchar);
}

/*------------------------------------------------------------
  Function: Parse regex pattern
  Enter   : str	    = pattern
  Exit    : Parsed regex pattern
  Notes   : Escape character is '
  ----------------------------------------------------------*/
AString ParsePathRegex(const AString& str)
{
	AString errors;
	return ParseRegex(str, errors);
}

/*------------------------------------------------------------
  Function: Parse regex pattern as path
  Enter   : str	    = pattern
			errors  = error string (UPDATED)
  Exit    : Parsed regex pattern
  Notes   : Escape character is '
  ----------------------------------------------------------*/
AString ParsePathRegex(const AString& str, AString& errors)
{
	return ParseRegex(str, errors, '\'');
}

/*------------------------------------------------------------
  Function: Desctructor for region lists
  Notes   : 
  ----------------------------------------------------------*/
static void __DeleteRegion(uptr_t val, void *context)
{
	UNUSED(context);
	delete (REGEXREGION *)val;
}

/*------------------------------------------------------------
  Function: Match string against parsed or unparsed pattern
  Enter   : str   	 = string
			pat   	 = pattern (parsed or unparsed)
			casesens = true if match is case-sensitive
			escchar  = escape character
  Exit    : true if match is successful
  Notes   : 
  ----------------------------------------------------------*/
bool MatchRegex(const AString& str, const AString& pat, bool casesens, char escchar)
{
	AString errors;
	bool success;

	if (!IsRegexPattern(pat)) {
		AString pat1 = ParseRegex(pat, errors, escchar);
		success = (errors.Empty() && pat1.len() && (IsRegexAnyPattern(pat1) || matchex(str, pat1, NULL, errors, casesens)));
	}
	else success = (pat.len() && (IsRegexAnyPattern(pat) || matchex(str, pat, NULL, errors, casesens)));

	return success;
}

/*------------------------------------------------------------
  Function: Match string against parsed or unparsed pattern and
			save region info in list
  Enter   : str   	   = string
			pat   	   = pattern (parsed or unparsed)
			regionlist = list of regions (of type REGEXREGION)
			casesens   = true if match is case-sensitive
			escchar    = escape character
  Exit    : true if match is successful
  Notes   : 
  ----------------------------------------------------------*/
bool MatchRegex(const AString& str, const AString& pat, ADataList& regionlist, bool casesens, char escchar)
{
	AString errors;
	bool success;

	regionlist.DeleteList();
	regionlist.SetDestructor(&__DeleteRegion);
	regionlist.EnableDuplication();

	if (!IsRegexPattern(pat)) {
		AString pat1 = ParseRegex(pat, errors, escchar);
		success = (errors.Empty() && pat1.len() && (IsRegexAnyPattern(pat1) || matchex(str, pat1, &regionlist, errors, casesens)));
	}
	else success = (pat.len() && (IsRegexAnyPattern(pat) || matchex(str, pat, &regionlist, errors, casesens)));

	return success;
}

/*------------------------------------------------------------
  Function: Match string against parsed or unparsed pattern
			assuming pattern is a path
  Enter   : str = string
			pat = pattern (parsed or unparsed)
  Exit    : true if match is successful
  Notes   : 
  ----------------------------------------------------------*/
bool MatchPathRegex(const AString& str, const AString& pat)
{
	return MatchRegex(str, pat, PATH_REGEX_CASE_SENSITIVE, '\'');
}

/*------------------------------------------------------------
  Function: Match string against parsed or unparsed pattern
			assuming pattern is a path and save regions in
			a list
  Enter   : str   	   = string
			pat   	   = pattern (parsed or unparsed)
			regionlist = list of regions (of type REGEXREGION)
  Exit    : true if match is successful
  Notes   : 
  ----------------------------------------------------------*/
bool MatchPathRegex(const AString& str, const AString& pat, ADataList& regionlist)
{
	return MatchRegex(str, pat, regionlist, PATH_REGEX_CASE_SENSITIVE, '\'');
}

/*------------------------------------------------------------
  Function: Get Regex Region
  Enter   : str        = original string
			expandstr  = expansion string
			i          = position in expansion string
			regionlist = list of regions
			updater    = string updater
  Exit    : true if expansion work
  Notes   : 
  ----------------------------------------------------------*/
bool GetRegexRegion(const AString& str, const AString& expandstr, uint_t& i, const ADataList& regionlist, AStringUpdate& updater)
{
	uint_t n = regionlist.Count();
	bool success = false;

	if (IsNumeralChar(expandstr[i])) {
		const REGEXREGION *reg;
		uint_t n = 0;
		
		while (IsNumeralChar(expandstr[i])) {
			n = n * 10 + (expandstr[i++] - '0');
		}
		
		AString marker, text;
		// region 0 refers to entire string
		if (n == 0) {
			text    = str;
			success = true;
		}
		// regions 1 upwards represent those captured in match
		else if ((reg = (const REGEXREGION *)regionlist[n - 1]) != NULL) {
			marker  = reg->marker;
			text    = str.Mid(reg->pos, reg->len);
			success = true;
		}

		switch (expandstr[i]) {
			case 'm':
				// marker only
				if (success) updater.Update(marker);
				i++;
				break;

			case 't':
				// text only
				if (success) updater.Update(text);
				i++;
				break;

			case 'b':
				// marker then text
				if (success) {
					updater.Update(marker);
					updater.Update(text);
				}
				i++;
				break;

			default:
				// marker then text
				if (success) {
					updater.Update(marker);
					updater.Update(text);
				}
				break;
		}

		// optional terminator
		if (expandstr[i] == ':') i++;
	}
	else if (expandstr[i] == 'm') {
		AString marker, text;
		uint_t j;
		
		i++;

		{	// get marker
			AStringUpdate updater(&marker);
			while (expandstr[i] && (expandstr[i] != ':')) {
				if (expandstr[i] == '\\') {
					i++;
					updater.Update(deescape(expandstr, i));
				}
				else updater.Update(expandstr[i++]);
			}
		}

		// optional marker terminator
		if (expandstr[i] == ':') i++;
		
		// find named marker
		for (j = 0; j < n; j++) {
			const REGEXREGION *reg = (const REGEXREGION *)regionlist[j];
			
			if (marker == reg->marker) {
				text    = str.Mid(reg->pos, reg->len);
				success = true;
				break;
			}
		}
		
		switch (expandstr[i]) {
			case 'm':
				// marker only
				if (success) updater.Update(marker);
				i++;
				break;
				
			case 't':
				// text only
				if (success) updater.Update(text);
				i++;
				break;
				
			case 'b':
				// marker then text
				if (success) {
					updater.Update(marker);
					updater.Update(text);
				}
				i++;
				break;
				
			default:
				// text only
				if (success) updater.Update(text);
				break;
		}
		
		// optional terminator
		if (expandstr[i] == ':') i++;
	}

	return success;
}

/*------------------------------------------------------------
  Function: Expand regex regions according to expansion string
  Enter   : 
  Exit    : 
  Notes   : 
  ----------------------------------------------------------*/
AString ExpandRegexRegions(const AString& str, const AString& expandstr, const ADataList& regionlist, char expandchar)
{
	AString res;
	AStringUpdate updater(&res);
	uint_t i;

#ifdef SHOW_REGEX_REGIONS
	for (i = 0; i < regionlist.Count(); i++) {
		const REGEXREGION *reg = (const REGEXREGION *)regionlist[i];
		
		debug("Region %u/%u: marker = '%s', string = '%s' (pos = %u, len = %u)\n",
			  i, regionlist.Count(),
			  reg->marker.str(),
			  str.Mid(reg->pos, reg->len).str(),
			  reg->pos, reg->len);
	}
#endif

	for (i = 0; expandstr[i];) {
		if (expandstr[i] == expandchar) {
			i++;

			if (IsNumeralChar(expandstr[i]) || (expandstr[i] == 'm')) {
				// update result with extracted region
				GetRegexRegion(str, expandstr, i, regionlist, updater);
			}
			// else if the expansion character is the escape character, de-escape character
			else if (expandchar == '\\') updater.Update(deescape(expandstr, i));
			// else normal single-character update
			else updater.Update(expandstr[i++]);
		}
		// else if the character is the escape character, de-escape character
		else if (expandstr[i] == '\\') {
			i++;
			updater.Update(deescape(expandstr, i));
		}
		// else normal single-character update
		else updater.Update(expandstr[i++]);
	}

	updater.Flush();

	return res;
}

/*------------------------------------------------------------
  Function: Returns whether supplied pattern is a parsed regex 
			pattern
  Notes   : 
  ----------------------------------------------------------*/
bool IsRegexPattern(const AString& pat)
{
	return (pat[0] == TOKEN_REGEX);
}

/*------------------------------------------------------------
  Function: Returns whether supplied pattern is an exclusion 
			regex pattern
  Notes   : 
  ----------------------------------------------------------*/
bool IsExclusionPattern(const AString& pat)
{
	return ((pat[0] == TOKEN_REGEX) && (pat[1] == '~'));
}

/*------------------------------------------------------------
  Function: Returns whether supplied pattern matches anything
  Notes   : 
  ----------------------------------------------------------*/
bool IsRegexAnyPattern(const AString& pat)
{
	static const char anypattern1[] = {TOKEN_REGEX, '#', '?', 0};
	static const char anypattern2[] = {TOKEN_REGEX, '*', 0};
	static const char anypattern3[] = {'#', '?', 0};
	static const char anypattern4[] = {'*', 0};
	return ((pat == anypattern1) || (pat == anypattern2) ||
			(pat == anypattern3) || (pat == anypattern4));
}
