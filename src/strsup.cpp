
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>

#include "StdFile.h"
#include "strsup.h"
#include "md5.h"

/* end of includes */

#define CREATE_TEXT(l) (new char[l])
#define DELETE_TEXT(p) delete[] p

#define IsWhiteSpaceEx(c)			(IsWhiteSpace(c) || ((c) == '\n'))
#define IsQuoteCharFlags(c)			((((c) == '\'') && (flags & FLAG_SINGLE_QUOTE))       || (((c) == '\"') && (flags & FLAG_DOUBLE_QUOTE)))
#define IsInnerQuoteCharFlags(c)	((((c) == '\'') && (flags & FLAG_INNER_SINGLE_QUOTE)) || (((c) == '\"') && (flags & FLAG_INNER_DOUBLE_QUOTE)))
#define IsEscapeCharFlags(c)		((((c) == '\'') && (flags & FLAG_QUOTE_ESCAPE))       || (((c) == '\\') && (flags & FLAG_SLASH_ESCAPE)))

#ifdef __WIN32__
#define IsDriveSpecifier(pos, c) (((pos) == 1) && ((c) == ':'))
#else
#define IsDriveSpecifier(pos, c) false
#endif

static const char   Base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const uint_t Base64Count   = NUMBEROF(Base64Chars) - 1;
static uint8_t Base64Lookup[256];
static bool Base64LookupInitialised = false;

NODETYPE_IMPLEMENT(AString);

const char *AString::pDefaultText = "";
bool AString::bSlashFromEnd = true;

AString::AString(char *iText, sint_t iLength) : AListNode(),
												pText((char *)pDefaultText),
												Length(0)
{
	Create(iText, iLength);
}

AString::AString(const char *iText, sint_t iLength) : AListNode(),
													  pText((char *)pDefaultText),
													  Length(0)
{
	Create(iText, iLength);
}

AString::AString(const AString *pString, sint_t iLength) : AListNode(),
														   pText((char *)pDefaultText),
														   Length(0)
{
	if (pString) Create(pString->pText, iLength);
}

AString::AString(const AString& String, sint_t iLength) : AListNode(),
														  pText((char *)pDefaultText),
														  Length(0)
{
	Create(String.pText, iLength);
}

AString::AString(bool v) : AListNode(),
						   pText((char *)pDefaultText),
						   Length(0)
{
	operator = (v);
}

AString::AString(char c) : AListNode(),
						   pText((char *)pDefaultText),
						   Length(0)
{
	operator = (c);
}

AString::AString(sshort_t val) : AListNode(),
								 pText((char *)pDefaultText),
								 Length(0)
{
	operator = (val);
}

AString::AString(ushort_t val) : AListNode(),
								 pText((char *)pDefaultText),
								 Length(0)
{
	operator = (val);
}

AString::AString(sint_t val) : AListNode(),
							   pText((char *)pDefaultText),
							   Length(0)
{
	operator = (val);
}

AString::AString(uint_t val) : AListNode(),
							   pText((char *)pDefaultText),
							   Length(0)
{
	operator = (val);
}

#if !SYSTEM_IS_64BITS
AString::AString(sint32_t val) : AListNode(),
								 pText((char *)pDefaultText),
								 Length(0)
{
	operator = (val);
}

AString::AString(uint32_t val) : AListNode(),
								 pText((char *)pDefaultText),
								 Length(0)
{
	operator = (val);
}
#endif

AString::AString(sint64_t val) : AListNode(),
								 pText((char *)pDefaultText),
								 Length(0)
{
	operator = (val);
}

AString::AString(uint64_t val) : AListNode(),
								 pText((char *)pDefaultText),
								 Length(0)
{
	operator = (val);
}

AString::AString(float val) : AListNode(),
							  pText((char *)pDefaultText),
							  Length(0)
{
	operator = (val);
}

AString::AString(double val) : AListNode(),
							   pText((char *)pDefaultText),
							   Length(0)
{
	operator = (val);
}

AString::~AString()
{
	Delete();
}

bool AString::Create(const char *iText, sint_t iLength, bool limit)
{
	char *pStr;
	bool  res = false;
	
	if ((iText == NULL) || (iLength == 0)) iText = (char *)pDefaultText;
	
	if (iText != (char *)pDefaultText) {
		if (iLength < 0) {
			iLength = strlen(iText);
		}
		else if (limit) {
			int l = strlen(iText);
			iLength = MIN(iLength, l);
		}

		if ((pStr = CREATE_TEXT(iLength + 1)) != NULL) {
			memcpy(pStr, iText, iLength);
			pStr[iLength] = 0;

			Delete();

			pText  = pStr;
			Length = strlen(pText);

			res = true;
		}
		else debug("Failed to allocate string memory for '%s'\n", iText);
	}
	else {
		Delete();
		res = true;
	}

	return res;
}

bool AString::Append(const char *iText, sint_t iLength, bool limit)
{
	bool res = false;
	char *pStr;

	if (iLength < 0) {
		iLength = strlen(iText);
	}
	else if (limit) {
		int l = strlen(iText);
		iLength = MIN(iLength, l);
	}

	if (iText && iLength) {
		if ((pStr = CREATE_TEXT(Length + iLength + 1)) != NULL) {
			if (Length)	 memcpy(pStr, pText, Length);
			if (iLength) memcpy(pStr + Length, iText, iLength);
			pStr[Length + iLength] = 0;

			Delete();

			pText  = pStr;
			Length = strlen(pText);

			res = true;
		}
		else debug("Failed to allocate string memory for '%s'\n", iText);
	}
	else res = true;

	return res;
}

void AString::Delete()
{
	if (pText && (pText != (char *)pDefaultText)) DELETE_TEXT(pText);

	pText  = (char *)pDefaultText;
	Length = 0;
}

char *AString::Steal(sint_t *pLength)
{
	char *p = pText;
	if (p == (char *)pDefaultText) {
		if ((p = CREATE_TEXT(1)) != NULL) p[0] = 0;
	}
	if (pLength) *pLength = Length;

	pText  = (char *)pDefaultText;
	Length = 0;

	return p;
}

void AString::Take(const char *p)
{
	Delete();

	pText  = (char *)(p ? p : (char *)pDefaultText);
	Length = strlen(pText);
}

AString AString::ReadFile(const char *filename)
{
	AString str;

	str.ReadFromFile(filename);

	return str;
}

bool AString::ReadFromFile(const char *filename, bool append)
{
	AStdFile fp;
	bool     success = false;

	if (fp.open(filename)) {
		fp.seek(0, SEEK_END);

		size_t l = fp.tell();

		if (l > 0) {
			size_t offset = append ? Length : 0;
			char   *pStr;

			if ((pStr = CREATE_TEXT(offset + l + 1)) != NULL) {
				if (append && (Length > 0)) memcpy(pStr, pText, Length);

				fp.rewind();
				fp.readbytes(pStr + offset, l);

				pStr[offset + l] = 0;

				Delete();

				pText  = pStr;
				Length = strlen(pText);

				success = true;
			}
			else debug("Failed to allocate string memory for file '%s'\n", filename);
		}
		else {
			if (!append) Delete();

			success = true;
		}

		fp.close();
	}

	return success;
}

bool AString::WriteToFile(const char *filename, const char *mode) const
{
	AStdFile fp;
	bool success = false;

	if (fp.open(filename, mode)) {
		fp.writebytes(str(), len());
		fp.close();

		success = true;
	}

	return success;
}

AString *AString::Find(const char *iText, bool bPartial) const
{
	AString *pLine = (AString *)this;
	bool Found = false;
	sint_t l1 = strlen(iText), l2;

	while (pLine && !Found) {
		l2 = pLine->Length;
		
		if (bPartial) Found = (strnicmp(iText, pLine->pText, MIN(l1, l2)) == 0);
		else		  Found = (stricmp(iText, pLine->pText) == 0);
	}

	return pLine;
}

AString *AString::Find(const AString& String, bool bPartial) const
{
	AString *pLine = (AString *)this;
	bool Found = false;
	sint_t l1 = String.Length, l2;

	while (pLine && !Found) {
		l2 = pLine->Length;
		
		if (bPartial) Found = (strnicmp(String.pText, pLine->pText, MIN(l1, l2)) == 0);
		else		  Found = (stricmp(String.pText, pLine->pText) == 0);

		if (!Found) pLine = pLine->Next();
	}
	
	return pLine;
}

AString *AString::Find(const char *iText, int& n, bool bPartial) const
{
	AString *pLine = (AString *)this;
	bool Found = false;
	sint_t l1 = strlen(iText), l2;

	n = 0;
	while (pLine && !Found) {
		l2 = pLine->Length;
		
		if (bPartial) Found = (strnicmp(iText, pLine->pText, MIN(l1, l2)) == 0);
		else		  Found = (stricmp(iText, pLine->pText) == 0);

		if (!Found) {
			pLine = pLine->Next();
			n++;
		}
	}

	return pLine;
}

AString *AString::Find(const AString& String, int& n, bool bPartial) const
{
	AString *pLine = (AString *)this;
	bool Found = false;
	sint_t l1 = String.Length, l2;

	n = 0;
	while (pLine && !Found) {
		l2 = pLine->Length;
		
		if (bPartial) Found = (strnicmp(String.pText, pLine->pText, MIN(l1, l2)) == 0);
		else		  Found = (stricmp(String.pText, pLine->pText) == 0);

		if (!Found) {
			pLine = pLine->Next();
			n++;
		}
	}

	return pLine;
}

AString *AString::FindBackwards(const char *iText, bool bPartial) const
{
	AString *pLine = (AString *)this;
	bool Found = false;
	sint_t l1 = strlen(iText), l2;

	while (pLine && !Found) {
		l2 = pLine->Length;
		
		if (bPartial) Found = (strnicmp(iText, pLine->pText, MIN(l1, l2)) == 0);
		else		  Found = (stricmp(iText, pLine->pText) == 0);

		if (!Found)	pLine = pLine->Prev();
	}

	return pLine;
}

