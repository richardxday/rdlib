
#ifndef __STRING_SUPPORT__
#define __STRING_SUPPORT__

#include <time.h>

#include <string>

#include "ListNode.h"
#include "StdData.h"
#include "EvalNumber.h"

#define COMP_OPERATOR(op)                                                                                                       \
friend sint_t operator op(const AString& Object1, const AString& Object2) {return (strcmp(Object1.pText, Object2.pText) op 0);} \
friend sint_t operator op(const char *iText, const AString& Object2) {return (strcmp(iText, Object2.pText) op 0);}              \
friend sint_t operator op(const AString& Object1, const char *iText) {return (strcmp(Object1.pText, iText) op 0);}

#define COMP_CASE_OPERATOR(a, func)                                                                                     \
friend sint_t Compare##a(const AString& Object1, const AString& Object2) {return func(Object1.pText, Object2.pText);}   \
friend sint_t Compare##a(const char *iText, const AString& Object2) {return func(iText, Object2.pText);}                \
friend sint_t Compare##a(const AString& Object, const char *iText) {return func(Object.pText, iText);}                  \
sint_t Compare##a(const AString& Object) const {return func(pText, Object.pText);}                                      \
sint_t Compare##a(const char *iText)     const {return func(pText, iText);}

#define COMP_CASE_OPERATOR_N(a, func)                                                                                                   \
friend sint_t Compare##a##N(const AString& Object1, const AString& Object2, sint_t n) {return func(Object1.pText, Object2.pText, n);}   \
friend sint_t Compare##a##N(const char *iText, const AString& Object2, sint_t n) {return func(iText, Object2.pText, n);}                \
friend sint_t Compare##a##N(const AString& Object1, const char *iText, sint_t n) {return func(Object1.pText, iText, n);}                \
sint_t Compare##a##N(const AString& Object, sint_t n) const {return func(pText, Object.pText, n);}                                      \
sint_t Compare##a##N(const char *iText, sint_t n)     const {return func(pText, iText, n);}

#define StringCompare(a,b) (CompareNoCase(a,b) == 0)
#define StringCompareCase(a,b) (CompareCase(a,b) == 0)

inline sint_t CompareCase(const char *iText1, const char *iText2) {return strcmp(iText1, iText2);}
inline sint_t CompareNoCase(const char *iText1, const char *iText2) {return stricmp(iText1, iText2);}
inline sint_t CompareCaseN(const char *iText1, const char *iText2, sint_t n) {return strncmp(iText1, iText2, n);}
inline sint_t CompareNoCaseN(const char *iText1, const char *iText2, sint_t n) {return strnicmp(iText1, iText2, n);}

extern sint_t CompareWordWise(const char *pText1, const char *pText2, bool comparenumbers = true);
extern sint_t CompareWordWiseNoCase(const char *pText1, const char *pText2, bool comparenumbers = true);

extern void EnableSlashFromEnd(bool bEnable = true);

extern AString Base64Encode(const uint8_t *ptr, uint_t bytes);

#define MAXPOS   MAX_SIGNED(sint_t)
#define MAXWORDS MAX_SIGNED(sint_t)

class AString : public AListNode {
public:
    AString(const char *iText = NULL, sint_t iLength = -1);
    AString(const AString *pString, sint_t iLength = -1);
    AString(const AString& String, sint_t iLength = -1);
    AString(const std::string& String, sint_t iLength = -1);
    AString(bool     v);
    AString(char     c);
    AString(sshort_t val, const char *format = "");
    AString(ushort_t val, const char *format = "");
    AString(sint_t   val, const char *format = "");
    AString(uint_t   val, const char *format = "");
    AString(slong_t  val, const char *format = "");
    AString(ulong_t  val, const char *format = "");
    AString(sllong_t val, const char *format = "");
    AString(ullong_t val, const char *format = "");
    AString(float    val, const char *format = "");
    AString(double   val, const char *format = "");
    virtual ~AString();

    LIST_FUNCTIONS(AString);

    LISTNODE_DUPLICATE(AString);

