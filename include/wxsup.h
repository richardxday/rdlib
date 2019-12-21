#ifndef __WXSUP__
#define __WXSUP__

#include "wx/wx.h"
#ifdef __CYGWIN__
#undef _WIN32
#endif
#include "strsup.h"

inline wxString _wxString(const AString& str) {
    return wxString::FromAscii(str.str());
}

inline AString _AString(const wxString& str) {
    return AString(str.mb_str(wxConvUTF8));
}

#endif