AString *AString::FindBackwards(const AString& String, bool bPartial) const
{
	AString *pLine = (AString *)this;
	bool Found = false;
	sint_t l1 = String.Length, l2;

	while (pLine && !Found) {
		l2 = pLine->Length;
		
		if (bPartial) Found = (strnicmp(String.pText, pLine->pText, MIN(l1, l2)) == 0);
		else		  Found = (stricmp(String.pText, pLine->pText) == 0);

		if (!Found) pLine = pLine->Prev();
	}

	return pLine;
}

AString *AString::FindBackwards(const char *iText, int& n, bool bPartial) const
{
	AString *pLine = (AString *)this;
	bool Found = false;
	sint_t l1 = strlen(iText), l2;

	n = 0;
	while (pLine && !Found) {
		l2 = pLine->Length;
		
		if (bPartial) Found = (strnicmp(iText, pLine->pText, MIN(l1, l2)) == 0);
		else		  Found = (stricmp(iText, pLine->pText) == 0);

		if (!Found) {
			pLine = pLine->Prev();
			n--;
		}
	}

	return pLine;
}

AString *AString::FindBackwards(const AString& String, int& n, bool bPartial) const
{
	AString *pLine = (AString *)this;
	bool Found = false;
	sint_t l1 = String.Length, l2;

	n = 0;
	while (pLine && !Found) {
		l2 = pLine->Length;
		
		if (bPartial) Found = (strnicmp(String.pText, pLine->pText, MIN(l1, l2)) == 0);
		else		  Found = (stricmp(String.pText, pLine->pText) == 0);

		if (!Found) {
			pLine = pLine->Prev();
			n--;
		}
	}

	return pLine;
}

AString& AString::operator = (bool v)
{
	char *pStr;

	if ((pStr = CREATE_TEXT(2)) != NULL) {
		pStr[0] = v ? '1' : '0';
		pStr[1] = 0;

		Delete();
		pText  = pStr;
		Length = 1;
	}

	return *this;
}

AString& AString::operator = (char c)
{
	char *pStr;

	if ((pStr = CREATE_TEXT(2)) != NULL) {
		pStr[0] = c;
		pStr[1] = 0;

		Delete();
		pText  = pStr;
		Length = 1;
	}

	return *this;
}

AString& AString::operator = (char *iText)
{
	Create(iText);

	return *this;
}

AString& AString::operator = (const char *iText)
{
	Create(iText);

	return *this;
}

AString& AString::operator = (const AString *pString)
{
	Create(pString->pText);

	return *this;
}

AString& AString::operator = (const AString& String)
{
	Create(String.pText);

	return *this;
}

AString& AString::operator = (sint16_t val)
{
	Create(AValue(val).GenerateString());
	return *this;
}

AString& AString::operator = (uint16_t val)
{
	Create(AValue(val).GenerateString());
	return *this;
}

AString& AString::operator = (sint_t val)
{
	Create(AValue(val).GenerateString());
	return *this;
}

AString& AString::operator = (uint_t val)
{
	Create(AValue(val).GenerateString());
	return *this;
}

#if !SYSTEM_IS_64BITS
AString& AString::operator = (sint32_t val)
{
	Create(AValue(val).GenerateString());
	return *this;
}

AString& AString::operator = (uint32_t val)
{
	Create(AValue(val).GenerateString());
	return *this;
}
#endif

AString& AString::operator = (sint64_t val)
{
	Create(AValue(val).GenerateString());
	return *this;
}

AString& AString::operator = (uint64_t val)
{
	Create(AValue(val).GenerateString());
	return *this;
}

AString& AString::operator = (float val)
{
	Create(AValue(val).GenerateString());
	return *this;
}

AString& AString::operator = (double val)
{
	Create(AValue(val).GenerateString());
	return *this;
}

AString& AString::operator += (char c)
{
	char *pStr;

	if ((pStr = CREATE_TEXT(Length + 2)) != NULL) {
		strcpy(pStr, pText);

		pStr[Length] = c;
		pStr[Length + 1] = 0;

		Delete();
		
		pText = pStr;
		Length = strlen(pText);
	}

	return *this;
}

AString& AString::operator += (const char *iText)
{
	sint_t l = strlen(iText);

	if (l > 0) {
		char *pStr;

		if ((pStr = CREATE_TEXT(Length + l + 1)) != NULL) {
			strcpy(pStr, pText);
			strcpy(pStr + Length, iText);

			Delete();
			
			pText = pStr;
			Length = strlen(pText);
		}
	}

	return *this;
}

AString& AString::operator += (const AString *pString)
{
	sint_t l = strlen(pString->pText);

	if (l > 0) {
		char *pStr;

		if ((pStr = CREATE_TEXT(Length + l + 1)) != NULL) {
			strcpy(pStr, pText);
			strcpy(pStr + Length, pString->pText);

			Delete();
			
			pText = pStr;
			Length = strlen(pText);
		}
	}

	return *this;
}

AString& AString::operator += (const AString& String)
{
	sint_t l = strlen(String.pText);

	if (l > 0) {
		char *pStr;

		if ((pStr = CREATE_TEXT(Length + l + 1)) != NULL) {
			strcpy(pStr, pText);
			strcpy(pStr + Length, String.pText);

			Delete();
			
			pText = pStr;
			Length = strlen(pText);
		}
	}

	return *this;
}

AString operator + (const AString& Object1, const char *iText2)
{
	AString res = Object1;
	res += iText2;

	return res;
}

AString operator + (const char *iText1, const AString& Object2)
{
	AString res = iText1;
	res += Object2;

	return res;
}

AString operator + (const AString& Object1, const AString& Object2)
{
	AString res = Object1;
	res += Object2;

	return res;
}

static char deescape(const char *str, sint_t& pos)
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

sint_t AString::FindEndQuote(char q, sint_t length, const char *p, uint_t flags)
{
	sint_t i;

	for (i = 0; (i < length) && p[i];) {
		if (IsEscapeCharFlags(p[i])) {
			i++;
			deescape(p, i);
		}
		else if (p[i] == q) break;
		else i++;
	}

	return i;
}

AString::operator bool() const
{
	return (operator sint64_t() != 0);
}

AString::operator float() const
{
	return (float)operator double();
}

AString::operator double() const
{
	return (double)EvalNumber(0, NULL, true);
}

AString::operator sint64_t() const
{
	return (sint64_t)EvalNumber(0, NULL, true);
}

AValue AString::EvalNumber(uint_t i, uint_t *endIndex, bool allowModifiers, const char *terminators, AString *error) const
{
	AValue value;

	i = MIN(i, (uint_t)Length);
	uint_t n = value.EvalNumber(pText + i, allowModifiers, terminators, error) - pText;

	if (endIndex) *endIndex = n;

	return value;
}

AString& AString::ConvertToHex(double val)
{
	uint64_t uval;
	memcpy(&uval, &val, sizeof(val));
	Format("#%016" FMT64 "x", uval);
	return *this;
}

AString& AString::ConvertToHex(uint_t val, bool pad)
{
	if (pad) Format("$%08x", val);
	else	 Format("$%x", val);
	return *this;
}

#if !SYSTEM_IS_64BITS
AString& AString::ConvertToHex(uint32_t val, bool pad)
{
#if LONG_IS_64BITS
	if (pad) Format("$%08x", val);
	else	 Format("$%x", val);
#else
	if (pad) Format("$%08lx", val);
	else	 Format("$%lx", val);
#endif
	return *this;
}

AString& AString::ConvertToHex(uint64_t val, bool pad)
{
#if LONG_IS_64BITS
	if (pad) Format("$%08lx", val);
	else	 Format("$%lx", val);
#else
	if (pad) Format("$%08llx", val);
	else	 Format("$%llx", val);
#endif
	return *this;
}
#endif

void AString::Format(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	FormatV(format, ap);
	va_end(ap);
}

void AString::FormatV(const char *format, va_list ap)
{
	char *buffer = NULL;
	int l = vasprintf(&buffer, format, ap);
	
	if (buffer && (l > 0)) {
		Create(buffer);
	}
	else Delete();
	
	if (buffer) free(buffer);
}

AString AString::Formatify(const char *format, ...)
{
	AString str;
	va_list ap;

	va_start(ap, format);
	str.FormatV(format, ap);
	va_end(ap);

	return str;
}

AString AString::FormatifyV(const char *format, va_list ap)
{
	AString str;

	str.FormatV(format, ap);

	return str;
}

void AString::printf(const char *format,...)
{
	va_list ap;

	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}

void AString::vprintf(const char *format, va_list ap)
{
	char *buffer = NULL;
	int l = vasprintf(&buffer, format, ap);
	
	if (buffer && (l > 0)) {
		Append(buffer);
	}
	
	if (buffer) free(buffer);
}

sint_t AString::FindFormatSpecifier(sint_t& len) const
{
	len = 0;

	sint_t i, j;
	for (i = 0; i < Length; i++) {
		if ((pText[i] == '%') && (pText[i + 1] != '%')) {
			for (j = 1; (i + j) < Length; j++) {
				static const char *Specifiers = "uidxefpsc";
				if (strchr(Specifiers, tolower(pText[i + j]))) {
					len = j + 1;
					break;
				}
				else if (pText[i + j] == '%') break;
			}
			break;
		}
	}

	return ((i < Length) && (len > 0)) ? i : -1;
}

AString AString::Arg(const char *iText) const
{
	AString str = *this;
	sint_t pos, len;

	if ((pos = str.FindFormatSpecifier(len)) >= 0) {
		AString str2;

		str2.Format(str.Mid(pos, len), iText);
		str = str.Left(pos) + str2 + str.Mid(pos + len);
	}

	return str;
}