    // UTF8 encoding:
    // 0x00000000 - 0x0000007F:
    //      0xxxxxxx
    //
    //  0x00000080 - 0x000007FF:
    //      110xxxxx 10xxxxxx
    //
    //  0x00000800 - 0x0000FFFF:
    //      1110xxxx 10xxxxxx 10xxxxxx
    //
    //  0x00010000 - 0x001FFFFF:
    //      11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    static bool IsUTF8Char(char c)      {return (((uint8_t)c & 0x80) == 0x80);}
    static bool IsUTF8StartChar(char c) {return (((uint8_t)c & 0xc0) == 0xc0);}
    static bool IsUTF8ContChar(char c)  {return (((uint8_t)c & 0xc0) == 0x80);}
    static size_t UTF8CharLen(char c);
    static size_t UTF8Strlen(const char *p, size_t maxlen = (size_t)-1);

    bool Create(const char *iText, sint_t iLength = -1, bool limit = true);
    bool Append(const char *iText, sint_t iLength = -1, bool limit = true);
    void Delete();

    char *Steal(sint_t *pLength = NULL);
    void Take(const char *iText, sint_t iLength = -1);

    static AString ReadFile(const char *filename);
    bool ReadFromFile(const char *filename, bool append = false);
    bool WriteToFile(const char *filename, const char *mode = "w") const;

    AString& operator = (bool v);
    AString& operator = (char c);
    AString& operator = (const char *iText);
    AString& operator = (const AString *pString) {return operator = (pString->pText);}
    AString& operator = (const AString& String) {return operator = (String.pText);}
    AString& operator = (const std::string& String) {return operator = (String.c_str());}

    AString& operator = (sshort_t val);
    AString& operator = (ushort_t val);
    AString& operator = (uint_t   val);
    AString& operator = (sint_t   val);
    AString& operator = (slong_t  val);
    AString& operator = (ulong_t  val);
    AString& operator = (sllong_t val);
    AString& operator = (ullong_t val);
    AString& operator = (float    val);
    AString& operator = (double   val);

    AString& operator += (char c);
    AString& operator += (const char *iText);
    AString& operator += (const AString *pString);
    AString& operator += (const AString& String);
    AString& operator += (const std::string& String) {return operator = (String.c_str());}
    friend AString operator + (const AString& Object1, const char *iText2);
    friend AString operator + (const char *iText1, const AString& Object2);
    friend AString operator + (const AString& Object1, const AString& Object2);

    COMP_OPERATOR(==)
    COMP_OPERATOR(!=)
    COMP_OPERATOR(<=)
    COMP_OPERATOR(>=)
    COMP_OPERATOR(<)
    COMP_OPERATOR(>)

    bool Empty() const {return (Length == 0);}
    bool Valid() const {return (Length > 0);}

    COMP_CASE_OPERATOR(Case, strcmp)
    COMP_CASE_OPERATOR_N(Case, strncmp)

    COMP_CASE_OPERATOR(NoCase, stricmp)
    COMP_CASE_OPERATOR_N(NoCase, strnicmp)

    friend sint_t CompareWordWise(const AString& String1, const AString& String2, bool comparenumbers = true) {return CompareWordWise(String1.pText, String2.pText, comparenumbers);}
    friend sint_t CompareWordWise(const AString& String1, const char *pText2, bool comparenumbers = true) {return CompareWordWise(String1.pText, pText2, comparenumbers);}
    friend sint_t CompareWordWise(const char *pText1, const AString& String2, bool comparenumbers = true) {return CompareWordWise(pText1,String2.pText, comparenumbers);}

    friend sint_t CompareWordWiseNoCase(const AString& String1, const AString& String2, bool comparenumbers = true) {return CompareWordWiseNoCase(String1.pText, String2.pText, comparenumbers);}
    friend sint_t CompareWordWiseNoCase(const AString& String1, const char *pText2, bool comparenumbers = true) {return CompareWordWiseNoCase(String1.pText, pText2, comparenumbers);}
    friend sint_t CompareWordWiseNoCase(const char *pText1, const AString& String2, bool comparenumbers = true) {return CompareWordWiseNoCase(pText1,String2.pText, comparenumbers);}

