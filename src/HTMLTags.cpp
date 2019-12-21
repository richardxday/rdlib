
#include <stdio.h>

#include "HTMLTags.h"

/* end of includes */

AStdData *AHTML::Output = Stdout;

AString Options(const AString& options, const AString& name = "")
{
    return (options.Valid() ? (" " + (name.Valid() ? name + "='" + options + "'" : options)) : "");
}

AString Options(uint_t n, const AString& name = "", uint_t defval = 0, bool force = false)
{
    return Options((force || (n != defval)) ? AString(n) : "", name);
}

AHTML::AHTML(const AString& tag, bool lf)
{
    if (tag.Valid()) {
        if (lf) Output->printf("<%s>\n", tag.str());
        else    Output->printf("<%s>", tag.str());
    }
}

sint_t AHTML::printf(const char *fmt, ...)
{
    va_list ap;
    sint_t l;

    va_start(ap, fmt);
    l = AHTML::vprintf(fmt, ap);
    va_end(ap);

    return l;
}

AHTMLText::AHTMLText(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    AHTML::vprintf(format, ap);
    va_end(ap);
}

AHTMLTag::AHTMLTag(const AString& tag, const AString& options, bool lf, bool lfend) : Tag(tag),
                                                                                      LFAtEnd(lfend)
{
    if (Tag.Valid()) {
        AHTML(Tag + Options(options.Words(0)), lf);
    }
}

AHTMLTag::~AHTMLTag()
{
    if (Tag.Valid()) {
        AHTML("/" + Tag, LFAtEnd);
    }
}

AHTMLQuickTag::AHTMLQuickTag(const AString& tag, const AString& text)
{
    AHTMLTag _tag(tag);
    AHTML::printf("%s", text.str());
}

AString AHTMLFont::DefaultFamily = "helvetica, sans-serif";

AHTMLFont::AHTMLFont(const AString& family, uint_t size, const AString& options) :
    AHTMLTag("font", GenerateOptions(family, size, options), true)
{
}

AHTMLFont::AHTMLFont(uint_t size, const AString& options) :
    AHTMLTag("font", GenerateOptions(DefaultFamily, size, options), true)
{
}

AString AHTMLFont::GenerateOptions(const AString& family, uint_t size, const AString& options)
{
    return "style='font-size: " + AString(size) + "px; font-family: " + family + "'" + Options(options);
}

AHTMLTextSize::AHTMLTextSize(uint_t size) : AHTMLTag("font", "size='" + AString(size) + "'")
{
}

AHTMLTextColour::AHTMLTextColour(const AString& colour) : AHTMLTag("font", "color='" + colour + "'")
{
}

AHTMLForm::AHTMLForm(const AString& action, const AString& options) :
    AHTMLTag("form", "method='post' action='" + action + "'" + Options(options), true)
{
}

AHTMLInput::AHTMLInput(const AString& type, const AString& name, const AString& value, const AString& options)
{
    AHTML("input type='" + type + "'" + Options(name, "name") + Options(value, "value") + Options(options) + " /");
}

AHTMLSubmit::AHTMLSubmit(const AString& value, const AString& options) : AHTMLInput("submit", "", value, options)
{
}

AHTMLNamedSubmit::AHTMLNamedSubmit(const AString& name, const AString& value, const AString& options) : AHTMLInput("submit", name, value, options)
{
}

AHTMLTable::AHTMLTable() : AHTMLTag("table", "", true)
{
}

AHTMLTable::AHTMLTable(const AString& bgcolour, uint_t border, uint_t spacing, uint_t padding, const AString& options) :
    AHTMLTag("table", GenerateOptions(bgcolour, border, spacing, padding, options), true)
{
}

AString AHTMLTable::GenerateOptions(const AString& bgcolour, uint_t border, uint_t spacing, uint_t padding, const AString& options)
{
    return (Options(bgcolour, "bgcolor") +
            Options(border, "border", 0, true) +
            Options(spacing, "cellspacing", 0, true) +
            Options(padding, "cellpadding", 0, true) +
            Options(options));
}

AHTMLTableRow::AHTMLTableRow() : AHTMLTag("tr", "", true)
{
}