AString AString::Arg(const AString& String) const
{
	AString str = *this;
	sint_t pos, len;

	if ((pos = str.FindFormatSpecifier(len)) >= 0) {
		AString str2;

		str2.Format(str.Mid(pos, len), String.str());
		str = str.Left(pos) + str2 + str.Mid(pos + len);
	}

	return str;
}

AString AString::Arg(sint16_t n) const
{
	AString str = *this;
	sint_t pos, len;

	if ((pos = str.FindFormatSpecifier(len)) >= 0) {
		AString str2;

		str2.Format(str.Mid(pos, len), n);
		str = str.Left(pos) + str2 + str.Mid(pos + len);
	}

	return str;
}

AString AString::Arg(uint16_t n) const
{
	AString str = *this;
	sint_t pos, len;

	if ((pos = str.FindFormatSpecifier(len)) >= 0) {
		AString str2;

		str2.Format(str.Mid(pos, len), n);
		str = str.Left(pos) + str2 + str.Mid(pos + len);
	}

	return str;
}

AString AString::Arg(sint_t n) const
{
	AString str = *this;
	sint_t pos, len;

	if ((pos = str.FindFormatSpecifier(len)) >= 0) {
		AString str2;

		str2.Format(str.Mid(pos, len), n);
		str = str.Left(pos) + str2 + str.Mid(pos + len);
	}

	return str;
}

AString AString::Arg(uint_t n) const
{
	AString str = *this;
	sint_t pos, len;

	if ((pos = str.FindFormatSpecifier(len)) >= 0) {
		AString str2;

		str2.Format(str.Mid(pos, len), n);
		str = str.Left(pos) + str2 + str.Mid(pos + len);
	}

	return str;
}

#if !SYSTEM_IS_64BITS
AString AString::Arg(sint32_t n) const
{
	AString str = *this;
	sint_t pos, len;

	if ((pos = str.FindFormatSpecifier(len)) >= 0) {
		AString str2;

		str2.Format(str.Mid(pos, len), n);
		str = str.Left(pos) + str2 + str.Mid(pos + len);
	}

	return str;
}

AString AString::Arg(uint32_t n) const
{
	AString str = *this;
	sint_t pos, len;

	if ((pos = str.FindFormatSpecifier(len)) >= 0) {
		AString str2;

		str2.Format(str.Mid(pos, len), n);
		str = str.Left(pos) + str2 + str.Mid(pos + len);
	}

	return str;
}
#endif

AString AString::Arg(sint64_t n) const
{
	AString str = *this;
	sint_t pos, len;

	if ((pos = str.FindFormatSpecifier(len)) >= 0) {
		AString str2;

		str2.Format(str.Mid(pos, len), n);
		str = str.Left(pos) + str2 + str.Mid(pos + len);
	}

	return str;
}

AString AString::Arg(uint64_t n) const
{
	AString str = *this;
	sint_t pos, len;

	if ((pos = str.FindFormatSpecifier(len)) >= 0) {
		AString str2;

		str2.Format(str.Mid(pos, len), n);
		str = str.Left(pos) + str2 + str.Mid(pos + len);
	}

	return str;
}

AString AString::Arg(float n) const
{
	AString str = *this;
	sint_t pos, len;

	if ((pos = str.FindFormatSpecifier(len)) >= 0) {
		AString str2;

		str2.Format(str.Mid(pos, len), n);
		str = str.Left(pos) + str2 + str.Mid(pos + len);
	}

	return str;
}

AString AString::Arg(double n) const
{
	AString str = *this;
	sint_t pos, len;

	if ((pos = str.FindFormatSpecifier(len)) >= 0) {
		AString str2;

		str2.Format(str.Mid(pos, len), n);
		str = str.Left(pos) + str2 + str.Mid(pos + len);
	}

	return str;
}

AString AString::Arg(const void *p) const
{
	AString str = *this;
	sint_t pos, len;

	if ((pos = str.FindFormatSpecifier(len)) >= 0) {
		AString str2;

		str2.Format(str.Mid(pos, len), p);
		str = str.Left(pos) + str2 + str.Mid(pos + len);
	}

	return str;
}

AString AString::Left(sint_t length) const
{
	AString String;

	length = MIN(length, Length);
	if (length > 0) String.Create(pText, length);

	return String;
}

AString AString::Mid(sint_t pos, sint_t length) const
{
	AString String;

	if ((pos >= 0) && (pos < Length)) {
		if (length < 0) length = Length - pos;

		if ((pos + length) > Length) length = Length - pos;

		if ((length > 0) && (length <= Length)) {
			String.Create(pText + pos, length);
		}
	}

	return String;
}

AString AString::Right(sint_t length) const
{
	AString String;

	if ((length > 0) && (length <= Length)) {
		String.Create(pText + Length - length, length);
	}

	return String;
}

AString AString::Prefix() const
{
	AString String;
	sint_t pos = LastPos(".");

	if (pos > 0) String = Left(pos);
	else		 String = *this;

	return String;
}

AString AString::Suffix() const
{
	AString String;
	sint_t pos = LastPos(".");

	if (pos > 0) String = Mid(pos + 1);

	return String;
}

AString AString::SearchAndReplace(const char *pSearch, const char *pReplace, uint_t flags) const
{
	AString String;
	const sint_t l = strlen(pSearch), end = len() - l;
	sint_t p = 0, p1;

	while ((p1 = Pos(pSearch, p)) >= 0) {
		bool valid = ((!(flags & REPLACE_START) || (p1 == 0)) && (!(flags & REPLACE_END) || (p1 == end)));

		if (valid) {
			String.Append(pText + p, p1 - p);
			String.Append(pReplace);
			p = p1 + l;
		}
		else {
			p1++;
			String.Append(pText + p, p1 - p);
			p = p1;
		}
	}
	String.Append(pText + p);

	return String;
}

AString AString::SearchAndReplaceNoCase(const char *pSearch, const char *pReplace, uint_t flags) const
{
	AString String;
	const sint_t l = strlen(pSearch), end = len() - l;
	sint_t p = 0, p1;

	while ((p1 = PosNoCase(pSearch, p)) >= 0) {
		bool valid = ((!(flags & REPLACE_START) || (p1 == 0)) && (!(flags & REPLACE_END) || (p1 == end)));

		if (valid) {
			String.Append(pText + p, p1 - p);
			String.Append(pReplace);
			p = p1 + l;
		}
		else {
			p1++;
			String.Append(pText + p, p1 - p);
			p = p1;
		}
	}
	String.Append(pText + p);

	return String;
}

AString AString::ExpandTabs(sint_t TabSize) const
{
	AString		  String;
	AStringUpdate Updater(&String);
	sint_t i;

	for (i = 0; i < Length; i++) {
		if (pText[i] == '\t') {
			Updater.Update(' ');
			while (Updater.TotalLength() % TabSize) Updater.Update(' ');
		}
		else Updater.Update(pText[i]);
	}
	Updater.Flush();

	return String;
}

sint_t AString::OutputLength(sint_t TabSize, sint_t l) const
{
	sint_t i, j;

	if (l < 0) l = Length;

	for (i = j = 0; i < l; i++) {
		if (pText[i] == '\t') j += TabSize - (j % TabSize);
		else                  j++;
	}

	return j;
}

void AString::TabToNextColumn(sint_t TabSize, sint_t ColumnSize)
{
	char *buffer;

	if ((buffer = CREATE_TEXT(32768)) != NULL) {
		sint_t i, j, l, k;

		strcpy(buffer, pText);
		for (i = j = 0; i < Length; i++) {
			if (pText[i] == '\t') j += TabSize - (j % TabSize);
			else                  j++;
		}
		
		k = Length;
		l = j + ColumnSize - (j % ColumnSize);
		while (j < l) {
			buffer[k++] = '\t';
			j += TabSize - (j % TabSize);
		}
		
		buffer[k] = 0;
		Create(buffer);

		DELETE_TEXT(buffer);
	}
}

sint_t AString::ForwardWord(sint_t pos) const
{
	pos = LIMIT(pos, 0, Length);
	
	while ((pos < Length) && !IsWhiteSpaceEx(pText[pos])) pos++;
	while ((pos < Length) &&  IsWhiteSpaceEx(pText[pos])) pos++;

	return pos;
}

sint_t AString::BackwardWord(sint_t pos) const
{
	pos = LIMIT(pos, 0, Length);

	if (pos > 0) pos--;
	while ((pos > 0) &&  IsWhiteSpaceEx(pText[pos])) pos--;
	while ((pos > 0) && !IsWhiteSpaceEx(pText[pos])) pos--;
	if (IsWhiteSpaceEx(pText[pos])) pos++;

	return pos;
}

sint_t AString::StartOfNextWord(sint_t pos, uint_t flags) const
{
	(void)flags;
	
	pos = MIN(pos, Length);
	while ((pos < Length) && IsWhiteSpaceEx(pText[pos])) pos++;

	return pos;
}

sint_t AString::EndOfThisWord(sint_t pos, uint_t flags) const
{
	pos = MIN(pos, Length);
	if (pos < Length) {
		if (IsQuoteCharFlags(pText[pos])) {
			char q = pText[pos++];
			pos += FindEndQuote(q, Length - pos, pText + pos, flags) + 1;
		}
		else {
			while ((pos < Length) && !IsWhiteSpaceEx(pText[pos])) {
				if (IsInnerQuoteCharFlags(pText[pos])) {
					char q = pText[pos++];
					pos += FindEndQuote(q, Length - pos, pText + pos, flags) + 1;
				}
				else if (IsEscapeCharFlags(pText[pos])) {
					pos++;
					deescape(pText, pos);
				}
				else pos++;
			}
		}
	}

	return pos;
}

/*
 * WordUnderPos and CountWords
 *
 *				'abcdef ghijk '
 * WordUnderPos  0000001111112
 *
 * CountWords -> 2
 *
 */