    char operator[](sint_t n) const {
        if ((n >= 0) && (n < Length)) return pText[n];
        else                          return 0;
    }
    char operator[](uint_t n) const {
        if (n < (uint_t)Length) return pText[n];
        else                    return 0;
    }

    operator bool()           const;
    operator const char *()   const {return (const char *)pText;}

    operator sint8_t()        const {return (sint8_t)operator sllong_t();}
    operator uint8_t()        const {return (uint8_t)operator ullong_t();}
    operator sshort_t()       const {return (sshort_t)operator sllong_t();}
    operator ushort_t()       const {return (ushort_t)operator ullong_t();}
    operator sint_t()         const {return (sint_t)operator sllong_t();}
    operator uint_t()         const {return (uint_t)operator ullong_t();}
    operator slong_t()        const {return (slong_t)operator sllong_t();}
    operator ulong_t()        const {return (ulong_t)operator ullong_t();}
    operator sllong_t()       const;
    operator ullong_t()       const {return (ullong_t)operator sllong_t();}

    operator float()          const;
    operator double()         const;
    //operator uptr_t()       const {return (uptr_t)operator ullong_t();}

    operator std::string()    const {return std::string(pText);}

    AValue EvalNumber(AString *error) const {return EvalNumber(0, NULL, true, NULL, error);}
    AValue EvalNumber(bool allowModifiers, const char *terminators, AString *error = NULL) const {return EvalNumber(0, NULL, allowModifiers, terminators, error);}
    AValue EvalNumber(uint_t i = 0, uint_t *endIndex = NULL, bool allowModifiers = true, const char *terminators = NULL, AString *error = NULL) const;

    sint_t GetStartOfCurrentChar(sint_t pos) const;
    sint_t GetEndOfCurrentChar(sint_t pos) const;
    sint_t GetCharLength(sint_t pos) const {return GetEndOfCurrentChar(pos) + 1 - GetStartOfCurrentChar(pos);}
    sint_t GetStartOfChar(sint_t n) const;
    sint_t GetEndOfChar(sint_t n) const;
    sint_t CountChars(sint_t p1 = 0, sint_t p2 = INT_MAX) const;

    sint_t      GetLength()    const {return Length;}
    sint_t      GetCharCount() const {return CharCount;}
    const char *GetBuffer()    const {return pText;}

    sint_t     len()   const {return Length;}
    sint_t     chars() const {return CharCount;}
    const char *str()  const {return pText;}

    void   Format(const char *format, ...) PRINTF_FORMAT_METHOD;
    void   FormatV(const char *format, va_list ap);
    static AString Formatify(const char *format, ...) PRINTF_FORMAT_FUNCTION;
    static AString FormatifyV(const char *format, va_list ap);

    void   printf(const char *format, ...) PRINTF_FORMAT_METHOD;
    void   vprintf(const char *format, va_list ap);

    AString Arg(const char *iText) const;
    AString Arg(const AString *pString) const {return Arg(pString->pText);}
    AString Arg(const AString& String) const {return Arg(String.pText);}
    AString Arg(const std::string& String) const {return Arg(String.c_str());}
    AString Arg(bool n) const {return Arg((ulong_t)n);}
    AString Arg(sshort_t n) const;
    AString Arg(ushort_t n) const;
    AString Arg(sint_t   n) const;
    AString Arg(uint_t   n) const;
    AString Arg(slong_t  n) const;
    AString Arg(ulong_t  n) const;
    AString Arg(sllong_t n) const;
    AString Arg(ullong_t n) const;
    AString Arg(float    n) const;
    AString Arg(double   n) const;
    AString EndArgs() const;

    AString Left(sint_t length)                 const;
    AString Mid(sint_t pos, sint_t length = -1) const;
    AString Right(sint_t length)                const;
    AString Prefix()                            const;
    AString Suffix()                            const;
    AString SubString(char Marker, int& p)      const;

    enum {
        REPLACE_START = 1,
        REPLACE_END   = 2,
    };
    AString SearchAndReplace(const char *pSearch, const char *pReplace, uint_t flags = 0) const;
    AString SearchAndReplaceNoCase(const char *pSearch, const char *pReplace, uint_t flags = 0) const;