AHTMLTableRow::AHTMLTableRow(const AString& bgcolour) : AHTMLTag("tr", Options(bgcolour, "bgcolor"))
{
}

AHTMLTableCell::AHTMLTableCell() : AHTMLTag("td")
{
}

AHTMLTableCell::AHTMLTableCell(const AString& bgcolour, const AString& align, bool nowrap, const AString& options) :
    AHTMLTag("td", Options(bgcolour, "bgcolor") + Options(align, "align") + Options((uint_t)nowrap, "nowrap") + Options(options))
{
}

AHTMLEmptyCell::AHTMLEmptyCell(uint_t span)
{
    AHTMLTableCell cell("", "", false, Options(span, "colspan", 1));
    AHTML::printf("&nbsp;");
}

AHTMLEmptyCell::AHTMLEmptyCell(const AString& bgcolour, uint_t span, const AString& options)
{
    AHTMLTableCell cell(bgcolour, "", false, Options(span, "colspan", 1) + Options(options));
    AHTML::printf("&nbsp;");
}

AHTMLTextSpan::AHTMLTextSpan(bool bold, bool underline, bool lf) : AHTMLTag("span", GenerateOptions(bold, underline), false, lf)
{
}

AString AHTMLTextSpan::GenerateOptions(bool bold, bool underline)
{
    AString options = "style='";
    if (bold)      options += "font-weight: bold; ";
    if (underline) options += "text-decoration: underline; ";
    options += "'";
    return options;
}

AHTMLHRef::AHTMLHRef(const AString& url, const AString& text, bool bold, const AString& options)
{
    AHTMLTag tag("a", "href='" + url + "'" + Options(options), false, false);
    if (bold) {
        AHTMLBoldText bold(false);
        AHTML::printf("%s", text.str());
    }
    else AHTML::printf("%s", text.str());
}

AHTMLImage::AHTMLImage(const AString& src, const AString& options)
{
    AHTML("img src='" + src + "'" + Options(options));
}

AHTMLBreak::AHTMLBreak(bool lf)
{
    AHTML("br", lf);
}

AHTMLTitle::AHTMLTitle(const AString& text) : AHTMLQuickTag("title", text)
{
}

AHTMLOption::AHTMLOption(const AString& text) : AHTMLQuickTag("option", text)
{
}

AHTMLDualOption::AHTMLDualOption(const AString& value, const AString& text)
{
    AHTMLTag _tag("option", "value='" + value + "'");
    AHTML::printf("%s", text.str());
}

AHTMLSpacePad::AHTMLSpacePad()
{
    AHTML::printf("&nbsp;");
}

AHTMLSpacePad::~AHTMLSpacePad()
{
    AHTML::printf("&nbsp;");
}

AHTMLStyleSheet::AHTMLStyleSheet() : AHTMLTag("style", "type='text/css'", true)
{
}

AHTMLDocument::AHTMLDocument()
{
    AHTMLText("<!DOCTYPE html PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN'>\n");
}

AHTMLMetaData::AHTMLMetaData(const AString& charset)
{
    AHTMLText("<meta http-equiv='content-type'\n");
    AHTMLText("content='text/html; charset=%s'>\n", charset.str());
}

AHTMLMetaData::AHTMLMetaData(const AString& redirecturl, uint_t seconds, const AString& charset)
{
    if (redirecturl.Valid()) {
        AHTMLText("<meta http-equiv='refresh' content='%u; url=%s'", seconds, redirecturl.str());
    }
    else {
        AHTMLText("<meta http-equiv='content-type'\n");
    }

    AHTMLText("content='text/html; charset=%s'>\n", charset.str());
}

ARSSFeedDocument::ARSSFeedDocument()
{
    AHTMLText("<?xml version='1.0' encoding='UTF-8'?>\n");
}

AHTMLHead::AHTMLHead() : AHTMLTag("head", "", true)
{
}

AHTMLBody::AHTMLBody() : AHTMLTag("body", "", true)
{
}

AHTMLHTML::AHTMLHTML() : AHTMLTag("html", "", true)
{
}