sint_t AString::WordUnderPos(sint_t pos, uint_t flags) const
{
	sint_t n = 0, pos2 = 0;
	pos2 = StartOfNextWord(pos2, flags);

	while (pos2 < pos) {
		pos2 = EndOfThisWord(pos2, flags);
		if (pos2 < pos) {
			n++;
			pos2 = StartOfNextWord(pos2, flags);
		}
	}

	return n;
}

sint_t AString::CountWords(uint_t flags) const
{
	sint_t n = 0, pos = StartOfNextWord(0, flags);
	while (pos < Length) {
		pos = EndOfThisWord(pos, flags);
		pos = StartOfNextWord(pos, flags);
		n++;
	}

	return n;
}

sint_t AString::StartOfWord(sint_t n, uint_t flags) const
{
	sint_t pos = StartOfNextWord(0, flags);
	while (n > 0) {
		pos = EndOfThisWord(pos, flags);
		pos = StartOfNextWord(pos, flags);
		n--;
	}

	return pos;
}

sint_t AString::EndOfWord(sint_t n, uint_t flags) const
{
	sint_t pos = StartOfNextWord(0, flags);
	pos = EndOfThisWord(pos, flags);
	while (n > 0) {
		pos = StartOfNextWord(pos, flags);
		pos = EndOfThisWord(pos, flags);
		n--;
	}

	return pos;
}

AString AString::Words(sint_t nWord, sint_t nWords, uint_t flags) const
{
	AString String;
	sint_t pos1, pos2;

	pos1 = pos2 = StartOfWord(nWord, flags);
	sint_t endword = CountWords(flags);
	if (nWords < MAXWORDS) endword = MIN(endword, nWord + nWords);
	if (endword > 0) pos2 = EndOfWord(endword - 1, flags);

	if (pos2 > pos1) String.Create(pText + pos1, pos2 - pos1);

	return String;
}

AString AString::LastWords(sint_t nWords, uint_t flags) const
{
	return Words(CountWords(flags) - nWords, nWords, flags);
}

void AString::SplitWords(sint_t n, AString& String1, AString& String2, uint_t flags) const
{
	AString string1;
	AString string2;

	if (n > 0) {
		string1.Create(pText, EndOfWord(n - 1, flags));
		string2.Create(pText + StartOfWord(n, flags));
	}
	else {
		string1.Delete();
		string2 = *this;
	}

	String1 = string1;
	String2 = string2;
}

AString AString::CatWords(const AString& word1, const AString& word2) const
{
	AString String = *this;

	if (!word1.Empty()) {
		if		(String.Empty())				   String  = word1;
		else if (!IsWhiteSpace(String.LastChar())) String += " " + word1;
		else									   String += word1;
	}
	if (!word2.Empty()) {
		if		(String.Empty())				   String  = word2;
		else if (!IsWhiteSpace(String.LastChar())) String += " " + word2;
		else									   String += word2;
	}

	return String;
}

AString AString::DelWords(sint_t nWord, sint_t nWords, uint_t flags)
{
	AString String, String1, String2;

	String	= Words(nWord, nWords, flags);
	String1 = Words(0, nWord, flags);
	if (nWords != MAXWORDS) String2 = Words(nWord + nWords, MAXWORDS, flags);

	Create(String1 + " " + String2);

	return String;
}

sint_t AString::PosEx(const char *text, sint_t len, sint_t startpos, sint_t endpos, bool bCase) const
{
	int  (*fn)(const char *p1, const char *p2, size_t n) = bCase ? &strncmp : &strnicmp;
	bool found = false;

	if (len < 0) len = strlen(text);

	if (len > Length) return -1;

	startpos = MIN(startpos, Length);
	startpos = MAX(startpos, 0);
	endpos 	 = MIN(endpos, 	 Length);
	endpos 	 = MAX(endpos, 	 0);

	sint_t i, inc = (endpos > startpos) ? 1 : -1;
	for (i = startpos; !(found = ((*fn)(pText + i, text, len) == 0)) && (i != endpos); i += inc) ;

	return found ? i : -1;
}

sint_t AString::Pos(const char *iText, sint_t startpos, sint_t endpos) const
{
	return PosEx(iText, -1, startpos, endpos);
}

sint_t AString::Pos(const AString& String, sint_t startpos, sint_t endpos) const
{
	return PosEx(String, String.len(), startpos, endpos);
}

sint_t AString::PosNoCase(const char *iText, sint_t startpos, sint_t endpos) const
{
	return PosEx(iText, -1, startpos, endpos, false);
}

sint_t AString::PosNoCase(const AString& String, sint_t startpos, sint_t endpos) const
{
	return PosEx(String, String.len(), startpos, endpos, false);
}

sint_t AString::CharPos(char c, sint_t pos) const
{
	sint_t i;

	if (pos < 0)		   pos = 0;
	else if (pos > Length) pos = Length;

	for (i = pos; (i < Length) && (pText[i] != c); i++) ;
	if (i >= Length) i = -1;

	return i;
}

sint_t AString::LastCharPos(char c, sint_t pos) const
{
	sint_t i;

	if (pos < 0)		   pos = 0;
	else if (pos > Length) pos = Length;

	for (i = pos; (i >= 0) && (pText[i] != c); i--) ;

	return i;
}

sint_t AString::MatchBeginning(const char *iText) const
{
	sint_t l = strlen(iText);
	l = MIN(l, Length);
	sint_t n = strnicmp(iText, pText, l);

	return n;
}

sint_t AString::MatchBeginning(const AString& String) const
{
	sint_t l = MIN(String.Length, Length);
	sint_t n = strnicmp(String.pText, pText, l);

	return n;
}

sint_t AString::MatchLength(const char *iText) const
{
	sint_t i = 0, l = strlen(iText);

	for (i = 0; (i < Length) && (i < l) && (tolower(pText[i]) == tolower(iText[i])); i++) ;

	return i;
}

sint_t AString::MatchLength(const AString& String) const
{
	char *iText = String.pText;
	sint_t i = 0, l = strlen(iText);

	for (i = 0; (i < Length) && (i < l) && (tolower(pText[i]) == tolower(iText[i])); i++) ;

	return i;
}

sint_t AString::MatchLength(AString *pString) const
{
	char *iText = pString->pText;
	sint_t i = 0, l = strlen(iText);

	for (i = 0; (i < Length) && (i < l) && (tolower(pText[i]) == tolower(iText[i])); i++) ;

	return i;
}

AString AString::FindMinimumMatchLength(const AString& String) const
{
	AString *pString = (AString *)this;
	AString MaxString;
	bool bFirst = true;

	while (pString) {
		if (pString->MatchBeginning(String) == 0) {
			if (bFirst) {
				MaxString = pString;
				bFirst = false;
			} else {
				sint_t n = MaxString.MatchLength(pString);
				MaxString = (const char *)MaxString.Left(n);
			}
		}

		pString = pString->Next();
	}

	return MaxString;
}

AString AString::PathPart() const
{
	AString String;
	sint_t i;

	if (Length > 0) {
		for (i = Length - 1; (i > 0) && (pText[i] != '/') && (pText[i] != '\\') && !IsDriveSpecifier(i, pText[i]); i--) ;
		if (i > 0) {
			if (IsDriveSpecifier(i, pText[i])) i++;
			String = Left(i);
		} else {
			char slash = SlashChar();

			if (pText[0] == slash) String = slash;
		}
	}

	return String;
}

AString AString::FilePart() const
{
	AString String;
	sint_t i;

	if (Length > 0) {
		for (i = Length - 1; (i > 0) && (pText[i] != '/') && (pText[i] != '\\') && !IsDriveSpecifier(i, pText[i]); i--) ;
		if (i == 0) String = *this;
		else if (i < (Length - 1)) {
			if (IsDriveSpecifier(i, pText[i])) i++;
			if (i < (Length - 1)) {
				String = Right(Length - 1 - i);
			}
		}
	}

	return String;
}

AString AString::CatPath(const AString& Path1, const AString& Path2, char slash) const
{
	AString String = *this;
	int p;

	for (p = 0; Path1[p] && ((Path1[p] == '/') || (Path1[p] == '\\')); p++) ;
	if (Path1[p]) {
		String.AddSlash(slash);
		String += Path1.Mid(p);
	}

	for (p = 0; Path2[p] && ((Path2[p] == '/') || (Path2[p] == '\\')); p++) ;
	if (Path2[p]) {
		String.AddSlash(slash);
		String += Path2.Mid(p);
	}

	return String;
}

AString AString::Quotify(bool bEnd, bool bForce) const
{
	AString String;
	AStringUpdate updater(&String);

	if (!bForce) bForce = (strchr(pText, ' ') != NULL);

	if (bForce) updater.Update('\"');
	updater.Update(pText);
	if (bForce && bEnd) updater.Update('\"');

	updater.Flush();

	return String;
}

AString AString::DeQuotify(bool bBoth) const
{
	AString String;

	if		(IsQuoteChar(FirstChar()) && (LastChar() == FirstChar())) String = Mid(1, len() - 2);
	else if (!bBoth && IsQuoteChar(FirstChar()))					  String = Mid(1);
	else															  String = *this;

	return String;
}

static const char *EscapeSearch  = "\\\"'\n\r\t\b";
static const char *EscapeReplace = "\\\"'nrtb";

AString AString::Escapify() const
{
	AString String;
	AStringUpdate Updater(&String);

	sint_t i;
	const char *p;
	for (i = 0; i < Length; i++) {
		if ((p = strchr(EscapeSearch, pText[i])) != NULL) {
			Updater.Update("\\");
			Updater.Update(EscapeReplace[(int)(p - EscapeSearch)]);
		}
		else Updater.Update(pText[i]);
	}

	Updater.Flush();

	return String;
}