    enum {
        FLAG_SINGLE_QUOTE = 1,
        FLAG_DOUBLE_QUOTE = 2,
        FLAG_QUOTE_ESCAPE = 4,
        FLAG_SLASH_ESCAPE = 8,
        FLAG_INNER_SINGLE_QUOTE = 16,
        FLAG_INNER_DOUBLE_QUOTE = 32,

        DEFAULT_FLAGS = FLAG_SINGLE_QUOTE | FLAG_DOUBLE_QUOTE | FLAG_SLASH_ESCAPE,
    };

    sint_t StartOfLine(sint_t p, const char *pMarker = "\n") const;
    sint_t EndOfLine(sint_t p, const char *pMarker = "\n") const;
    sint_t PositionOfLine(sint_t n, const char *pMarker = "\n", uint_t flags = DEFAULT_FLAGS) const;
    sint_t CountLines(const char *pMarker = "\n", uint_t flags = DEFAULT_FLAGS) const;
    AString Line(sint_t n, const char *pMarker = "\n", uint_t flags = DEFAULT_FLAGS) const;
    AString CutLine(sint_t n, const char *pMarker = "\n", uint_t flags = DEFAULT_FLAGS);
    void    PasteLine(sint_t n, const AString& String, const char *pMarker = "\n", uint_t flags = DEFAULT_FLAGS);

    sint_t StartOfColumn(sint_t p, const char *pMarker = ",") const {
        return StartOfLine(p, pMarker);
    }
    sint_t EndOfColumn(sint_t p, const char *pMarker = ",") const {
        return EndOfLine(p, pMarker);
    }
    sint_t PositionOfColumn(sint_t n, const char *pMarker = ",", uint_t flags = DEFAULT_FLAGS) const {
        return PositionOfLine(n, pMarker, flags);
    }
    sint_t CountColumns(uint_t flags = DEFAULT_FLAGS) const {
        return CountLines(",", flags);
    }
    AString Column(sint_t n, uint_t flags = DEFAULT_FLAGS) const {
        return Line(n, ",", flags);
    }
    AString CutColumn(sint_t n, uint_t flags = DEFAULT_FLAGS) {
        return CutLine(n, ",", flags);
    }
    void PasteColumn(sint_t n, const AString& String, uint_t flags = DEFAULT_FLAGS) {
        return PasteLine(n, String, ",", flags);
    }

    sint_t Pos(const char *iText, sint_t startpos = 0, sint_t endpos = MAXPOS) const;
    sint_t Pos(const AString& String, sint_t startpos = 0, sint_t endpos = MAXPOS) const;
    sint_t LastPos(const char *iText, sint_t endpos = 0, sint_t startpos = MAXPOS) const {return Pos(iText, startpos, endpos);}
    sint_t LastPos(const AString& String, sint_t endpos = 0, sint_t startpos = MAXPOS) const {return Pos(String, startpos, endpos);}
    sint_t PosNoCase(const char *iText, sint_t startpos = 0, sint_t endpos = MAXPOS) const;
    sint_t PosNoCase(const AString& String, sint_t startpos = 0, sint_t endpos = MAXPOS) const;
    sint_t LastPosNoCase(const char *iText, sint_t endpos = 0, sint_t startpos = MAXPOS) const {return PosNoCase(iText, startpos, endpos);}
    sint_t LastPosNoCase(const AString& String, sint_t endpos = 0, sint_t startpos = MAXPOS) const {return PosNoCase(String, startpos, endpos);}

    bool StartsWith(const AString& String)       const {return (CompareCaseN(String, String.len()) == 0);}
    bool StartsWithNoCase(const AString& String) const {return (CompareNoCaseN(String, String.len()) == 0);}
    bool EndsWith(const AString& String)         const {return (::CompareCaseN(str() + MAX(len() - String.len(), 0), String, String.len()) == 0);}
    bool EndsWithNoCase(const AString& String)   const {return (::CompareNoCaseN(str() + MAX(len() - String.len(), 0), String, String.len()) == 0);}

    sint_t CharPos(char c, sint_t pos = 0) const;
    sint_t LastCharPos(char c, sint_t pos = MAXPOS) const;

