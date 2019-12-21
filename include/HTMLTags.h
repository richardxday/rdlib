
#ifndef __HTML_TAGS__
#define __HTML_TAGS__

#include "strsup.h"

class AHTML {
public:
    AHTML(const AString& tag = "", bool lf = true);

    static sint_t   printf(const char *fmt, ...) PRINTF_FORMAT_FUNCTION;
    static sint_t   vprintf(const char *fmt, va_list ap) {return Output->vprintf(fmt, ap);}
    static sint32_t writebytes(const void *buf, uint32_t bytes, uint_t swap = AStdData::SWAP_NEVER) {return Output->writebytes(buf, bytes, swap);}

    static AStdData *Output;
};

class AHTMLFileSet {
public:
    AHTMLFileSet(AStdData *output) : OldOutput(AHTML::Output) {AHTML::Output = output;}
    ~AHTMLFileSet() {AHTML::Output = OldOutput;}

protected:
    AStdData *OldOutput;
};

class AHTMLText {
public:
    AHTMLText(const char *format, ...) PRINTF_FORMAT_METHOD;
};

class AHTMLTag {
public:
    AHTMLTag(const AString& tag, const AString& options = "", bool lf = false, bool lfend = true);
    virtual ~AHTMLTag();

protected:
    AString Tag;
    bool    LFAtEnd;
};

class AHTMLQuickTag {
public:
    AHTMLQuickTag(const AString& tag, const AString& text);
};

class AHTMLFont : public AHTMLTag {
public:
    AHTMLFont(const AString& family, uint_t size, const AString& options = "");
    AHTMLFont(uint_t size, const AString& options = "");

    static AString Style(const AString& family, uint_t size, const AString& options = "") {return GenerateOptions(family, size, options);}
    static AString Style(uint_t size, const AString& options = "") {return GenerateOptions(DefaultFamily, size, options);}

    static void           SetDefaultFamily(const AString& family) {DefaultFamily = family;}
    static const AString& GetDefaultFamily() {return DefaultFamily;}

protected:
    static AString GenerateOptions(const AString& family, uint_t size, const AString& options);

protected:
    static AString DefaultFamily;
};

class AHTMLTextSize : public AHTMLTag {
public:
    AHTMLTextSize(uint_t size);
};

class AHTMLTextColour : public AHTMLTag {
public:
    AHTMLTextColour(const AString& colour);
};

class AHTMLRedText : public AHTMLTextColour {
public:
    AHTMLRedText() : AHTMLTextColour("red") {}
};

class AHTMLForm : public AHTMLTag {
public:
    AHTMLForm(const AString& action, const AString& options = "");
};

class AHTMLInput {
public:
    AHTMLInput(const AString& type, const AString& name, const AString& value = "", const AString& options = "");
};

class AHTMLSubmit : public AHTMLInput {
public:
    AHTMLSubmit(const AString& value = "", const AString& options = "");
};

class AHTMLNamedSubmit : public AHTMLInput {
public:
    AHTMLNamedSubmit(const AString& name, const AString& value = "", const AString& options = "");
};

class AHTMLTable : public AHTMLTag {
public:
    AHTMLTable();
    AHTMLTable(const AString& bgcolour, uint_t border = 0, uint_t spacing = 0, uint_t padding = 0, const AString& options = "");

protected:
    AString GenerateOptions(const AString& bgcolour, uint_t border, uint_t spacing, uint_t padding, const AString& options);
};

class AHTMLTableRow : public AHTMLTag {
public:
    AHTMLTableRow();
    AHTMLTableRow(const AString& bgcolour);
};

class AHTMLTableCell : public AHTMLTag {
public:
    AHTMLTableCell();
    AHTMLTableCell(const AString& bgcolour, const AString& align = "", bool nowrap = false, const AString& options = "");
};

class AHTMLEmptyCell {
public:
    AHTMLEmptyCell(uint_t span = 1);
    AHTMLEmptyCell(const AString& bgcolour, uint_t span = 1, const AString& options = "");
};

class AHTMLCentreText : public AHTMLTag {
public:
    AHTMLCentreText() : AHTMLTag("center") {}
};

class AHTMLBoldText : public AHTMLTag {
public:
    AHTMLBoldText(bool lf = true) : AHTMLTag("b", "", false, lf) {}
};

class AHTMLSpan : public AHTMLTag {
public:
    AHTMLSpan(const AString& options, bool lf = true) : AHTMLTag("span", options, false, lf) {}
};

class AHTMLTextSpan : public AHTMLTag {
public:
    AHTMLTextSpan(bool bold = true, bool underline = true, bool lf = true);

protected:
    AString GenerateOptions(bool bold = true, bool underline = true);
};

class AHTMLHRef {
public:
    AHTMLHRef(const AString& url, const AString& text, bool bold = false, const AString& options = "");
};

class AHTMLImage {
public:
    AHTMLImage(const AString& src, const AString& options = "");
};

class AHTMLBreak {
public:
    AHTMLBreak(bool lf = true);
};

class AHTMLTitle : public AHTMLQuickTag {
public:
    AHTMLTitle(const AString& text);
};

class AHTMLOption : public AHTMLQuickTag {
public:
    AHTMLOption(const AString& text);
};

class AHTMLDualOption {
public:
    AHTMLDualOption(const AString& value, const AString& text);
};

class AHTMLSpacePad {
public:
    AHTMLSpacePad();
    ~AHTMLSpacePad();
};

class AHTMLStyleSheet : public AHTMLTag {
public:
    AHTMLStyleSheet();
};

class AHTMLDocument {
public:
    AHTMLDocument();
};

class AHTMLMetaData {
public:
    AHTMLMetaData(const AString& charset = "iso-8859-1");
    AHTMLMetaData(const AString& redirecturl, uint_t seconds, const AString& charset = "iso-8859-1");
};

class ARSSFeedDocument {
public:
    ARSSFeedDocument();
};

class AHTMLHead : public AHTMLTag {
public:
    AHTMLHead();
};

class AHTMLBody : public AHTMLTag {
public:
    AHTMLBody();
};

class AHTMLHTML : public AHTMLTag {
public:
    AHTMLHTML();
};

#endif