AString AString::DeEscapify() const
{
	AString String;
	AStringUpdate Updater(&String);

	sint_t i;
	const char *p;
	for (i = 0; i < Length; i++) {
		if (pText[i] == '\\') {
			i++;
			if (pText[i] && ((p = strchr(EscapeReplace, pText[i])) != NULL)) Updater.Update(EscapeSearch[(int)(p - EscapeReplace)]);
			else Updater.Update(deescape(pText, i));
		}
		else Updater.Update(pText[i]);
	}

	Updater.Flush();

	return String;
}

AString AString::URLify() const
{
	static const char *chars = " +%!#$&'()*+,-:;<=>?@[\"\\]^{}~";
	AString String, str;
	AStringUpdate updater(&String);
	sint_t i;

	for (i = 0; i < Length; i++) {
		if (strchr(chars, pText[i])) {
			str.Format("%%%02x", (uint_t)pText[i]);
			updater.Update(str);
		}
		else updater.Update(pText[i]);
	}

	updater.Flush();

	return String;
}

AString AString::DeURLify() const
{
	AString String, str;
	AStringUpdate updater(&String);
	sint_t i;

	for (i = 0; i < Length; i++) {
		if (pText[i] == '%') {
			char text[] = {pText[i + 1], pText[i + 2], 0};
			int  val;
			
			if (sscanf(text, "%x", &val) > 0) updater.Update((char)val);

			i += 2;
		}
		else if (pText[i] == '+') updater.Update(' ');
		else updater.Update(pText[i]);
	}

	updater.Flush();

	return String;
}

AString AString::HTMLify() const
{
	AString str;

	str = SearchAndReplace("&", "&amp;");
	str = str.SearchAndReplace("<", "&lt;");
	str = str.SearchAndReplace(">", "&gt;");

	return str;
}

AString AString::DeHTMLify() const
{
	AString str;

	str = SearchAndReplace("&lt;", "<");
	str = str.SearchAndReplace("&gt;", ">");
	str = str.SearchAndReplace("&amp;", "&");

	return str;
}

char AString::SlashChar() const
{
	char slash = '/';
	sint_t i;

	if (bSlashFromEnd) {
		for (i = Length - 1; (i > 0) && (pText[i] != '/') && (pText[i] != '\\'); i --) ;
	} else {
		for (i = 0; (i < Length) && (pText[i] != '/') && (pText[i] != '\\'); i++) ;
	}

	if ((i >= 0) && (pText[i] == '\\')) slash='\\';

	return slash;
}

AString& AString::AddSlash(char slash)
{
	if (!slash) slash = SlashChar();
	char last = LastChar();

	if ((Length > 0) && (last != slash) && (last != '/') && (last != '\\')) {
		operator += (slash);
	}

	return *this;
}

AString& AString::Replace(const char *pSearchChars, const char *pReplaceChars, bool bIncludeZero)
{
	sint_t   i, j, l1 = strlen(pSearchChars), l2 = strlen(pReplaceChars);
	char  lookup[256];
	uint8_t c;

	for (i = 0; i < 256; i++) lookup[i] = i;

	for (i = 0; i < l1; i++) {
		c = (uint8_t)pSearchChars[i];
		if (i < l2) lookup[c] = pReplaceChars[i];
		else		lookup[c] = 0;
	}

	if (pText == (char *)pDefaultText) pText = CREATE_TEXT(Length + 1);

	for (i = j = 0; (i < Length); i++) {
		c = (uint8_t)pText[i];
		if (bIncludeZero || (lookup[c] != 0)) pText[j++] = lookup[c];
	}
	pText[j] = 0;
	Length   = j;

	return *this;
}

AString AString::Copies(sint_t n) const
{
	AString String;
	sint_t l = n * Length;

	if (l > 0) {
		if ((String.pText = CREATE_TEXT(l + 1)) != NULL) {
			if (Length == 1) {
				memset(String.pText, pText[0], l);
			}
			else {
				sint_t p;

				memcpy(String.pText, pText, Length);
				for (p = Length; p < l;) {
					sint_t nchars = MIN(p, l - p);

					if (!nchars) break;

					memcpy(String.pText + p, String.pText, nchars);

					p += nchars;
				}
			}

			String.Length = l;
			String.pText[String.Length] = 0;
		}
	}

	return String;
}

AString& AString::AppendCopies(const char *pString, sint_t n)
{
	{
		AStringUpdate updater(this);

		sint_t i;
		for (i = 0; i < n; i++) updater += pString;
	}

	return *this;
}

void AString::UpperCase()
{
	sint_t i;

	for (i = 0; i < Length; i++) pText[i] = toupper(pText[i]);
}

void AString::LowerCase()
{
	sint_t i;

	for (i = 0; i < Length; i++) pText[i] = tolower(pText[i]);
}

void AString::ApplyCase(const char *iText)
{
	uint_t i, l = MIN(strlen(iText), (uint_t)Length);
	for (i = 0; i < l; i++) {
		if (islower(iText[i])) pText[i] = tolower(pText[i]);
		else				   pText[i] = toupper(pText[i]);
	}
}

void AString::ApplyCase(const AString& String)
{
	sint_t i, l = MIN(String.len(), Length);
	const char *iText = String.str();
	for (i = 0; i < l; i++) {
		if (islower(iText[i])) pText[i] = tolower(pText[i]);
		else				   pText[i] = toupper(pText[i]);
	}
}

AString AString::ToUpper() const
{
	AString String = *this;

	String.UpperCase();

	return String;
}

AString AString::ToLower() const
{
	AString String = *this;

	String.LowerCase();

	return String;
}

AString AString::RemoveWhiteSpace() const
{
	AString String;

	sint_t i, j;
	for (i = 0; (i < Length) && IsWhiteSpace(pText[i]); i++) ;
	for (j = Length; (j > i) && IsWhiteSpace(pText[j - 1]); j--) ;

	if (j > i) String.Create(pText + i, j - i);

	return String;
}

sint_t AString::ReadCh(FILE *fp)
{
	int c;

	Delete();

	{
		AStringUpdate updater(this);
		while (((c = fgetc(fp)) != EOF) && (c != 0)) {
			if (c != '\r') updater += c;
		}
	}

	return ((c == EOF) && (Length == 0)) ? -1 : Length;
}

sint_t AString::ReadLn(FILE *fp)
{
	int c;

	Delete();

	{
		AStringUpdate updater(this);
		while (((c = fgetc(fp)) != EOF) && (c != '\n')) {
			if ((c != 0) && (c != '\r')) updater += c;
		}
	}

	return ((c == EOF) && (Length == 0)) ? -1 : Length;
}

bool AString::WriteCh(FILE *fp) const
{
	bool ok;

	ok  = (fprintf(fp, "%s", pText) >= 0);
	if (ok) ok = (fputc(0, fp) != EOF);

	return ok;
}

bool AString::WriteLn(FILE *fp) const
{
	bool ok;

	ok = (fprintf(fp, "%s\n", pText) >= 0);

	return ok;
}

sint_t AString::ReadCh(AStdData *fp)
{
	sint_t c;

	Delete();

	{
		AStringUpdate updater(this);
		while (((c = fp->getc()) != EOF) && (c != 0)) {
			updater += c;
		}
	}

	return ((c == EOF) && (Length == 0)) ? -1 : Length;
}

sint_t AString::ReadLn(AStdData *fp)
{
	sint_t c;

	Delete();

	{
		AStringUpdate updater(this);
		while (((c = fp->getc()) != EOF) && (c != '\n')) {
			if ((c != 0) && (c != '\r')) updater += c;
		}
	}

	return ((c == EOF) && (Length == 0)) ? -1 : Length;
}

bool AString::WriteCh(AStdData *fp) const
{
	bool ok;

	ok  = (fp->printf("%s", pText) >= 0);
	if (ok) ok = (fp->putc(0) != EOF);

	return ok;
}

bool AString::WriteLn(AStdData *fp) const
{
	bool ok;

	ok = (fp->printf("%s\n", pText) >= 0);

	return ok;
}

sint_t AString::ReadLines(FILE *fp, AString *pInitialString)
{
	AString Line;
	sint_t Lines = 0;
	sint_t Level = 0;

	if (pInitialString) Line = pInitialString;
	else				Line.ReadLn(fp);

	do {
		Line = Line.RemoveWhiteSpace();

		if (Line.Pos("{") >= 0) Level++;
		if (Line.Pos("}") >= 0) {
			if ((--Level) == 0) break;
		}

		if (Line.len() > 0) {
			Line += "\n";
			Append(Line);
			Lines++;
		}
	} while (Line.ReadLn(fp) >= 0);

	return Lines;
}

sint_t AString::FindMarker(const char *pText, const char *pMarker, sint_t TextLen, sint_t MarkerLen, uint_t flags, sint_t p)
{
	int quote = 0;

	while (p < TextLen) {
		if (IsEscapeCharFlags(pText[p])) {
			p++;
			deescape(pText, p);
		}
		else if (quote) {if (pText[p++] == quote) quote = 0;}
		else if (strncmp(pText + p, pMarker, MarkerLen) == 0) break;
		else if (IsQuoteCharFlags(pText[p])) quote = pText[p++];
		else p++;
	}

	return p;
}

sint_t AString::PositionOfLine(sint_t n, const char *pMarker, uint_t flags) const
{
	sint_t l = strlen(pMarker);
	sint_t p = 0;

	while ((n > 0) && (p < Length)) {
		p = FindMarker(pText, pMarker, Length, l, flags, p);
		if (p < Length) p += l;
		n--;
	}

	return p;
}

sint_t AString::CountLines(const char *pMarker, uint_t flags) const
{
	sint_t l = strlen(pMarker);
	sint_t p = 0, n = 0;

	while (p < Length) {
		n++;
		p = FindMarker(pText, pMarker, Length, l, flags, p) + l;
	}

	return n;
}