    sint_t OutputLength(sint_t TabSize = 8, sint_t l = -1) const;
    void TabToNextColumn(sint_t TabSize = 8, sint_t ColumnSize = 8);
    AString ExpandTabs(sint_t TabSize = 8) const;

    sint_t StartOfNextWord(sint_t pos, uint_t flags = DEFAULT_FLAGS) const;
    sint_t EndOfThisWord(sint_t pos, uint_t flags = DEFAULT_FLAGS) const;
    sint_t WordUnderPos(sint_t pos, uint_t flags = DEFAULT_FLAGS) const;

    sint_t CountWords(uint_t flags = DEFAULT_FLAGS) const;
    sint_t StartOfWord(sint_t n, uint_t flags = DEFAULT_FLAGS) const;
    sint_t EndOfWord(sint_t n, uint_t flags = DEFAULT_FLAGS) const;
    AString Word(sint_t nWord, uint_t flags = DEFAULT_FLAGS) const {return Words(nWord, 1, flags);}
    AString Words(sint_t nWord, sint_t nWords = MAXWORDS, uint_t flags = DEFAULT_FLAGS) const;
    AString LastWords(sint_t nWords = 1, uint_t flags = DEFAULT_FLAGS) const;
    AString LastWord(uint_t flags = DEFAULT_FLAGS) const {return LastWords(1, flags);}
    void SplitWords(sint_t n, AString& String1, AString& String2, uint_t flags = DEFAULT_FLAGS) const;
    AString CatWords(const AString& word1, const AString& word2 = "") const;
    AString DelWords(sint_t nWord, sint_t nWords = 1, uint_t flags = DEFAULT_FLAGS);

    sint_t ForwardWord(sint_t pos) const;
    sint_t BackwardWord(sint_t pos) const;

    virtual void UpperCase();
    virtual void LowerCase();
    virtual void ApplyCase(const char *iText);
    virtual void ApplyCase(const AString& String);

    AString ToUpper() const;
    AString ToLower() const;

    sint_t ForwardSexp(sint_t pos, const char *sexp = "{}[]()", bool bQuotes = true) const;
    sint_t BackwardSexp(sint_t pos, const char *sexp = "{}[]()", bool bQuotes = true) const;

    AString *Find(const char *iText, bool bPartial = false) const;
    AString *Find(const AString& String, bool bPartial = false) const;
    AString *Find(const char *iText, int& n, bool bPartial = false) const;
    AString *Find(const AString& String, int& n, bool bPartial = false) const;
    AString *FindBackwards(const char *iText, bool bPartial = false) const;
    AString *FindBackwards(const AString& String, bool bPartial = false) const;
    AString *FindBackwards(const char *iText, int& n, bool bPartial = false) const;
    AString *FindBackwards(const AString& String, int& n, bool bPartial = false) const;

    sint_t ReadCh(FILE *fp);
    sint_t ReadLn(FILE *fp);
    bool WriteCh(FILE *fp) const;
    bool WriteLn(FILE *fp) const;

    sint_t ReadCh(AStdData *fp);
    sint_t ReadLn(AStdData *fp);
    bool WriteCh(AStdData *fp) const;
    bool WriteLn(AStdData *fp) const;

    sint_t ReadCh(AStdData& file) {return ReadCh(&file);}
    sint_t ReadLn(AStdData& file) {return ReadLn(&file);}
    bool WriteCh(AStdData& file) const {return WriteCh(&file);}
    bool WriteLn(AStdData& file) const {return WriteLn(&file);}

    sint_t ReadLines(FILE *fp, AString *pInitialString = NULL);

    sint_t MatchLength(const char *iText) const;
    sint_t MatchLength(const AString& String) const;
    sint_t MatchLength(AString *pString) const;
    AString FindMinimumMatchLength(const AString& String) const;

    sint_t MatchBeginning(const char *iText) const;
    sint_t MatchBeginning(const AString& String) const;

    AString Quotify(bool bEnd = false, bool bForce = false) const;
    AString DeQuotify(bool bBoth = false) const;

    AString Escapify() const;
    AString DeEscapify() const;

    AString URLify() const;
    AString DeURLify() const;

    AString HTMLify() const;
    AString DeHTMLify() const;

    AString PathPart() const;
    AString FilePart() const;
    AString CatPath(const AString& Path1, const AString& Path2 = "", char slash = 0) const;

    char FirstChar() const {return (Length > 0) ? pText[0] : 0;}
    char LastChar()  const {return (Length > 0) ? pText[Length - 1] : 0;}
    char SlashChar() const;

    AString& AddSlash(char slash = 0);

    AString Copies(sint_t n = 1) const;

    AString& AppendCopies(const char *pString, sint_t n = 1);

    AString& Replace(const char *pSearchChars, const char *pReplaceChars, bool bIncludeZero = false);

    AString RemoveWhiteSpace() const;
    AString Remove(const char *pChars) const;
    AString StripUnprintable() const;

    void ForwardSlashify() {Replace("\\","/");}
    void BackSlashify() {Replace("/","\\");}

    AString ForwardSlashes() const {
        AString String = this;
        String.Replace("\\","/");
        return String;
    }
    AString BackSlashes() const {
        AString String = this;
        String.Replace("/","\\");
        return String;
    }

    AString Abbreviate(sint_t n) const;

    AString& Insert(sint_t Pos, const char *pString);
    AString& DeleteChars(sint_t Pos, sint_t nChars);

    sint_t  Base64DecodeLength() const;
    sint_t  Base64Decode(uint8_t *buffer, uint_t maxbytes) const;

    AString Base64Encode() const;
    AString Base64Decode() const;

    virtual AString Encode() const;
    virtual AString Decode() const;

    virtual void TimeString();
    virtual void TimeString(time_t seconds);

    AString ExtractColumns(const char *pSeparator = ",") const;

    void SplitColumns(const char *pSeparator, AString *pPreString, AString *pPostString) const;
    sint_t ExtractColumns(const char *pSeparator, AString *pStrings, sint_t MaxStrings) const;

    AString ExtractRegion(char StartChar, char EndChar, const char *pReplace);

    AString GetToken(const char *pSeparators, sint_t& pos) const;

    virtual AString StripBrackets() const;

    sint_t  GetFieldPos(const char *pStartMarker, const char *pEndMarker, sint_t start = 0, sint_t *len = NULL) const;
    AString GetField(const char *pStartMarker, const char *pEndMarker, sint_t start = 0, sint_t *len = NULL) const;
    sint_t  GetField(const char *pStartMarker, const char *pEndMarker, AString& str, sint_t start = 0, sint_t *len = NULL) const;
    bool    SetField(const char *pStartMarker, const char *pEndMarker, const char *pString, sint_t start = 0);

    sint_t  GetFieldNumber(const char *pMarker, sint_t n, AString& str, sint_t start = 0) const;

    AString AllUpperCase() const;
    AString InitialCapitalCase() const;

    AString MD5Hash() const;

    uint_t  Count(const char *pString) const;
    uint_t  CountNoCase(const char *pString) const;

    sint_t  FindClosing(char terminator, sint_t p, const char *nestedchars = "{}[]()") const;

    static sint_t AlphaCompareNoCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext);
    static sint_t AlphaCompareCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext);

    static AString EngFormat(double value, uint_t mindec = 1, uint_t maxdec = 3, double scale = 1000.0);

    static void EnableSlashFromEnd(bool bEnable) {bSlashFromEnd = bEnable;}

    static void DeleteString(uptr_t item, void *context) {UNUSED(context); if (item) delete (AString *)item;}

protected:
    void Assign(char *pStr, int iLength = -1);

    static sint_t FindEndQuote(char q, sint_t length, const char *p, uint_t flags);

    AString FindFormatSpecifier(AString& left, AString& right) const;
    AString InsertValue(const AValue& val) const;

    sint_t PosEx(const char *text, sint_t len, sint_t startpos, sint_t endpos, bool bCase = true) const;

    enum {
        FIND_NO_QUOTES = 0,
        FIND_DOUBLE_QUOTE,
        FIND_SINGLE_QUOTE,
        FIND_BOTH_QUOTES,
    };
    static const char *FindTokenForwards(const char *p, char token, const char *tokens = NULL, uint_t quotes = FIND_NO_QUOTES);
    static const char *FindTokenBackwards(const char *p, const char *p0, char token, const char *tokens = NULL, uint_t quotes = FIND_NO_QUOTES);

    static sint_t FindMarker(const char *pText, const char *pMarker, sint_t TextLen, sint_t MarkerLen, uint_t flags, sint_t p);

    // to avoid hidden function warning
    virtual void Insert(AListNode *pNode1, AListNode *pNode2) {AListNode::Insert(pNode1, pNode2);}