AString AString::Line(sint_t n, const char *pMarker, uint_t flags) const
{
	AString String;
	sint_t p, p1;

	p = PositionOfLine(n, pMarker, flags);
	if (p < Length) {
		p1 = FindMarker(pText, pMarker, Length, strlen(pMarker), flags, p);
		String = Mid(p, p1 - p);
	}

	return String;
}

AString AString::CutLine(sint_t n, const char *pMarker, uint_t flags)
{
	AString String;
	sint_t p, p1;

	p = PositionOfLine(n, pMarker, flags);
	if (p < Length) {
		sint_t l = strlen(pMarker);
		p1 = FindMarker(pText, pMarker, Length, l, flags, p);
		String = Mid(p, p1 - p);
		*this = Left(p) + Mid(p1 + l);
	}

	return String;
}

void AString::PasteLine(sint_t n, const AString& String, const char *pMarker, uint_t flags)
{
	sint_t p;
	
	while (CountLines(pMarker, flags) < n) {
		*this += pMarker;
	}

	if (CountLines(pMarker, flags) == n) {
		if (Right(strlen(pMarker)) != pMarker)  *this += pMarker;
	}

	p  = PositionOfLine(n, pMarker, flags);

	*this = Left(p) + String + pMarker + Mid(p);
}

AString AString::Remove(const char *pChars) const
{
	AString String;
	sint_t i;

	for (i = 0; (i < Length) && ((Length - 1 - i) > i) && (pText[i] == pText[Length - 1 - i]) && strchr(pChars, pText[i]); i++) ;

	String.Create(pText + i, Length - 2 * i);

	return String;
}

AString AString::Abbreviate(sint_t n) const
{
	AString String;

	if (n >= Length) String = *this;
	else {
		if (n > 3) {
			String.Create(pText, n - 3);
			n = 3;
		}

		if		(n == 3) String += "...";
		else if (n == 2) String += "..";
		else			 String += ".";
	}

	return String;
}

AString AString::Encode() const
{
	return *this;
}

AString AString::Decode() const
{
	return *this;
}

AString AString::ExtractColumns(const char *pSeparator) const
{
	AString String;
	char *p;
	sint_t l = strlen(pSeparator);

	p = pText;
	while (p) {
		char *p1;

		if ((p1 = strstr(p, pSeparator)) != NULL) {
			String += "'";
			String.Append(p, p1 - p);
			String += "' ";

			p = p1 + l;
		}
		else break;
	}

	return String;
}

AString AString::GetToken(const char *pSeparators, sint_t& pos) const
{
	while ((pos < Length) && strchr(pSeparators, pText[pos])) pos++;

	uint_t pos0 = pos;

	while ((pos < Length) && !strchr(pSeparators, pText[pos])) pos++;

	return Mid(pos0, pos - pos0);
}

void AString::SplitColumns(const char *pSeparator, AString *pPreString, AString *pPostString) const
{
	char *p;
	sint_t l = strlen(pSeparator);

	if ((p = strstr(pText, pSeparator)) != NULL) {
		if (pPreString)  pPreString->Create(pText, p - pText);
		if (pPostString) pPostString->Create(p + l);
	}
	else {
		if (pPreString)  pPreString->Create(pText);
		if (pPostString) pPostString->Delete();
	}
}

sint_t AString::ExtractColumns(const char *pSeparator, AString *pStrings, sint_t MaxStrings) const
{
	char *p;
	sint_t n, l = strlen(pSeparator);

	p = pText;
	for (n = 0; p && (n < MaxStrings);) {
		char *p1;

		if ((p1 = strstr(p, pSeparator)) != NULL) {
			pStrings[n++].Create(p, p1 - p);

			p = p1 + l;
		}
		else break;
	}

	return n;
}

AString AString::ExtractRegion(char Startchar, char Endchar, const char *pReplace)
{
	AString String;
	sint_t i, i0, level = 0;

	for (i = 0; (i < Length) && (pText[i] != Startchar); i++) ;

	if (i < Length) {
		for (i0 = i; (i < Length); i++) {
			if (pText[i] == Startchar) level++;
			else if (pText[i] == Endchar) {
				if ((--level) == 0) {
					i++;
					String.Create(pText + i0 + 1, i - i0 - 2);
					
					AString String1 = Left(i0);
					AString String2 = Mid(i);

					Create(String1 + pReplace + String2);
					break;
				}
			}
		}
	}

	return String;
}

AString AString::StripBrackets() const
{
	AString String;
	sint_t i, j, level = 0, mlevel;

	for (i = 0; (i < (Length / 2)) && (pText[i] == '(') && (pText[Length - 1 - i] == ')'); i++) level++;
	
	mlevel = level;
	for (j = i; (j < (Length - i)); j++) {
		if ((pText[j] == '\\') && pText[j + 1]) j++;
		else if (pText[j] == '(') level++;
		else if (pText[j] == ')') level--;

		mlevel = MIN(mlevel, level);
	}

	if (mlevel >= 0) String.Create(pText + mlevel, Length - 2 * mlevel);
	else			 String.Create(pText);

	return String;
}

sint_t AString::StartOfLine(sint_t p, const char *pMarker) const
{
	sint_t l = strlen(pMarker);

	while ((p >= l) && (strncmp(pText + p - l, pMarker, l) != 0)) p--;

	return p;
}

sint_t AString::EndOfLine(sint_t p, const char *pMarker) const
{
	sint_t l = strlen(pMarker);

	while ((p < Length) && (strncmp(pText + p, pMarker, l) != 0)) p++;

	return p;
}

AString& AString::Insert(sint_t Pos, const char *pString)
{
	sint_t  l = strlen(pString);
	char *pStr;

	Pos = LIMIT(Pos, 0, Length);
	
	if ((pStr = new char [Length + l + 1]) != NULL) {
		if (Pos > 0) 	  memcpy(pStr, pText, Pos);
		if (l   > 0) 	  memcpy(pStr + Pos, pString, l);
		if (Pos < Length) memcpy(pStr + Pos + l, pText + Pos, Length - Pos);
		pStr[Length + l] = 0;

		Create(pStr);
	}

	return *this;
}

AString& AString::DeleteChars(sint_t Pos, sint_t nchars)
{
	Pos    = LIMIT(Pos, 0, Length);
	nchars = LIMIT(nchars, 0, Length - Pos);

	char *pStr;
	
	if ((pStr = new char [Length - nchars + 1]) != NULL) {
		if (Pos > 0) 	             memcpy(pStr, pText, Pos);
		if ((Pos + nchars) < Length) memcpy(pStr + Pos, pText + Pos + nchars, Length - Pos - nchars);
		pStr[Length - nchars] = 0;

		Create(pStr);
	}

	return *this;
}

void AString::TimeString()
{
	TimeString(time(NULL));
}

void AString::TimeString(time_t seconds)
{
	struct tm *newtime;
	time_t clock;

	clock	= seconds;
	newtime = localtime(&clock);

	Create(asctime(newtime));
	Replace("\n", "");
}

const char *AString::FindTokenForwards(const char *p, char token, const char *tokens, uint_t quotes)
{
	const char *p1 = p;
	bool  found = false;

	while (p1[0]) {
		const char *pp;

		if		((p1[0] == '\\') && p1[1]) p1 += 2;
		else if (p1[0] == token) {found = true; break;}
		else if (tokens && ((pp = strchr(tokens, p1[0])) != NULL) && !((uint_t)(pp - tokens) & 1) && pp[1]) {
			const char *p2;
			
			p1++;
			if ((p2 = FindTokenForwards(p1, pp[1], tokens, quotes)) > p1) p1 = p2;
			else break;
		}
		else if (((quotes & FIND_DOUBLE_QUOTE) && (p1[0] == '\"')) || 
				 ((quotes & FIND_SINGLE_QUOTE) && (p1[0] == '\''))) {
			const char *p2;
			char newtoken = p1[0];

			p1++;
			if ((p2 = FindTokenForwards(p1, newtoken)) > p1) p1 = p2;
			else break;
		}
		else p1++;

	}

	return found ? p1 + 1 : p;
}

const char *AString::FindTokenBackwards(const char *p, const char *p0, char token, const char *tokens, uint_t quotes)
{
	const char *p1 = p;
	bool  found = false;

	while (p1 > p0) {
		const char *pp;

		p1--;

		if		((p1[-1] == '\\') && p1[0]) p1--;
		else if (p1[0]  == token) {found = true; break;}
		else if (tokens && ((pp = strchr(tokens, p1[0])) != NULL) && ((uint_t)(pp - tokens) & 1) && pp[-1]) {
			const char *p2;
			
			if ((p2 = FindTokenBackwards(p1, p0, pp[-1], tokens, quotes)) < p1) p1 = p2;
			else break;
		}
		else if (((quotes & FIND_DOUBLE_QUOTE) && (p1[0] == '\"')) || 
				 ((quotes & FIND_SINGLE_QUOTE) && (p1[0] == '\''))) {
			const char *p2;
			char newtoken = p1[0];

			if ((p2 = FindTokenBackwards(p1, p0, newtoken)) < p1) p1 = p2;
			else break;
		}

	}

	return found ? p1 : p;
}

sint_t AString::ForwardSexp(sint_t pos, const char *sexp, bool bQuotes) const
{
	if ((pos >= 0) && (pos < Length)) {
		const char *p1 = pText + pos, *p2;

		if ((p2 = FindTokenForwards(pText + pos, 0, sexp, bQuotes ? FIND_BOTH_QUOTES : FIND_DOUBLE_QUOTE)) > p1) pos = p2 - pText;
	}

	return pos;
}

sint_t AString::BackwardSexp(sint_t pos, const char *sexp, bool bQuotes) const
{
	if ((pos >= 0) && (pos < Length)) {
		const char *p1 = pText + pos, *p2;

		if ((p2 = FindTokenBackwards(pText + pos, pText, 0, sexp, bQuotes ? FIND_BOTH_QUOTES : FIND_DOUBLE_QUOTE)) > p1) pos = p2 - pText;
	}

	return pos;
}

sint_t AString::AlphaCompareCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext)
{
	const AString *pString1 = (const AString *)pNode1;
	const AString *pString2 = (const AString *)pNode2;
	sint_t cmp = ::CompareCase(*pString1, *pString2);

	if (pContext && *(bool *)pContext) cmp = -cmp;

	return cmp;
}

sint_t AString::AlphaCompareNoCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext)
{
	const AString *pString1 = (const AString *)pNode1;
	const AString *pString2 = (const AString *)pNode2;
	sint_t cmp = ::CompareNoCase(*pString1, *pString2);

	if (pContext && *(bool *)pContext) cmp = -cmp;

	return cmp;
}

AString AString::EngFormat(double value, uint_t mindec, uint_t maxdec, double scale)
{
	static const char *Sufficies = "fpnum kMGT";
	AString str, format, suffix;
	double  sign = (value < 0.0) ? -1.0 : 1.0;

	value *= sign;

	if (value > 0.0) {
		sint_t factor;

		factor = (sint_t)floor(log10(value) / log10(scale));
		factor = MAX(factor, -5);
		factor = MIN(factor,  4);

		value *= pow(scale, -(double)factor);

		if ((value < 1.0)    && (factor > -5)) {value *= scale; factor--;}
		if ((value >= scale) && (factor <  4)) {value /= scale; factor++;}

		if (factor != 0) suffix = Sufficies[5 + factor];
	}

	format.Format("%%0.%ulf", maxdec);
	value *= sign;
	str.Format(format, value);

	if (maxdec > mindec) {
		int p;
		if ((p = str.LastPos(".")) >= 0) {
			uint_t dec;

			p++;
			for (dec = maxdec - 1; (dec > mindec) && (str[p + dec] == '0'); dec--) ;
			if (!dec) p--;
			str = str.Left(p + dec);
		}
	}

	return str + suffix;
}

AString	AString::Base64Encode() const
{
	return ::Base64Encode((const uint8_t *)pText, Length);
}

sint_t AString::Base64DecodeLength() const
{
	assert(Base64Count == 64);

	if (!Base64LookupInitialised) {
		Base64LookupInitialised = true;

		memset(Base64Lookup, 0xff, sizeof(Base64Lookup));

		uint_t i;
		for (i = 0; i < Base64Count; i++) {
			Base64Lookup[(uint8_t)Base64Chars[i]] = i;
		}
	}

	sint_t i, p;
	if ((p = Pos("=")) < 0) p = Length;
	for (i = 0; i < p; i++) {
		if (Base64Lookup[(uint8_t)pText[i]] == 0xff) break;
	}

	return (i == p) ? (p * 3) / 4 : -1;
}

sint_t AString::Base64Decode(uint8_t *buffer, uint_t maxbytes) const
{
	sint_t n = Base64DecodeLength();
	if (n > 0) {
		uint32_t data;
		
		n = MIN(n, (sint_t)maxbytes);

		sint_t i, j = 0;
		for (i = 0; ((j + 3) <= n); i += 4) {
			data  = (uint32_t)Base64Lookup[(uint8_t)pText[i]]     << 18;
			data |= (uint32_t)Base64Lookup[(uint8_t)pText[i + 1]] << 12;
			data |= (uint32_t)Base64Lookup[(uint8_t)pText[i + 2]] << 6;
			data |= (uint32_t)Base64Lookup[(uint8_t)pText[i + 3]];

			buffer[j++] = (uint8_t)(data >> 16);
			buffer[j++] = (uint8_t)(data >> 8);
			buffer[j++] = (uint8_t)data;
		}

		switch (n - j) {
			case 2:
				data  = (uint32_t)Base64Lookup[(uint8_t)pText[i]]     << 18;
				data |= (uint32_t)Base64Lookup[(uint8_t)pText[i + 1]] << 12;
				data |= (uint32_t)Base64Lookup[(uint8_t)pText[i + 2]] << 6;
				buffer[j++] = (uint8_t)(data >> 16);
				buffer[j++] = (uint8_t)(data >> 8);
				break;

			case 1:
				data  = (uint32_t)Base64Lookup[(uint8_t)pText[i]]     << 18;
				data |= (uint32_t)Base64Lookup[(uint8_t)pText[i + 1]] << 12;
				buffer[j++] = (uint8_t)(data >> 16);
				break;

			case 0:
				break;

			default:
				assert(false);
				break;
		}
	}

	return n;
}

AString AString::Base64Decode() const
{
	AString res;

	sint_t l = Base64DecodeLength();

	if (l > 0) {
		char *buffer;

		if ((buffer = CREATE_TEXT(l + 1)) != NULL) {
			Base64Decode((uint8_t *)buffer, l);
			res.Create(buffer, l);

			DELETE_TEXT(buffer);
		}
	}

	return res;
}

AString Base64Encode(const uint8_t *ptr, uint_t bytes)
{
	AString res;
	AStringUpdate updater(&res);
	uint_t i, n = bytes - (bytes % 3);
	uint32_t data;

	assert(Base64Count == 64);

	for (i = 0; i < n; i += 3) {
		data = ((uint32_t)ptr[i] << 16) | ((uint32_t)ptr[i + 1] << 8) | (uint32_t)ptr[i + 2];
		updater.Update(Base64Chars[(data >> 18) & 0x3f]);
		updater.Update(Base64Chars[(data >> 12) & 0x3f]);
		updater.Update(Base64Chars[(data >> 6)  & 0x3f]);
		updater.Update(Base64Chars[data         & 0x3f]);
	}
	
	switch (bytes - i) {
		case 2:
			data = ((uint32_t)ptr[i] << 16) | ((uint32_t)ptr[i + 1] << 8);
			updater.Update(Base64Chars[(data >> 18) & 0x3f]);
			updater.Update(Base64Chars[(data >> 12) & 0x3f]);
			updater.Update(Base64Chars[(data >> 6)  & 0x3f]);
			updater.Update('=');
			break;
			
		case 1:
			data = (uint32_t)ptr[i] << 16;
			updater.Update(Base64Chars[(data >> 18) & 0x3f]);
			updater.Update(Base64Chars[(data >> 12) & 0x3f]);
			updater.Update('=');
			updater.Update('=');
			break;

		case 0:
			break;

		default:
			assert(false);
			break;
	}
	
	updater.Flush();

	return res;
}

sint_t AString::GetFieldPos(const char *pStartMarker, const char *pEndMarker, sint_t start, sint_t *len) const
{
	sint_t pos = -1, p, p1;

	if ((p = Pos(pStartMarker, start)) >= start) {
		sint_t l = strlen(pStartMarker);

		if ((p1 = Pos(pEndMarker, p + l)) >= 0) {
			pos = p + l;
			if (len) *len = p1 - pos;
		}
	}

	return pos;
}

AString AString::GetField(const char *pStartMarker, const char *pEndMarker, sint_t start, sint_t *len) const
{
	AString res;
	sint_t pos, len1;

	if ((pos = GetFieldPos(pStartMarker, pEndMarker, start, &len1)) >= 0) {
		res = Mid(pos, len1);
		if (len) *len = len1;
	}
	else if (len) *len = -1;

	return res;
}

sint_t AString::GetField(const char *pStartMarker, const char *pEndMarker, AString& str, sint_t start, sint_t *len) const
{
	sint_t pos, len1;

	if ((pos = GetFieldPos(pStartMarker, pEndMarker, start, &len1)) >= 0) {
		str = Mid(pos, len1);
		if (len) *len = len1;
	}
	else {
		if (len) *len = -1;
		str.Delete();
	}

	return pos;
}

bool AString::SetField(const char *pStartMarker, const char *pEndMarker, const char *pString, sint_t start)
{
	sint_t pos, len;
	bool res = false;

	if ((pos = GetFieldPos(pStartMarker, pEndMarker, start, &len)) >= 0) {
		*this = Left(pos) + AString(pString) + Mid(pos + len);
		res = true;
	}
	else *this += AString(pStartMarker) + AString(pString) + AString(pEndMarker);

	return res;
}

sint_t AString::GetFieldNumber(const char *pMarker, sint_t n, AString& str, sint_t start) const
{
	sint_t p = start, p1 = start;
	int  i, l = strlen(pMarker);

	for (i = 0; i <= n; i++) {
		if (i > 0) p = p1 + l;

		if ((p1 = Pos(pMarker, p)) < 0) p1 = Length;
	}

	if (p1 > p) {
		str = Mid(p, p1 - p);
	}
	else {
		str.Delete();
		p = -1;
	}

	return p;
}

AString AString::AllUpperCase() const
{
	AString res;
	AStringUpdate updater(&res);
	sint_t i;

	for (i = 0; pText[i]; i++) {
		if (pText[i] != tolower(pText[i])) updater.Update('_');
		updater.Update(toupper(pText[i]));
	}
	
	updater.Flush();

	return res;
}

AString AString::InitialCapitalCase() const
{
	AString res;
	AStringUpdate updater(&res);
	sint_t i;
	bool forceupper = true;

	for (i = 0; pText[i]; i++) {
		if (pText[i] == '_') forceupper = true;
		else if (forceupper) {
			updater.Update(toupper(pText[i]));
			forceupper = false;
		}
		else updater.Update(pText[i]);
	}
	
	updater.Flush();

	return res;
}

AString AString::MD5Hash() const
{
	AString res;
	AStringUpdate updater(&res);
	MD5_CONTEXT context;
	uint8_t val[16];
	uint_t i;

	memset(&context, 0, sizeof(context));

	MD5Init(&context);
	MD5Update(&context, (const unsigned char *)pText, Length);
	MD5Final(val, &context);

	memset(&context, 0, sizeof(context));

	for (i = 0; i < NUMBEROF(val); i++) {
		updater.Update(AString::Formatify("%02x", (uint_t)val[i]));
	}

	updater.Flush();

	return res;
}