protected:
    char   *pText;
    sint_t Length;
    sint_t CharCount;

    NODETYPE_DEFINE(AString);

    static char pDefaultText[];
    static bool bSlashFromEnd;
};

class AStringUpdate {
public:
    AStringUpdate(AString *iString = NULL, sint_t iBufferSize = 256);
    ~AStringUpdate();

    bool Attach(AString *iString, sint_t iBufferSize = 256);

    void Update(char c);
    void Update(char *p) {Update((const char *)p);}
    void Update(const char *p);
    void Update(const AString& String);

    void Flush();

    AStringUpdate& operator += (char c)                {Update(c);      return *this;}
    AStringUpdate& operator += (char *p)               {Update(p);      return *this;}
    AStringUpdate& operator += (const char *p)         {Update(p);      return *this;}
    AStringUpdate& operator += (const AString& String) {Update(String); return *this;}

    sint_t TotalLength() const;

protected:
    AString *pString;
    char    *pBuffer;
    sint_t  BufferPos, BufferSize;
};

class AKeyValuePair : public AListNode {
public:
    AKeyValuePair();
    AKeyValuePair(const AKeyValuePair& object);
    virtual ~AKeyValuePair();

    LIST_FUNCTIONS(AKeyValuePair);

    LISTNODE_DUPLICATE(AKeyValuePair);

    AString Key;
    AString Value;

    static sint_t AlphaCompareKeyNoCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext);
    static sint_t AlphaCompareKeyCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext);
    static sint_t AlphaCompareValueNoCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext);
    static sint_t AlphaCompareValueCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext);

protected:
    NODETYPE_DEFINE(AKeyValuePair);
};

class AKeyObjectPair : public AListNode {
public:
    AKeyObjectPair(bool autodelete = true);
    AKeyObjectPair(const AKeyObjectPair& object, bool autodelete = true);
    virtual ~AKeyObjectPair();

    LIST_FUNCTIONS(AKeyObjectPair);

    LISTNODE_DUPLICATE(AKeyObjectPair);

    AString   Key;
    AListNode *pObject;

    static sint_t AlphaCompareKeyNoCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext);
    static sint_t AlphaCompareKeyCase(const AListNode *pNode1, const AListNode *pNode2, void *pContext);

protected:
    bool bAutoDelete;

    NODETYPE_DEFINE(AKeyObjectPair);
};

class AStringPair : public AListNode {
public:
    AStringPair();
    AStringPair(const AStringPair& object);
    virtual ~AStringPair();

    LIST_FUNCTIONS(AStringPair);

    LISTNODE_DUPLICATE(AStringPair);

    AString String1;
    AString String2;

protected:
    NODETYPE_DEFINE(AStringPair);
};

class AStringPairWithInt : public AListNode {
public:
    AStringPairWithInt();
    AStringPairWithInt(const AStringPairWithInt& object);
    virtual ~AStringPairWithInt();

    LIST_FUNCTIONS(AStringPairWithInt);

    LISTNODE_DUPLICATE(AStringPairWithInt);

    uint_t  Integer;
    AString String1;
    AString String2;

protected:
    NODETYPE_DEFINE(AStringPairWithInt);
};

class AStringTriplet : public AListNode {
public:
    AStringTriplet();
    AStringTriplet(const AStringTriplet& object);
    virtual ~AStringTriplet();

    LIST_FUNCTIONS(AStringTriplet);

    LISTNODE_DUPLICATE(AStringTriplet);

    AString String1;
    AString String2;
    AString String3;

protected:
    NODETYPE_DEFINE(AStringTriplet);
};

inline AString AddressString(const void *p) {return AValue(p).ToString("x");}

#endif