uint_t AString::Count(const char *pString) const
{
	uint_t n = 0, l = strlen(pString);
	int  p = 0;

	while ((p = Pos(pString, p)) >= 0) {
		n++;
		p += l;
	}

	return n;
}

uint_t AString::CountNoCase(const char *pString) const
{
	uint_t n = 0, l = strlen(pString);
	int  p = 0;

	while ((p = PosNoCase(pString, p)) >= 0) {
		n++;
		p += l;
	}

	return n;
}

sint_t AString::FindClosing(char terminator, sint_t p, const char *nestedchars) const
{
	if (p >= (sint_t)strlen(pText)) return -1;

	uint_t l = nestedchars ? strlen(nestedchars) : 0, n = l / 2;
	uint_t *counters = n ? new uint_t[n] : NULL;
	uint_t total = 0;

	if (counters) memset(counters, 0, n * sizeof(*counters));

	for (;pText[p] && (total || (pText[p] != terminator));p++) {
		const char *np;

		if (nestedchars && ((np = strchr(nestedchars, pText[p])) != NULL)) {
			uint_t a = np - nestedchars, b = a / 2;

			if (a & 1) {
				if (counters[b]) {
					counters[b]--;
					total--;
				}
			}
			else {
				counters[b]++;
				total++;
			}
		}
	}

	if (counters) delete[] counters;

	if (!pText[p]) p = -1;

	return p;
}

/*-------------------------------------------------------------------------------------------*/

AStringUpdate::AStringUpdate(AString *iString, sint_t iBufferSize) : pString(NULL),
																	 pBuffer(NULL),
																	 BufferPos(0),
																	 BufferSize(0)
{
	Attach(iString, iBufferSize);
}

AStringUpdate::~AStringUpdate()
{
	Flush();

	if (pBuffer) {
		delete[] pBuffer;
		pBuffer = NULL;
	}
}

bool AStringUpdate::Attach(AString *iString, sint_t iBufferSize)
{
	Flush();

	if (pBuffer) {
		delete[] pBuffer;
		pBuffer = NULL;
	}
	BufferSize = BufferPos = 0;
	pString = NULL;

	if (iString) {
		if ((pBuffer = new char[iBufferSize]) != NULL) {
			pString    = iString;
			BufferSize = iBufferSize;
		}
	}

	return (pBuffer != NULL);
}

void AStringUpdate::Update(char c)
{
	if (pString && pBuffer) {
		pBuffer[BufferPos++] = c;

		if (BufferPos == (BufferSize - 1)) Flush();
	}
}

void AStringUpdate::Update(const char *p)
{
	if (pString && pBuffer) {
		sint_t l = strlen(p);

		if ((BufferPos + l) < (BufferSize - 1)) {
			memcpy(pBuffer + BufferPos, p, l);
			BufferPos += l;
		}
		else if (l < (BufferSize - 1)) {
			Flush();
			memcpy(pBuffer + BufferPos, p, l);
			BufferPos += l;
		}
		else {
			Flush();
			pString->Append(p);
		}
	}
}

void AStringUpdate::Update(const AString& String)
{
	Flush();

	if (pString) pString->Append(String);
}

void AStringUpdate::Flush()
{
	if (pString && pBuffer) {
		pBuffer[BufferPos] = 0;
		pString->Append(pBuffer);
	}

	BufferPos = 0;
}

sint_t AStringUpdate::TotalLength() const
{
	sint_t l = 0;

	if (pString && pBuffer) l = pString->len() + BufferPos;

	return l;
}

/*----------------------------------------------------------------------------------------------------*/

NODETYPE_IMPLEMENT(AKeyValuePair);

AKeyValuePair::AKeyValuePair() : AListNode()
{
}

AKeyValuePair::AKeyValuePair(const AKeyValuePair& object) : AListNode(),
															Key(object.Key),
															Value(object.Value)
{
}

AKeyValuePair::~AKeyValuePair()
{
}

sint_t AKeyValuePair::AlphaCompareKeyNoCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext)
{
	const AKeyValuePair *pair1 = AKeyValuePair::Cast(pNode1);
	const AKeyValuePair *pair2 = AKeyValuePair::Cast(pNode2);
	const AString *str1 = pair1 ? &pair1->Key : AString::Cast(pNode1);
	const AString *str2 = pair2 ? &pair2->Key : AString::Cast(pNode2);
	sint_t cmp;

	if (!str1 || !str2) return -1;

	cmp = CompareNoCase(*str1, *str2);

	if (pContext && *(bool *)pContext) cmp = -cmp;

	return cmp;
}

sint_t AKeyValuePair::AlphaCompareKeyCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext)
{
	const AKeyValuePair *pair1 = AKeyValuePair::Cast(pNode1);
	const AKeyValuePair *pair2 = AKeyValuePair::Cast(pNode2);
	const AString *str1 = pair1 ? &pair1->Key : AString::Cast(pNode1);
	const AString *str2 = pair2 ? &pair2->Key : AString::Cast(pNode2);
	sint_t cmp;

	if (!str1 || !str2) return -1;

	cmp = CompareCase(*str1, *str2);

	if (pContext && *(bool *)pContext) cmp = -cmp;

	return cmp;
}

sint_t AKeyValuePair::AlphaCompareValueNoCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext)
{
	const AKeyValuePair *pair1 = AKeyValuePair::Cast(pNode1);
	const AKeyValuePair *pair2 = AKeyValuePair::Cast(pNode2);
	const AString *str1 = pair1 ? &pair1->Value : AString::Cast(pNode1);
	const AString *str2 = pair2 ? &pair2->Value : AString::Cast(pNode2);
	sint_t cmp;

	if (!str1 || !str2) return -1;

	cmp = CompareNoCase(*str1, *str2);

	if (pContext && *(bool *)pContext) cmp = -cmp;

	return cmp;
}

sint_t AKeyValuePair::AlphaCompareValueCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext)
{
	const AKeyValuePair *pair1 = AKeyValuePair::Cast(pNode1);
	const AKeyValuePair *pair2 = AKeyValuePair::Cast(pNode2);
	const AString *str1 = pair1 ? &pair1->Value : AString::Cast(pNode1);
	const AString *str2 = pair2 ? &pair2->Value : AString::Cast(pNode2);
	sint_t cmp;

	if (!str1 || !str2) return -1;

	cmp = CompareCase(*str1, *str2);

	if (pContext && *(bool *)pContext) cmp = -cmp;

	return cmp;
}

/*----------------------------------------------------------------------------------------------------*/

NODETYPE_IMPLEMENT(AKeyObjectPair);

AKeyObjectPair::AKeyObjectPair(bool autodelete) : AListNode(),
												  pObject(NULL),
												  bAutoDelete(autodelete)
{
}

AKeyObjectPair::AKeyObjectPair(const AKeyObjectPair& object, bool autodelete) : AListNode(),
																				Key(object.Key),
																				pObject(object.pObject ? object.pObject->Duplicate() : NULL),
																				bAutoDelete(autodelete)
{
}

AKeyObjectPair::~AKeyObjectPair()
{
	if (pObject && bAutoDelete) delete pObject;
}

sint_t AKeyObjectPair::AlphaCompareKeyNoCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext)
{
	const AKeyObjectPair *pair1 = AKeyObjectPair::Cast(pNode1);
	const AKeyObjectPair *pair2 = AKeyObjectPair::Cast(pNode2);
	const AString *str1 = pair1 ? &pair1->Key : AString::Cast(pNode1);
	const AString *str2 = pair2 ? &pair2->Key : AString::Cast(pNode2);
	sint_t cmp;

	if (!str1 || !str2) return -1;

	cmp = CompareNoCase(*str1, *str2);

	if (pContext && *(bool *)pContext) cmp = -cmp;

	return cmp;
}

sint_t AKeyObjectPair::AlphaCompareKeyCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext)
{
	const AKeyObjectPair *pair1 = AKeyObjectPair::Cast(pNode1);
	const AKeyObjectPair *pair2 = AKeyObjectPair::Cast(pNode2);
	const AString *str1 = pair1 ? &pair1->Key : AString::Cast(pNode1);
	const AString *str2 = pair2 ? &pair2->Key : AString::Cast(pNode2);
	sint_t cmp;

	if (!str1 || !str2) return -1;

	cmp = CompareCase(*str1, *str2);

	if (pContext && *(bool *)pContext) cmp = -cmp;

	return cmp;
}

/*----------------------------------------------------------------------------------------------------*/

NODETYPE_IMPLEMENT(AStringPair);

AStringPair::AStringPair() : AListNode()
{
}

AStringPair::AStringPair(const AStringPair& object) : AListNode(),
													  String1(object.String1),
													  String2(object.String2)
{
}

AStringPair::~AStringPair()
{
}

/*----------------------------------------------------------------------------------------------------*/

NODETYPE_IMPLEMENT(AStringPairWithInt);

AStringPairWithInt::AStringPairWithInt() : AListNode(),
										   Integer(0)
{
}

AStringPairWithInt::AStringPairWithInt(const AStringPairWithInt& object) : AListNode(),
																		   Integer(object.Integer),
																		   String1(object.String1),
																		   String2(object.String2)
{
}

AStringPairWithInt::~AStringPairWithInt()
{
}

/*----------------------------------------------------------------------------------------------------*/

NODETYPE_IMPLEMENT(AStringTriplet);

AStringTriplet::AStringTriplet() : AListNode()
{
}

AStringTriplet::AStringTriplet(const AStringTriplet& object) : AListNode(),
															   String1(object.String1),
															   String2(object.String2),
															   String3(object.String3)
{
}

AStringTriplet::~AStringTriplet()
{
}
